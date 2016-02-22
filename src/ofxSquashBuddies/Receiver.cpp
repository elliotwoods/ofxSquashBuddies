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
	bool Receiver::init(int port) {
		this->close();

		try {
			this->socket = make_shared<ofxAsio::UDP::Server>(port);
			this->port = port;
		}
		catch (std::exception & e) {
			OFXSQUASH_ERROR << "Failed to open Receiver on port " << port << " : " << e.what();
			return false;
		}

		this->frameBuffers.setCodec(this->codec);
		this->frameBuffers.decompressorToFrameReceiver.reset();

		this->threadsRunning = true;
		for (int i = 0; i < OFXSQUASHBUDDIES_RECEIVETHREADCOUNT; i++) {
			this->socketThread[i] = thread([this]() {
				this->socketLoop();
			});
		}
		this->frameReceiverThread = thread([this]() {
			this->frameReceiverLoop();
		});

		return true;
	}

	//---------
	void Receiver::close() {
		this->threadsRunning = false;

		//close socketThread
		if (this->socket) {
			this->socket->close();
		}
		for (int i = 0; i < OFXSQUASHBUDDIES_RECEIVETHREADCOUNT; i++) {
			if (this->socketThread[i].joinable()) {
				this->socketThread[i].join();
			}
		}
		if (this->socket) {
			this->socket.reset();
		}

		//close frameReceiverThread
		this->frameBuffers.decompressorToFrameReceiver.close();
		if (this->frameReceiverThread.joinable()) {
			this->frameReceiverThread.join();
		}

		this->port = 0;
	}

	//----------
	int Receiver::getPort() const {
		return this->port;
	}

	//----------
	ofxAsio::UDP::Server & Receiver::getSocketServer() {
		return *this->socket;
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

		//bring in which data frames were dropped this application frame
		this->droppedFrames.clear();
		DroppedFrame droppedFrame;
		while (this->frameBuffers.droppedFrames.tryReceive(droppedFrame)) {
			this->droppedFrames.push_back(move(droppedFrame));
		}

		//add incoming fps to frame rate timer, and update the frame rate
		{
			chrono::high_resolution_clock::time_point timePoint;
			while (this->frameReceiverToFrameRateCalculator.tryReceive(timePoint)) {
				this->incomingFrameRateCounter.addFrame(timePoint);
			}
		}
		this->incomingFrameRateCounter.update();
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

	//---------
	vector<DroppedFrame> Receiver::getDroppedFrames() const {
		return this->droppedFrames;
	}

	//----------
	float Receiver::getIncomingFramerate() const {
		return this->incomingFrameRateCounter.getFrameRate();
	}

#pragma mark protected
	//---------
	void Receiver::asyncCallback(shared_ptr<ofxAsio::UDP::DataGram> dataGram) {
		this->frameBuffers.socketToFrameBuffers.send(dataGram);
	}

	//---------
	void Receiver::socketLoop() {
		while (this->threadsRunning) {
			auto dataGram = socket->receive(Packet::PacketAllocationSize * 2);
			if (dataGram) {
				this->frameBuffers.socketToFrameBuffers.send(dataGram);
			}
		}
	}

	//---------
	void Receiver::frameReceiverLoop() {
		while (this->threadsRunning) {
			Message message;
			if (this->frameBuffers.decompressorToFrameReceiver.receive(message)) {
				this->onMessageReceiveThreaded.notify(this, message);
				this->frameReceiverToApp.send(move(message));
				this->frameReceiverToFrameRateCalculator.send(chrono::high_resolution_clock::now());
			}
		}
	}
}