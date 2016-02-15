#include "Receiver.h"

namespace ofxSquashBuddies {
#pragma mark public
	//---------
	Receiver::Receiver() {
		this->setCodec(this->getDefaultCodec());
	}

	//---------
	Receiver::~Receiver() {
		this->close();
	}

	//---------
	void Receiver::init(int port) {
		this->close();

		this->socket = make_shared<ofxAsio::UDP::Server>(port);

// 		this->socket->asyncReceiveAll([this](ofxAsio::UDP::Socket::AsyncArguments args) {
// 			if (args.success) {
// 				this->asyncCallback(args.dataGram);
// 			}
// 		}, Packet::PacketSize * 2);

		this->frameBuffers.setCodec(this->codec);

		this->threadsRunning = true;
 		this->socketThread = thread([this]() {
 			this->socketLoop();
 		});
		this->frameReceiverThread = thread([this]() {
			this->frameReceiverLoop();
		});
	}

	//---------
	void Receiver::close() {
		this->threadsRunning = false;
		
		//close threads
		if (this->socketThread.joinable()) {
			this->socketThread.join();
		}
		if (this->frameReceiverThread.joinable()) {
			this->frameReceiverThread.join();
		}
	}

	//---------
	void Receiver::setCodec(const ofxSquash::Codec & codec) {
		this->codec = codec;
		this->frameBuffers.setCodec(this->codec);
	}

	//---------
	const ofxSquash::Codec & Receiver::getCodec() const {
		return this->codec;
	}

	//---------
	void Receiver::update() {
		bool newFrameReceived = false;
		Message message;
		while (this->frameReceiverToApp.tryReceive(message)) {
			this->onMessageReceive.notify(this, message);
			newFrameReceived = true;
		}
		if (newFrameReceived) {
			this->message = message;
			this->frameNew = true;
		}
		else {
			this->frameNew = false;
		}
	}

	//---------
	bool Receiver::isFrameNew() const
	{
		return this->frameNew;
	}

	//---------
	const Message & Receiver::getMessage() const {
		return this->message;
	}

	//---------
	Message Receiver::getNextMessage(uint64_t timeoutMS) {
		Message message;
		if (this->frameReceiverToApp.tryReceive(message, timeoutMS)) {
			this->message = message;
			this->frameNew = true;
		}
		return message;
	}

	//---------
	bool Receiver::receive(string & data) {
		if (this->message.empty()) {
			OFXSQUASHBUDDIES_WARNING << "Cannot receive. Message is empty";
			return false;
		}
		else {
			return this->message.getData(data);
		}
	}

	//---------
	bool Receiver::receive(ofPixels & data) {
		if (this->message.empty()) {
			OFXSQUASHBUDDIES_WARNING << "Cannot receive. Message is empty";
			return false;
		}
		else {
			return this->message.getData(data);
		}
	}

	//---------
	bool Receiver::receive(ofMesh & data) {
		if (this->message.empty()) {
			OFXSQUASHBUDDIES_WARNING << "Cannot receive. Message is empty";
			return false;
		}
		else {
			return this->message.getData(data);
		}
	}

#pragma mark protected
	//---------
	void Receiver::asyncCallback(shared_ptr<ofxAsio::UDP::DataGram> dataGram) {
		this->frameBuffers.socketToFrameBuffers.send(dataGram);
	}

	//---------
	void Receiver::socketLoop() {
		while (this->threadsRunning) {
			auto dataGram = socket->receive();
			this->frameBuffers.socketToFrameBuffers.send(dataGram);
		}
	}

	//---------
	void Receiver::frameReceiverLoop() {
		while (this->threadsRunning) {
			Message message;
			if (this->frameBuffers.decompressorToFrameReceiver.receive(message)) {
				this->onMessageReceiveThreaded.notify(this, message);
				this->frameReceiverToApp.send(message);
			}
		}
	}
}