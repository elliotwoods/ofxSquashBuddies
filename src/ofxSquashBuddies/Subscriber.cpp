#include "Subscriber.h"

namespace ofxSquashBuddies {
#pragma mark public
	//---------
	Subscriber::Subscriber() {
		this->setCodec(this->getDefaultCodec());
	}

	//---------
	Subscriber::~Subscriber() {
		this->close();
	}

	//---------
	bool Subscriber::init(string address, int port) {
		this->close();

		try {
			this->socket = make_unique<zmq::socket_t>(this->context, ZMQ_SUB);
			int highWaterMark = 3000;
			this->socket->setsockopt(ZMQ_RCVHWM, &highWaterMark, sizeof(highWaterMark)); 
			this->socket->connect("tcp://" + address + ":" + ofToString(port));
			this->socket->setsockopt(ZMQ_SUBSCRIBE, "", 0);
		}
		catch (std::exception & e) {
			OFXSQUASH_ERROR << "Failed subscribe to " << address << ":" << port << " : " << e.what();
			return false;
		}

		this->address = address;
		this->port = port;

		this->frameBuffers.setCodec(this->codec);
		this->frameBuffers.decompressorToFrameReceiver.reset();

		this->threadsRunning = true;
		this->socketThread = thread([this]() {
			this->socketLoop();
		});
		this->frameReceiverThread = thread([this]() {
			this->frameReceiverLoop();
		});

		return true;
	}

	//---------
	void Subscriber::close() {
		this->threadsRunning = false;

		//close socketThread
		if (this->socketThread.joinable()) {
			this->socketThread.join();
		}
		if (this->socket) {
			this->socket.reset();
		}

		this->address = "";
		this->port = -1;

		//close frameReceiverThread
		this->frameBuffers.decompressorToFrameReceiver.close();
		if (this->frameReceiverThread.joinable()) {
			this->frameReceiverThread.join();
		}
	}

	//---------
	const string & Subscriber::getAddress() const {
		return this->address;
	}
	
	//---------
	int Subscriber::getPort() const {
		return this->port;
	}

	//---------
	void Subscriber::setCodec(const ofxSquash::Codec & codec) {
		this->codec = codec;
		this->frameBuffers.setCodec(this->codec);
	}

	//---------
	const ofxSquash::Codec & Subscriber::getCodec() const {
		return this->codec;
	}

	//---------
	void Subscriber::update(const chrono::high_resolution_clock::duration & blockUntilNewFrameAvailable) {
		bool newFrameReceived = false;

		Message message;
		{
			//if we're supposed to block until there's something new, let's do that
			if (blockUntilNewFrameAvailable != chrono::milliseconds(0)) {
				if (this->frameReceiverToApp.tryReceive(message, chrono::duration_cast<chrono::milliseconds>(blockUntilNewFrameAvailable).count())) {
					this->onMessageReceive.notify(this, message);
					newFrameReceived = true;
				}
			}

			//pull all remaining frames in the buffer
			while (this->frameReceiverToApp.tryReceive(message)) {
				this->onMessageReceive.notify(this, message);
				newFrameReceived = true;
			}
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
	bool Subscriber::isFrameNew() const
	{
		return this->frameNew;
	}

	//---------
	const Message & Subscriber::getMessage() const {
		return this->message;
	}

	//---------
	Message Subscriber::getNextMessage(uint64_t timeoutMS) {
		Message message;
		if (this->frameReceiverToApp.tryReceive(message, timeoutMS)) {
			this->message = message;
			this->frameNew = true;
		}
		return message;
	}

	//---------
	bool Subscriber::receive(string & data) {
		if (this->message.empty()) {
			OFXSQUASHBUDDIES_WARNING << "Cannot receive. Message is empty";
			return false;
		}
		else {
			return this->message.getData(data);
		}
	}

	//---------
	bool Subscriber::receive(ofPixels & data) {
		if (this->message.empty()) {
			OFXSQUASHBUDDIES_WARNING << "Cannot receive. Message is empty";
			return false;
		}
		else {
			return this->message.getData(data);
		}
	}

	//---------
	bool Subscriber::receive(ofMesh & data) {
		if (this->message.empty()) {
			OFXSQUASHBUDDIES_WARNING << "Cannot receive. Message is empty";
			return false;
		}
		else {
			return this->message.getData(data);
		}
	}

	//---------
	vector<DroppedFrame> Subscriber::getDroppedFrames() const {
		return this->droppedFrames;
	}

	//----------
	void Subscriber::printDebug(const DroppedFrame & droppedFrame) {
		switch (droppedFrame.reason) {
		case ofxSquashBuddies::DroppedFrame::Reason::DroppedPackets:
			cout << "Dropped packets [" << droppedFrame.lastPacketIndex << "/" << droppedFrame.packetCount << "]" << endl;
			break;
		case ofxSquashBuddies::DroppedFrame::Reason::SkippedFrame:
		{
			cout << "Skipped frame" << endl;
			break;
		}
		default:
			cout << "Unknown" << endl;
		}
	}

	//----------
	float Subscriber::getIncomingFramerate() const {
		return this->incomingFrameRateCounter.getFrameRate();
	}

#pragma mark protected
	//---------
	void Subscriber::asyncCallback(shared_ptr<ofxAsio::UDP::DataGram> dataGram) {
		this->frameBuffers.socketToFrameBuffers.send(dataGram);
	}

	//---------
	void Subscriber::socketLoop() {
		while (this->threadsRunning) {
			if (this->socket) {
				auto dataGram = make_shared<ofxAsio::DataGram>();
				auto & message = dataGram->getMessage();

				message.resize(Packet::PacketAllocationSize * 2);
				auto receivedSize = this->socket->recv(message.data(), message.size(), ZMQ_DONTWAIT);
				if (receivedSize > 0) {
					this->frameBuffers.socketToFrameBuffers.send(dataGram);
				}
			}
			else {
				OFXSQUASHBUDDIES_WARNING << "Socket not connected, cannot receive packets.";
			}
		}
	}

	//---------
	void Subscriber::frameReceiverLoop() {
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