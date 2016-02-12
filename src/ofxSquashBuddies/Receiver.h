#pragma once

#include "ThingsInCommon.h"

#include "ofThreadChannel.h"
#include "ofEvent.h"

namespace ofxSquashBuddies {
	class Receiver : public ThingsInCommon {
	public:
		~Receiver();

		void init(int port);
		void close();

		void update();
		bool isFrameNew() const;
		bool isFrameIncoming() const;

		template<typename Type>
		bool receive(Type &);

		template<typename Type>
		bool receiveNext(Type &, const chrono::milliseconds & timeOut);

	protected:
		atomic_bool threadsRunning;
		thread socketThread;

		shared_ptr<ofxAsio::UDP::Server> socket;
		shared_ptr<ofThreadChannel<string>> decompressorToApp;

		atomic_bool frameNew;

		void socketLoop();

	protected:
		shared_ptr<ofxSquash::Stream> stream;

		struct FrameContext {
			uint32_t frameIndex;
			int32_t packetIndex;

			map<uint16_t, unique_ptr<Packet>> packetBuffer;
			string frameBuffer;
		};

		FrameContext context[2];
		FrameContext * contextPtr[2];

		FrameContext& getCurrentContext() { return *contextPtr[0]; }
		FrameContext& getNextContext() { return *contextPtr[1]; }

		void clear(FrameContext& ctx);
		void packetArrives(const Packet& pkt);
		void decompressPacket(const Packet& pkt);
		void decompressStreamArrives(const ofxSquash::WriteFunctionArguments & args);
		void frameArrives(const string& frameBuffer);
		void swap();
	};

	//==========

	template<typename Type>
	bool ofxSquashBuddies::Receiver::receive(Type &)
	{

	}

	template<typename Type>
	bool ofxSquashBuddies::Receiver::receiveNext(Type &, const chrono::milliseconds & timeOut)
	{

	}
}