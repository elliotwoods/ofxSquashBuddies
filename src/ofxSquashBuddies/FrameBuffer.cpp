#include "FrameBuffer.h"

#include "Constants.h"

namespace ofxSquashBuddies {
#pragma mark FrameBuffer
	//---------
	FrameBuffer::FrameBuffer(ThreadChannel<Message> & decompressorToFrameReceiver) :
	decompressorToFrameReceiver(decompressorToFrameReceiver)
	{
		this->clear();

		this->decompressThread = thread([this]() {
			this->decompressLoop();
		});
	}

	//---------
	FrameBuffer::~FrameBuffer() {
		this->threadsRunning = false;
		this->packetsToDecompressor.close();

		if (this->decompressThread.joinable()) {
			this->decompressThread.join();
		}
	}

	//---------
	void FrameBuffer::setFrameIndex(uint32_t frameIndex) {
		this->frameIndex = frameIndex;
	}

	//---------
	uint32_t FrameBuffer::getFrameIndex() const {
		return this->frameIndex;
	}

	//---------
	void FrameBuffer::add(const Packet & packet) {
		auto lock = unique_lock<mutex>(this->packetsMutex);

		//store the packet
		this->packets.emplace(packet.header.packetIndex, make_unique<Packet>(packet));

		//pass any sequential packets to the decompressor
		auto firstPacketInBuffer = this->packets.begin();
		while (this->packetIndexPosition == firstPacketInBuffer->first) {
			//send it
			this->packetsToDecompressor.send(*firstPacketInBuffer->second.get());

			//remove from here
			this->packets.erase(firstPacketInBuffer);
			this->packetIndexPosition++;

			//if there's still more to do
			if (this->packets.empty()) {
				break;
			}
			else {
				firstPacketInBuffer = this->packets.begin();
			}
		}
	}

	//---------
	void FrameBuffer::decompressLoop() {
		while (this->threadsRunning) {
			Packet packet;
			if (this->packetsToDecompressor.receive(packet)) {
				if (!this->codec.isValid()) {
					OFXSQUASHBUDDIES_ERROR << "Codec [" << this->codec.getName() << "] is not valid. Are you sure you have the plugins installed correctly?";
					continue;
				}

				auto lock = unique_lock<mutex>(this->streamMutex);

				this->stream->read((const void*)packet.payload, packet.header.payloadSize);

				if (packet.header.isLastPacket)
				{
					(*this->stream) << ofxSquash::Stream::Finish();
					lock.unlock(); //unlock the stream

					this->decompressorToFrameReceiver.send(this->message);

					this->success = true;
					this->clear();
				}
			}
		}
	}

	//---------
	void FrameBuffer::writeFunction(const ofxSquash::WriteFunctionArguments & args) {
		this->message.pushData(args.data, args.size);
	}

#pragma mark FrameBufferTCP
	//---------
	FrameBufferTCP::FrameBufferTCP(ThreadChannel<Message> & decompressorToFrameReceiver) : FrameBuffer(decompressorToFrameReceiver) {
		codec = ThingsInCommonTCP::getDefaultCodec();
	}

	//---------
	void FrameBufferTCP::setCodec(const ofxSquash::Codec & codec) {
		this->codec = codec;

		this->stream = make_unique<ofxSquash::Stream>(this->codec, ofxSquash::Decompress);
		this->stream->setWriteFunction([this](const ofxSquash::WriteFunctionArguments & args) {
			this->writeFunction(args);
		});

		this->clear();
	}
	
