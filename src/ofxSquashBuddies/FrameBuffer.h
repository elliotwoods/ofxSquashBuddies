#pragma once

#include "Packet.h"
#include "Message.h"
#include "ThingsInCommon.h"

#include "ThreadChannel.h"
#include "ofThreadChannel.h"
#include "ofEvent.h"
#include "ofxSquash/Stream.h"
#include "ofxAsio/UDP/DataGram.h"

#include <string>
#include <map>
#include <memory>
#include <thread>

using namespace std;

//the maximum positive discontinuity in incoming frame indexes before we consider this a skip forwards
#define OFXSQUASHBUDDIES_MAX_FRAME_DISCONTINUITY 30

namespace ofxSquashBuddies {
	struct DroppedFrame {
		enum Reason {
			DroppedPackets,
			SkippedFrame
		} reason;
		size_t packetCount;
		size_t lastPacketIndex;
	};

	class FrameBuffer {
	public:
		FrameBuffer(ThreadChannel<Message> &);
		~FrameBuffer();

		void setCodec(const ofxSquash::Codec &);

		void setFrameIndex(uint32_t);
		uint32_t getFrameIndex() const;

		void add(const Packet &);
		void clear();

		ofEvent<DroppedFrame> onDroppedFrame;

	protected:
		bool threadsRunning = true;

		ofxSquash::Codec codec = ThingsInCommon::getDefaultCodec();

		int32_t packetIndexPosition = 0;
		ofThreadChannel<Packet> packetsToDecompressor;

		void decompressLoop();
		thread decompressThread; // consumes packetsToDecompressor

		map<uint16_t, unique_ptr<Packet>> packets;
		mutex packetsMutex;

		unique_ptr<ofxSquash::Stream> stream;
		mutex streamMutex;

		Message message;
		mutex messageMutex;

		void writeFunction(const ofxSquash::WriteFunctionArguments &);
		ThreadChannel<Message> & decompressorToFrameReceiver;

		uint32_t frameIndex = 0;
	};

	class FrameBufferSet {
	public:
		FrameBufferSet();
		~FrameBufferSet();

		void setCodec(const ofxSquash::Codec &);

		FrameBuffer & getFrameBuffer(uint32_t frameIndex);
		bool isExpired(uint32_t frameIndex) const;

		ofThreadChannel<shared_ptr<ofxAsio::DataGram>> socketToFrameBuffers;
		ThreadChannel<Message> decompressorToFrameReceiver;
		ofThreadChannel<DroppedFrame> droppedFrames;
	protected:
		void callbackDroppedFrame(DroppedFrame &);
		vector<shared_ptr<FrameBuffer>> frameBuffers;

		thread dataGramProcessorThread; // consumes socketToFrameBuffers
		void dataGramProcessorLoop();
		bool threadRunning = true;
	};
}