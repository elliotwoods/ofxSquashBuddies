#pragma once

#include "ThingsInCommon.h"

#include "ofThreadChannel.h"
#include "ofPixels.h"
#include "ofMesh.h"

#include <condition_variable>

namespace ofxSquashBuddies {
	class Sender : public ThingsInCommon {
	public:
		~Sender();
		void init(string ipAddress, int port);
		void close();

		void send(const void * payload, size_t payloadSize);
		void send(const string &);
		void send(const ofPixels &);

		template<typename PodType, typename std::enable_if<std::is_pod<PodType>::value, void>::type>
		void send(PodType & data) {
			this->send(&data, sizeof(PodType));
		}

	protected:
		void compressLoop();
		void socketLoop();

		bool threadsRunning = false;
		thread compressThread;
		thread socketThread;

		shared_ptr<ofxAsio::UDP::Client> socket;

		shared_ptr<ofThreadChannel<string>> appToCompressor;
		shared_ptr<ofThreadChannel<Packet>> compressorToSocket;

		struct {
			ofxAsio::UDP::EndPoint endPoint;
		} config;
		mutex configMutex;
	};
}