	//---------
	void FrameBufferTCP::clear() {
		{
			auto lock = unique_lock<mutex>(this->packetsMutex);

			if (!this->success && this->frameIndex != 0) {
				//We cleared before all our packets were processed, and we are an active frame
				//Create some debug info and report back.
				size_t droppedPackets = 0;

				uint16_t lastPacket;
				if (!this->packets.empty()) {
					lastPacket = max<uint16_t>(this->packetIndexPosition, this->packets.rbegin()->first);
					for (int i = 0; i < lastPacket; i++) {
						//see what holes we have in our packet indices
						auto findPacket = this->packets.find(i);
						if (findPacket == this->packets.end() && i >= this->packetIndexPosition) {
							droppedPackets++;
						}
					}
				}
				else {
					lastPacket = this->packetIndexPosition;
				}

				//send a DroppedFrame back up the chain
				DroppedFrame droppedFrame = {
					DroppedFrame::DroppedPackets,
					droppedPackets,
					lastPacket
				};
				onDroppedFrame.notify(this, droppedFrame);
			}

			this->packets.clear();
			this->packetIndexPosition = 0;
			this->success = false;
		}

		{
			auto lock = unique_lock<mutex>(this->messageMutex);
			this->message.clear();
		}

		{
			auto lock = unique_lock<mutex>(this->streamMutex);

			if (this->stream) {
				this->stream->clear();
			}
		}

		this->frameIndex = 0;
	}

#pragma mark FrameBufferUDP
	//---------
	FrameBufferUDP::FrameBufferUDP(ThreadChannel<Message> & decompressorToFrameReceiver) : FrameBuffer(decompressorToFrameReceiver) {
		codec = ThingsInCommonUDP::getDefaultCodec();
	}

	//---------
	void FrameBufferUDP::setCodec(const ofxSquash::Codec & codec) {
		this->codec = codec;

		if (this->packetIndexPosition == 0) {
			//reset if we haven't started processing packets yet
			this->clear();
		}
	}

	//---------
	void FrameBufferUDP::clear() {
		{
			auto lock = unique_lock<mutex>(this->packetsMutex);

			if (!this->packets.empty()) {
				//We cleared before all our packets were processed
				auto lastPacket = max<uint16_t>(this->packetIndexPosition, this->packets.rbegin()->first);
				size_t droppedPackets = 0;
				for (int i = 0; i < lastPacket; i++) {
					auto findPacket = this->packets.find(i);
					if (findPacket == this->packets.end() && i >= this->packetIndexPosition) {
						droppedPackets++;
					}
				}

				//send a DroppedFrame back up the chain
				DroppedFrame droppedFrame = {
					DroppedFrame::DroppedPackets,
					droppedPackets,
					lastPacket
				};
				onDroppedFrame.notify(this, droppedFrame);
			}

			this->packets.clear();
			this->packetIndexPosition = 0;
		}

		{
			auto lock = unique_lock<mutex>(this->messageMutex);
			this->message.clear();
		}

		{
			auto lock = unique_lock<mutex>(this->streamMutex);

			this->stream = make_unique<ofxSquash::Stream>(this->codec, ofxSquash::Decompress);
			this->stream->setWriteFunction([this](const ofxSquash::WriteFunctionArguments & args) {
				this->writeFunction(args);
			});
		}

		this->frameIndex = 0;
	}

#pragma mark FrameBufferSet
	//---------
	FrameBufferSet::~FrameBufferSet() {
		this->threadRunning = false;
		if (this->dataGramProcessorThread.joinable()) {
			this->dataGramProcessorThread.join();
		}
	}

	//---------
	void FrameBufferSet::setCodec(const ofxSquash::Codec & codec) {
		for (auto & frameBuffer : this->frameBuffers) {
			frameBuffer->setCodec(codec);
		}
	}

	//---------
	bool FrameBufferSet::isExpired(uint32_t frameIndex) const {
		// this function is called by the datagram processor, to throw away any packets early on which are irrelevant
		// (will be associated with some sort of dropped packet)

		for (auto frameBuffer : this->frameBuffers) {
			if (frameBuffer->getFrameIndex() == frameIndex) {
				return false;
			}
		}

		auto currentMinFrameIndex = numeric_limits<uint32_t>::max();
		for (auto frameBuffer : this->frameBuffers) {
			if (frameBuffer->getFrameIndex() < currentMinFrameIndex) {
				currentMinFrameIndex = frameBuffer->getFrameIndex();
			}
		}

		if (frameIndex < currentMinFrameIndex) {
			//we're skipping backwards
			if (frameIndex + OFXSQUASHBUDDIES_MAX_FRAME_DISCONTINUITY > currentMinFrameIndex) {
				return true;
			}
		}

		return false;
	}

