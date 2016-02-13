#pragma once

#include "Packet.h"

#include "ofThreadChannel.h"
#include "ofxSquash/Stream.h"

#include <string>
#include <map>
#include <memory>
#include <thread>

using namespace std;

namespace ofxSquashBuddies {
	class FrameBuffer {
	public:
		FrameBuffer();
		~FrameBuffer();

		void setFrameIndex(uint32_t);
		uint32_t getFrameIndex() const;

		void add(const Packet &);
		void clear();
	protected:
		thread decompressThread;
		ofThreadChannel<Packet> bufferToDecompressor;
		void decompressLoop();
		bool threadRunning = true;
		unique_ptr<ofxSquash::Stream> stream;

		map<uint16_t, unique_ptr<Packet>> packets;
		string frameBuffer;

		int32_t packetIndexPosition = 0;
		uint32_t frameIndex = 0;
	};

	class FrameBufferSet {
	public:
		FrameBuffer & getFrameBuffer(uint32_t frameIndex);

		bool isExpired(uint32_t frameIndex) const;
	protected:
		FrameBuffer A, B;
	};
}