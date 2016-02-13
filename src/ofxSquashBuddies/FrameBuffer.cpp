#include "FrameBuffer.h"

namespace ofxSquashBuddies {
#pragma mark FrameBuffer
	//---------
	FrameBuffer::FrameBuffer() {
		this->clear();

		this->stream = make_unique<ofxSquash::Stream>(ofxSquash::Codec("density"), ofxSquash::Decompress);
		this->stream->setWriteFunction([this](const ofxSquash::WriteFunctionArguments & args) {

		});

		this->decompressThread = thread([this]() {
			this->decompressLoop();
		});
	}

	//---------
	FrameBuffer::~FrameBuffer() {
		this->threadRunning = false;
		this->bufferToDecompressor.close();
		this->decompressThread.join();
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
		//store the packet
		this->packets.emplace(packet.header.frameIndex, make_unique<Packet>(packet));

		//pass any sequential packets to the decompressor
		auto firstPacketInBuffer = this->packets.begin();
		while (this->packetIndexPosition == firstPacketInBuffer->first) {
			this->bufferToDecompressor.send(* firstPacketInBuffer->second.get());
			this->packets.erase(firstPacketInBuffer);
			firstPacketInBuffer = this->packets.begin();
			this->packetIndexPosition++;
		}
	}

	//---------
	void FrameBuffer::clear() {
		this->frameBuffer.clear();
		this->packets.clear();
		this->frameIndex = 0;
		this->packetIndexPosition = 0;
	}

	//---------
	void FrameBuffer::decompressLoop() {
		while (this->threadRunning) {
			Packet packet;
			if (this->bufferToDecompressor.receive(packet)) {
				this->stream->read((const void*)packet.payload, packet.header.payloadSize);

				if (packet.header.isLastPacket)
				{
					(* this->stream) << ofxSquash::Stream::Finish();
				}
			}
		}
	}

#pragma mark FrameBufferSet
	//---------
	FrameBuffer & FrameBufferSet::getFrameBuffer(uint32_t frameIndex) {
		if (A.getFrameIndex() == frameIndex) {
			return A;
		}
		else if (B.getFrameIndex() == frameIndex) {
			return B;
		}
		else {
			//we need to reassign one of our frames for this new frameIndex
			//generally frameIndex increases 1 per frame, but it may also jump backwards (e.g. if a client restarts)

			auto currentA = this->A.getFrameIndex();
			auto currentB = this->B.getFrameIndex();

			auto currentMaxFrameIndex = max<uint32_t>(currentA, currentB);
			auto currentMinFrameIndex = min<uint32_t>(currentA, currentB);
			auto BisAfterA = currentB >= currentA;

			FrameBuffer * frameToUse;

			if (frameIndex > currentMaxFrameIndex) {
				//frameIndex is increasing
				frameToUse = BisAfterA ? &this->A : &this->B;
			}
			else {
				//frameIndex skipped backwards, e.g. client restarts
				frameToUse = BisAfterA ? &this->B : &this->A;
			}

			frameToUse->clear();
			frameToUse->setFrameIndex(frameIndex);
			return *frameToUse;
		}
	}

	//---------
	bool FrameBufferSet::isExpired(uint32_t frameIndex) const {
		auto currentA = this->A.getFrameIndex();
		auto currentB = this->B.getFrameIndex();
		if (currentA == frameIndex) {
			return false;
		}
		else if (currentB == frameIndex) {
			return false;
		}
		else {
			auto currentMinFrameIndex = min<uint32_t>(this->A.getFrameIndex(), this->B.getFrameIndex());

			if (frameIndex < currentMinFrameIndex) {
				//we're skipping backwards
				if (frameIndex + 30 < currentMinFrameIndex) {
					//ignore up to 30 before the current active frame index
					return true;
				}
			}

			return false;
		}
	}
}