	//---------
	const vector<shared_ptr<FrameBuffer>> & FrameBufferSet::getFrameBuffers() const {
		return this->frameBuffers;
	}

#pragma mark FrameBufferSetTCP
	//---------
	FrameBufferSetTCP::FrameBufferSetTCP() {
		for (int i = 0; i < 2; i++) {
			auto frameBuffer = make_shared<FrameBufferTCP>(this->decompressorToFrameReceiver);
			this->frameBuffers.emplace_back(frameBuffer);
			frameBuffer->onDroppedFrame.add(this, &FrameBufferSetTCP::callbackDroppedFrame, 0);
		}

		this->dataGramProcessorThread = thread([this]() {
			this->dataGramProcessorLoop();
		});
	}

	//---------
	FrameBufferSetTCP::~FrameBufferSetTCP() {
		this->socketToFrameBuffers.close();
	}

	//---------
	FrameBuffer & FrameBufferSetTCP::getFrameBuffer(uint32_t frameIndex) {
		//this function is called by the datagram processor to provide a useful frame buffer
		// for this frameIndex. Note, that it's not the responsibility of this function
		// to determine if the frameIndex should be assigned a frameBuffer.
		// That's handled by isExpired earlier in execution.

		// Also note : FrameBuffers will clear themselves when correctly processed

		for (const auto & frameBuffer : this->frameBuffers) {
			if (frameBuffer->getFrameIndex() == frameIndex) {
				return *frameBuffer;
			}
		}

		//if we got to here, we didn't have one matching. This could mean:
		//  0. We have a blank frame available
		//	1. We've skipped backwards
		//	2. We're onto the next frame
		//	3. We're skipping frames




		//--
		//0. We have frame buffers ready for use
		//--
		//
		for (auto & frameBuffer : this->frameBuffers) {
			if (frameBuffer->getFrameIndex() == 0) {
				frameBuffer->setFrameIndex(frameIndex);
				//this is a blank buffer, use this one
				return *frameBuffer;
			}
		}
		//
		//--


		long earliestFrameIndex = std::numeric_limits<long>::max();
		long latestFrameIndex = std::numeric_limits<long>::min();
		shared_ptr<FrameBuffer> earliestFrame;
		shared_ptr<FrameBuffer> latestFrame;
		for (const auto & frameBuffer : frameBuffers) {
			if (frameBuffer->getFrameIndex() < earliestFrameIndex) {
				earliestFrameIndex = frameBuffer->getFrameIndex();
				earliestFrame = frameBuffer;
			}
			if (frameBuffer->getFrameIndex() > latestFrameIndex) {
				latestFrameIndex = frameBuffer->getFrameIndex();
				latestFrame = frameBuffer;
			}
		}


		//--
		//1. We've skipped backwards
		//--
		//
		// Remember, the packet must have been declared as valid already to arrive here!
		if (frameIndex < earliestFrameIndex) {
			//clear all frames
			for (const auto & frameBuffer : this->frameBuffers) {
				frameBuffer->clear();
			}
			auto frameBuffer = this->frameBuffers.front();
			frameBuffer->setFrameIndex(frameIndex);
			return *frameBuffer;
		}
		//
		//--



		//--
		//2. We're skipping frames
		//--
		//
		// This should be the only possible.
		// This is also true even if frameIndex = latestFrame - 1
		// (since we've established that no frames are clear right now)
		for (int i = latestFrameIndex; i < frameIndex; i++) {
			DroppedFrame droppedFrame = {
				DroppedFrame::SkippedFrame,
				0, 0
			};
			this->droppedFrames.send(droppedFrame);
		}

		//we clear the oldest frame and use that
		earliestFrame->clear();
		earliestFrame->setFrameIndex(frameIndex);
		return *earliestFrame;
		//
		//--
	}

