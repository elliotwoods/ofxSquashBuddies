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

		virtual void setCodec(const ofxSquash::Codec &) = 0;

		void setFrameIndex(uint32_t);
		uint32_t getFrameIndex() const;

		void add(const Packet &);
		virtual void clear() {};

		ofEvent<DroppedFrame> onDroppedFrame;

	protected:
		bool threadsRunning = true;

		ofxSquash::Codec codec = ThingsInCommon::getDefaultCodec();

		bool success = false;
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

	class FrameBufferTCP : public FrameBuffer {
	public:
		FrameBufferTCP(ThreadChannel<Message> &);
		virtual void setCodec(const ofxSquash::Codec &) override;

		virtual void clear() override;

	protected:
	};

	class FrameBufferUDP : public FrameBuffer {
	public:
		FrameBufferUDP(ThreadChannel<Message> &);
		virtual void setCodec(const ofxSquash::Codec &) override;

		virtual void clear() override;

	protected:
	};

	class FrameBufferSet {
	public:
		virtual ~FrameBufferSet();

		void setCodec(const ofxSquash::Codec &);

		virtual FrameBuffer & getFrameBuffer(uint32_t frameIndex) = 0;

		bool isExpired(uint32_t frameIndex) const;

		ThreadChannel<Message> decompressorToFrameReceiver;
		ofThreadChannel<DroppedFrame> droppedFrames;

		const vector<shared_ptr<FrameBuffer>> & getFrameBuffers() const;

	protected:
		vector<shared_ptr<FrameBuffer>> frameBuffers;

		thread dataGramProcessorThread; // consumes socketToFrameBuffers
		bool threadRunning = true;

		virtual void dataGramProcessorLoop() = 0;
	};

	class FrameBufferSetTCP : public FrameBufferSet {
	public:
		FrameBufferSetTCP();
		~FrameBufferSetTCP();

		virtual FrameBuffer & getFrameBuffer(uint32_t frameIndex) override;

		ofThreadChannel<shared_ptr<ofxAsio::DataGram>> socketToFrameBuffers;

	protected:
		void callbackDroppedFrame(DroppedFrame &);

		virtual void dataGramProcessorLoop() override;
	};

	class FrameBufferSetUDP : public FrameBufferSet {
	public:
		FrameBufferSetUDP();
		~FrameBufferSetUDP();

		virtual FrameBuffer & getFrameBuffer(uint32_t frameIndex) override;

		ofThreadChannel<shared_ptr<ofxAsio::UDP::DataGram>> socketToFrameBuffers;

	protected:
		void callbackDroppedFrame(DroppedFrame &);

		virtual void dataGramProcessorLoop() override;
	};
}