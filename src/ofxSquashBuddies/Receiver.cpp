#include "Receiver.h"

namespace ofxSquashBuddies {
	//----------
	bool receiveNext(Type & data, const chrono::duration & timeOut) {
		auto lock = unique_lock<mutex>(this->mutex);
		this->receiveWaiter.wait_for(lock, std::chrono::seconds(time))

		while (not timeout) {
			check for onReceive event
			ofSleepMillis(1);
		}
	}
}