#pragma once

#include "ThingsInCommon.h"

#include "ofThreadChannel.h"
#include "ofPixels.h"
#include "ofMesh.h"

#include <condition_variable>

namespace ofxSquashBuddies {
	class Sender : public ThingsInCommon {
	public:
		Sender();
		~Sender();
		void init(string ipAddress, int port);
		void close();

		void setCodec(const ofxSquash::Codec &) override;
		const ofxSquash::Codec & getCodec() const override;

		void send(const void *, size_t);
		void send(const string &);
		void send(const ofPixels &);
		void send(const Message &);

		template<typename PodType, typename std::enable_if<std::is_pod<PodType>::value, void>::type>
		void send(PodType & data) {
			this->send(&data, sizeof(PodType));
		}

	protected:
		void compressLoop();
		void socketLoop();

		ofxSquash::Codec codec;

		bool threadsRunning = false;
		thread compressThread;
		thread socketThread;

		shared_ptr<ofxAsio::UDP::Client> socket;

		shared_ptr<ofThreadChannel<Message>> appToCompressor;
		shared_ptr<ofThreadChannel<Packet>> compressorToSocket;

		struct {
			ofxAsio::UDP::EndPoint endPoint;
		} config;
		mutex configMutex;
	};
}