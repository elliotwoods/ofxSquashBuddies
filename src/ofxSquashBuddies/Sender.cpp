#include "Sender.h"
#include "Constants.h"

namespace ofxSquashBuddies {
	//----------
	Sender::Sender() {
		this->setCodec(this->getDefaultCodec());
	}

	//----------
	Sender::~Sender() {
		this->close();
	}

	//----------
	void Sender::init(string ipAddress, int port) {
		this->close();

		//CHECK IF INIT IS CALLED BEFORE USING THREAD CHANNELS!!!
		//or will they not be used at all if init isn't called?

		//recreate the thread channels
		this->appToCompressor = make_shared<ofThreadChannel<Message>>();
		this->compressorToSocket = make_shared<ofThreadChannel<Packet>>();

		this->socket = make_shared<ofxAsio::UDP::Client>();

		{
			auto lock = unique_lock<mutex>(this->configMutex);
			this->config.endPoint = ofxAsio::UDP::EndPoint(ipAddress, port);
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
	void Sender::close() {
		this->threadsRunning = false;

		if (this->appToCompressor) {
			this->appToCompressor->close();
			this->appToCompressor.reset();
		}
		if (this->compressorToSocket) {
			this->compressorToSocket->close();
			this->compressorToSocket.reset();
		}

		if (this->compressThread.joinable()) {
			this->compressThread.join();
		}
		if (this->socketThread.joinable()) {
			this->socketThread.join();
		}
	}

	//----------
	void Sender::setCodec(const ofxSquash::Codec & codec) {
		this->codec = codec;
	}

	//----------
	const ofxSquash::Codec & Sender::getCodec() const {
		return this->codec;
	}

	//----------
	void Sender::send(const void * data, size_t size) {
		this->appToCompressor->send(Message(data, size));
	}

	//----------
	void Sender::send(const string & data) {
		this->appToCompressor->send(Message(data));
	}

	//----------
	void Sender::send(const ofPixels & data) {
		this->send(Message(data));
	}

	//----------
	void Sender::send(const ofMesh & data) {
		this->send(Message(data));
	}

	//----------
	void Sender::send(const Message & message) {
		this->appToCompressor->send(message);
	}

	//----------
	void Sender::compressLoop() {
		uint32_t frameIndex = 0;
		while (this->threadsRunning) {
			Message message;
			while (this->appToCompressor->receive(message)) {
				Packet packet;
				packet.header.packetIndex = 0;
				packet.header.frameIndex = frameIndex;

				struct {
					size_t offset = 0;
					size_t availableBytes = Packet::MaxPayloadSize;
				} payloadState;

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
								payloadState.availableBytes = Packet::MaxPayloadSize;
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
	void Sender::socketLoop() {
		while (this->threadsRunning) {
			this->configMutex.lock();
			auto config = this->config;
			this->configMutex.unlock();

			Packet packet;
			while (this->compressorToSocket->receive(packet)) {
				if (this->socket) {
					auto dataGram = make_shared<ofxAsio::UDP::DataGram>();
					dataGram->setEndPoint(config.endPoint);

					dataGram->getMessage().set(&packet, sizeof(packet));
					this->socket->send(dataGram);
				}
				else {
					OFXSQUASHBUDDIES_WARNING << "Socket not connected, cannot send packets.";
				}
			}
		}
	}
}