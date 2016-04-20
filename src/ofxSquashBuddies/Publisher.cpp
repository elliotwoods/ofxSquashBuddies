#include "Publisher.h"
#include "Constants.h"

namespace ofxSquashBuddies {
	//----------
	Publisher::Publisher() {
		this->setCodec(this->getDefaultCodec());
	}

	//----------
	Publisher::~Publisher() {
		this->close();
	}

	//----------
	void Publisher::init(int port) {
		this->close();

		//recreate the thread channels
		this->appToCompressor = make_shared<ThreadChannel<Message>>();
		this->compressorToSocket = make_shared<ThreadChannel<Packet>>();

		try {
			this->socket = make_unique<zmq::socket_t>(this->context, ZMQ_PUB);
			int highWaterMark = 100;
			this->socket->setsockopt(ZMQ_SNDHWM, &highWaterMark, sizeof(highWaterMark));
			this->socket->bind("tcp://*:" + ofToString(port));
		}
		catch (std::exception & e) {
			OFXSQUASH_ERROR << "Failed to Publish on port " << port << " : " << e.what();
		}

		this->threadsRunning = true;
		this->compressThread = thread([this]() {
			this->compressLoop();
		});
		this->socketThread = thread([this]() {
			this->socketLoop();
		});
	}

	//----------
	void Publisher::close() {
		this->threadsRunning = false;

		if (this->appToCompressor) {
			this->appToCompressor->close();
		}
		if (this->compressorToSocket) {
			this->compressorToSocket->close();
		}

		if (this->compressThread.joinable()) {
			this->compressThread.join();
		}
		if (this->socketThread.joinable()) {
			this->socketThread.join();
		}
		if (this->socket) {
			this->socket->close();
			this->socket.reset();
		}

		this->appToCompressor.reset();
		this->compressorToSocket.reset();
	}

	//----------
	void Publisher::setCodec(const ofxSquash::Codec & codec) {
		this->codec = codec;
	}

	//----------
	const ofxSquash::Codec & Publisher::getCodec() const {
		return this->codec;
	}

	//----------
	bool Publisher::send(const void * data, size_t size) {
		return this->send(move(Message(data, size)));
	}

	//----------
	bool Publisher::send(const string & data) {
		return this->send(move(Message(data)));
	}

	//----------
	bool Publisher::send(const ofPixels & data) {
		return this->send(move(Message(data)));
	}

	//----------
	bool Publisher::send(const ofMesh & data) {
		return this->send(move(Message(data)));
	}

	//----------
	bool Publisher::send(const Message & message) {
		auto messageCopy = message;
		return this->send(move(messageCopy));
	}

	//----------
	bool Publisher::send(Message && message) {
		if (!this->appToCompressor) {
			OFXSQUASHBUDDIES_ERROR << "You cannot call send before you call init";
			return false;
		}
		else {
			if (this->compressorToSocket->size() < this->maxSocketBufferSize && this->appToCompressor->size() < this->maxCompressorQueueSize) {
				auto success = this->appToCompressor->send(move(message));
				if (success) {
					this->sendFramerateCounter.addFrame();
					this->sendFramerateCounter.update();
					return true;
				}
				else {
					return false;
				}
			}
			else {
				return false;
			}
		}
	}

	//----------
	float Publisher::getSendFramerate() const {
		return this->sendFramerateCounter.getFrameRate();
	}

	//----------
	void Publisher::setMaxSocketBufferSize(size_t maxSocketBufferSize) {
		this->maxSocketBufferSize = maxSocketBufferSize;
	}

	//----------
	size_t Publisher::getMaxSocketBufferSize() const {
		return this->maxSocketBufferSize;
	}

	//----------
	size_t Publisher::getCurrentSocketBufferSize() const {
		return this->compressorToSocket->size();
	}

	//----------
	void Publisher::setMaxCompressorQueueSize(size_t maxCompressorQueueSize) {
		this->maxCompressorQueueSize = maxCompressorQueueSize;
	}

	//----------
	size_t Publisher::getMaxCompressorQueueSize() const {
		return this->maxCompressorQueueSize;
	}

