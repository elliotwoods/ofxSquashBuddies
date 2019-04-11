#pragma once

#include "ofThreadChannel.h"
#include <chrono>

namespace ofxSquashBuddies {
	namespace Utils {
		class FramerateCounter {
		public:
			void update();
			void addFrame(std::chrono::high_resolution_clock::time_point = std::chrono::high_resolution_clock::now());
			float getFrameRate() const;
		protected:
			std::queue<std::chrono::high_resolution_clock::time_point> frameTimes;
			float frameRate = 0.0f;
		};
	}
}