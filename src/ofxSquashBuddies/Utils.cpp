#include "Utils.h"

using namespace std;

namespace ofxSquashBuddies {
	namespace Utils {
		//----------
		void FramerateCounter::update() {
			if (!this->frameTimes.empty()) {
				auto now = chrono::high_resolution_clock::now();
				auto old = this->frameTimes.front();
				auto size = this->frameTimes.size();
				while (this->frameTimes.size() > 30) {
					this->frameTimes.pop();
				}
				this->frameRate = (double)(size - 1) * 1e6 / (double)chrono::duration_cast<chrono::microseconds>(now - old).count();
			}
		}

		//----------
		void FramerateCounter::addFrame(chrono::high_resolution_clock::time_point timePoint) {
			this->frameTimes.push(timePoint);
		}

		//----------
		float FramerateCounter::getFrameRate() const {
			return this->frameRate;
		}
	}
}