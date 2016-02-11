#pragma once

#include "ThingsInCommon.h"

namespace ofxSquashBuddies {
	template<typename Type>
	class Sender {
	public:
		~Sender();
		void init(string address, int port);
		void close();

		void send(Type &);
	protected:
		void threadedFunction();

		bool threadRunning = false;
		thread sendThread;

		shared_ptr<ofxAsio::UDP::Socket> socket;
	};
}