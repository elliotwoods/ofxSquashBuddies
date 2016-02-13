#pragma once

#include "ThingsInCommon.h"
#include "FrameBuffer.h"

#include "ofThreadChannel.h"
#include "ofEvent.h"

namespace ofxSquashBuddies {
	class Receiver : public ThingsInCommon {
	public:
		~Receiver();

		void init(int port);
		void close();

		void update();
		bool isFrameNew() const;
		bool isFrameIncoming() const;

		template<typename Type>
		bool receive(Type &);

		template<typename Type>
		bool receiveNext(Type &, const chrono::milliseconds & timeOut);

	protected:
		atomic_bool threadsRunning;
		thread socketThread;

		shared_ptr<ofxAsio::UDP::Server> socket;
		shared_ptr<ofThreadChannel<string>> decompressorToApp;

		atomic_bool frameNew;

		void socketLoop();

	protected:
		shared_ptr<ofxSquash::Stream> stream;
		FrameBufferSet frameBuffers;
	};

	//==========

	template<typename Type>
	bool ofxSquashBuddies::Receiver::receive(Type &)
	{

	}

	template<typename Type>
	bool ofxSquashBuddies::Receiver::receiveNext(Type &, const chrono::milliseconds & timeOut)
	{

	}
}