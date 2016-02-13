#pragma once

#include "ThingsInCommon.h"
#include "FrameBuffer.h"

#include "ofThreadChannel.h"
#include "ofEvent.h"

namespace ofxSquashBuddies {
	class Receiver : public ThingsInCommon {
	public:
		Receiver();
		~Receiver();

		void init(int port);
		void close();

		void setCodec(const ofxSquash::Codec &) override;
		const ofxSquash::Codec & getCodec() const override;

		void update();

		bool isFrameNew() const;
		const Message & getMessage() const;
		Message getNextMessage(uint64_t timeoutMS);

		bool receive(string &);
		bool receive(ofPixels &);

		ofEvent<Message> onMessageReceive;
		ofEvent<Message> onMessageReceiveThreaded;
	protected:
		atomic_bool threadsRunning;

		ofxSquash::Codec codec;

		thread socketThread;
		void socketLoop();
		shared_ptr<ofxAsio::UDP::Server> socket;

		FrameBufferSet frameBuffers;

		thread frameReceiverThread;
		void frameReceiverLoop();
		ofThreadChannel<Message> frameReceiverToApp;

		Message message;
		atomic_bool frameNew;
	};
}
