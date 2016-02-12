#include "Receiver.h"

namespace ofxSquashBuddies {
	//---------
	Receiver::~Receiver() {
		this->close();
	}

	//---------
	void Receiver::init(int port) {

		this->close();

		this->decompressorToApp = make_shared<ofThreadChannel<string>>();

		this->socket = make_shared<ofxAsio::UDP::Server>(port);

		this->threadsRunning = true;
		this->socketThread = thread([this]() {
			this->socketLoop();
		});
	}

	//---------
	void Receiver::close() {
		this->threadsRunning = false;
		if (this->decompressorToApp) {
			this->decompressorToApp->close();
			this->decompressorToApp.reset();
		}

		if (this->socketThread.joinable()) {
			this->socketThread.join();
		}
	}

	//---------
	void Receiver::update()
	{

	}

	//---------
	bool Receiver::isFrameNew() const
	{
		return false;
	}

	//---------
	bool Receiver::isFrameIncoming() const
	{
		return false;
	}

	//---------
	void Receiver::socketLoop() {
		cout << "thread start" << endl;

		while (this->threadsRunning) {
			auto dataGram = socket->receive();
			if (!dataGram)
				continue;

			if (dataGram->getMessage().empty()) {
				continue;
			}

			Packet packet(dataGram->getMessage());

			if (this->frameBuffers.isExpired(packet.header.frameIndex)) {
				//this packet isn't relevant to what we're working on
				//drop the packet
				return;
			}

			auto & frameBuffer = this->frameBuffers.getFrameBuffer(packet.header.frameIndex);
			frameBuffer.add(packet);
		}

		cout << "thread end" << endl;
	}
}