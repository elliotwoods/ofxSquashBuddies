#pragma once

#include "ThingsInCommon.h"

#include "ofEvent.h"

namespace ofxSquashBuddies {
	class Receiver {
	public:
		struct ReceiveArguments {

		};

		void init(int port) { }

		void update() { }
		bool isFrameNew() const { return false;  }
		bool isFrameIncoming() const { return false;  }

		template<typename Type>
		bool receive(Type &) { return false;  }

		template<typename Type>
		bool receiveNext(Type &, const chrono::milliseconds & timeOut) { return false;  }

		// WARNING : This event will fire outside of the main thread
		ofEvent<ReceiveArguments> onReceive;
	protected:
	};
}