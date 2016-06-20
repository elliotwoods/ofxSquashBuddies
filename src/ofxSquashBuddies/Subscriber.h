#pragma once

#include "ThingsInCommon.h"
#include "FrameBuffer.h"
#include "Utils.h"

#include "ofxZmq.h"

#include "ofThreadChannel.h"
#include "ofEvent.h"

namespace ofxSquashBuddies {
	class Subscriber : public ThingsInCommon {
	public:
		Subscriber();
		~Subscriber();
			
		bool init(string address, int port);
		void close();

		const string & getAddress() const;
		int getPort() const;

		void setCodec(const ofxSquash::Codec &) override;
		const ofxSquash::Codec & getCodec() const override;

		void update(const chrono::high_resolution_clock::duration & blockUntilNewFrameAvailable = chrono::milliseconds(0));

		bool isFrameNew() const;

		const Message & getMessage() const;
		Message getNextMessage(uint64_t timeoutMS);

		bool receive(string &);
		bool receive(ofPixels &);
		bool receive(ofMesh &);

		vector<DroppedFrame> getDroppedFrames() const;
		static void printDebug(const DroppedFrame &);

		float getIncomingFramerate() const;

		ofEvent<Message> onMessageReceive;
		ofEvent<Message> onMessageReceiveThreaded;
	protected:
		void asyncCallback(shared_ptr<ofxAsio::UDP::DataGram>);

		string address;
		int port;

		atomic_bool threadsRunning;

		ofxSquash::Codec codec;

		thread socketThread;
		void socketLoop();

		zmq::context_t context;
		shared_ptr<zmq::socket_t> socket;

		FrameBufferSetTCP frameBuffers;

		thread frameReceiverThread; //consumes decompressorToFrameReceiver
		void frameReceiverLoop();
		ofThreadChannel<Message> frameReceiverToApp;

		Message message;
		atomic_bool frameNew;
		
		vector<DroppedFrame> droppedFrames;
		ofThreadChannel<chrono::high_resolution_clock::time_point> frameReceiverToFrameRateCalculator;
		Utils::FramerateCounter incomingFrameRateCounter;
	};
}