	//----------
	size_t Publisher::getCurrentCompressorQueueSize() const {
		return this->appToCompressor->size();
	}

	//----------
	size_t Publisher::getPacketSize() const {
		return this->packetSize;
	}

	//----------
	void Publisher::setPacketSize(size_t packetSize) {
		if (packetSize > Packet::PacketAllocationSize) {
			OFXSQUASHBUDDIES_WARNING << "Cannot set packet size to [" << packetSize << "] as it is higher than Packet::PacketSize";
			packetSize = Packet::PacketAllocationSize;
		}
		if (packetSize < Packet::HeaderSize + 16) {
			OFXSQUASHBUDDIES_WARNING << "Cannot set packet size to [" << packetSize << "] as it is lower than the minimum packet size";
			packetSize = Packet::HeaderSize + 16;
		}
		this->packetSize = packetSize;
	}

	//----------
	void Publisher::compressLoop() {
		uint32_t frameIndex = 0;
		while (this->threadsRunning) {
			Message message;
			while (this->appToCompressor->receive(message)) {
				Packet packet;
				packet.header.packetIndex = 0;
				packet.header.frameIndex = frameIndex;

				struct {
					size_t offset = 0;
					size_t availableBytes;
				} payloadState;
				payloadState.availableBytes = this->packetSize - Packet::HeaderSize;

				if (!this->getCodec().isValid()) {
					OFXSQUASHBUDDIES_ERROR << "Codec [" << this->getCodec().getName() << "] is not valid. Are you sure you have the plugins installed correctly?";
					continue;
				}
				ofxSquash::Stream compressStream(this->getCodec(), ofxSquash::Direction::Compress, [this, &packet, &payloadState](const ofxSquash::WriteFunctionArguments & args) {
					//copy incoming data and split into packets

					struct {
						uint8_t * readPosition;
						size_t availableBytes;
					} inputState;
					inputState.readPosition = (uint8_t*)args.data;
					inputState.availableBytes = args.size;

					while (inputState.availableBytes > 0) {
						auto bytesToCopy = min<size_t>(inputState.availableBytes, payloadState.availableBytes);

						memcpy(packet.payload + payloadState.offset, inputState.readPosition, bytesToCopy);
						
						inputState.readPosition += bytesToCopy;
						inputState.availableBytes -= bytesToCopy;
						payloadState.offset += bytesToCopy;
						payloadState.availableBytes -= bytesToCopy;

						if (payloadState.availableBytes == 0) {
							//finish off the packet header and send whenever we have a full packet
							{
								if (payloadState.offset > numeric_limits<uint32_t>::max()) {
									OFXSQUASHBUDDIES_ERROR << "Payload is too big! Sorry baby.";
								}
								else {
									packet.header.payloadSize = (uint32_t)payloadState.offset;
									this->compressorToSocket->send(packet);
								}
								
								packet.header.packetIndex++;
								packet.header.payloadSize = 0;
							}

							//reset the packet for next use
							{
								payloadState.offset = 0;
								payloadState.availableBytes = this->packetSize - Packet::HeaderSize;
							}
						}
					}
				});

				compressStream << message.getMessageString() << ofxSquash::Stream::Finish();

				//send whatever is left over
				if (payloadState.offset > numeric_limits<uint32_t>::max()) {
					OFXSQUASHBUDDIES_ERROR << "Payload is too big! Sorry baby.";
				}
				else {
					packet.header.payloadSize = (uint32_t) payloadState.offset;
					packet.header.isLastPacket = true;
					this->compressorToSocket->send(packet);
				}

				frameIndex++;
			}
		}
	}

	//----------
	void Publisher::socketLoop() {
		while (this->threadsRunning) {

			Packet packet;
			while (this->compressorToSocket->receive(packet)) {
				if (this->socket) {
					zmq::message_t message(packet.size());
					memcpy(message.data(), &packet, packet.size());

					this->socket->send(message);
				}
				else {
					OFXSQUASHBUDDIES_WARNING << "Socket not connected, cannot send packets.";
				}
			}
		}
	}
}