	//---------
	void FrameBufferSetTCP::callbackDroppedFrame(DroppedFrame & droppedFrame) {
		this->droppedFrames.send(droppedFrame);
	}

	//---------
	void FrameBufferSetTCP::dataGramProcessorLoop() {
		while (this->threadRunning) {
			shared_ptr<ofxAsio::DataGram> dataGram;
			while (this->socketToFrameBuffers.receive(dataGram)) {
				auto & message = dataGram->getMessage();
				if (message.empty()) {
					continue;
				}
				Packet packet(message);

				if (this->isExpired(packet.header.frameIndex)) {
					continue;
				}

				auto & frameBuffer = this->getFrameBuffer(packet.header.frameIndex);
				frameBuffer.add(packet);
			}
		}
	}

#pragma mark FrameBufferSetUDP
	//---------
	FrameBufferSetUDP::FrameBufferSetUDP() {
		for (int i = 0; i < 3; i++) {
			auto frameBuffer = make_shared<FrameBufferUDP>(this->decompressorToFrameReceiver);
			this->frameBuffers.emplace_back(frameBuffer);
			frameBuffer->onDroppedFrame.add(this, &FrameBufferSetUDP::callbackDroppedFrame, 0);
		}

		this->dataGramProcessorThread = thread([this]() {
			this->dataGramProcessorLoop();
		});
	}

	//---------
	FrameBufferSetUDP::~FrameBufferSetUDP() {
		this->socketToFrameBuffers.close();
	}

	//---------
	FrameBuffer & FrameBufferSetUDP::getFrameBuffer(uint32_t frameIndex) {
		for (auto & frameBuffer : this->frameBuffers) {
			if (frameBuffer->getFrameIndex() == frameIndex) {
				return *frameBuffer;
			}
		}

		//if we got to here, we didn't have one matching
		auto maxDistance = 0;
		auto minDistance = numeric_limits<int>::max();
		shared_ptr<FrameBuffer> furthest, closest;
		for (auto & frameBuffer : this->frameBuffers) {
			auto distance = abs((int)frameBuffer->getFrameIndex() - (int)frameIndex);
			if (distance > maxDistance) {
				maxDistance = distance;
				furthest = frameBuffer;
			}
			if (distance < minDistance) {
				minDistance = distance;
				closest = frameBuffer;
			}
		}

		//notify dropped frames if we're going forwards
		if (minDistance < OFXSQUASHBUDDIES_MAX_FRAME_DISCONTINUITY && closest->getFrameIndex() < frameIndex) {
			DroppedFrame droppedFrame = {
				DroppedFrame::Reason::SkippedFrame,
				0,
				0
			};
			this->droppedFrames.send(droppedFrame);
		}
		furthest->clear();
		furthest->setFrameIndex(frameIndex);
		return *furthest;
	}

	//---------
	void FrameBufferSetUDP::callbackDroppedFrame(DroppedFrame & droppedFrame) {
		this->droppedFrames.send(droppedFrame);
	}

	//---------
	void FrameBufferSetUDP::dataGramProcessorLoop() {
		while (this->threadRunning) {
			shared_ptr<ofxAsio::UDP::DataGram> dataGram;
			while (this->socketToFrameBuffers.receive(dataGram)) {
				auto & message = dataGram->getMessage();
				if (message.empty()) {
					continue;
				}
				Packet packet(message);

				if (this->isExpired(packet.header.frameIndex)) {
					continue;
				}

				auto & frameBuffer = this->getFrameBuffer(packet.header.frameIndex);
				frameBuffer.add(packet);
			}
		}
	}
}
