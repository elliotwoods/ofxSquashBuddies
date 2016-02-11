#pragma once

#include "ThingsInCommon.h"

namespace ofxSquashBuddies {
	template<typename Type>
	class Receiver {
	public:
		struct ReceiveArguments {
			Type data;
		};

		void init(int port);

		void update();
		bool isFrameNew() const;

		bool receive(Type &);
		bool receiveNext(Type &, const chrono::duration & timeOut);

		// WARNING :Tthis will happen outside of the main thread
		ofEvent<ReceiveArguments> onReceive;
	protected:
		conditional_variable receiveWaiter;
		mutex mutex;
	};
}