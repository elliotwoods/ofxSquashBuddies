#pragma once

#include "Packet.h"
#include "Message.h"
#include "ThingsInCommon.h"

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
		FrameBuffer(ofThreadChannel<Message> &);
		~FrameBuffer();

		void setCodec(const ofxSquash::Codec &);

		void setFrameIndex(uint32_t);
		uint32_t getFrameIndex() const;

		void add(const Packet &);
		void clear();

	protected:
		bool threadsRunning = true;

		ofxSquash::Codec codec = ThingsInCommon::getDefaultCodec();

		ofThreadChannel<Packet> socketToPacketStore;

		void packetStoreLoop();
		thread packetStoreThread;
		map<uint16_t, unique_ptr<Packet>> packets;
		int32_t packetIndexPosition = 0;
		ofThreadChannel<Packet> bufferToDecompressor;

		void decompressLoop();
		thread decompressThread;
		unique_ptr<ofxSquash::Stream> stream;
		void writeFunction(const ofxSquash::WriteFunctionArguments &);
		Message message;
		ofThreadChannel<Message> & decompressorToFrameReceiver;

		uint32_t frameIndex = 0;
	};

	class FrameBufferSet {
	public:
		FrameBufferSet();

		void setCodec(const ofxSquash::Codec &);

		FrameBuffer & getFrameBuffer(uint32_t frameIndex);
		bool isExpired(uint32_t frameIndex) const;

		ofThreadChannel<Message> decompressorToFrameReceiver;
	protected:
		FrameBuffer A, B;
	};
}