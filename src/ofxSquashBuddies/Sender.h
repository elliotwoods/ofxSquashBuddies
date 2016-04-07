#pragma once

#include "ThingsInCommon.h"

#include "ThreadChannel.h"
#include "Utils.h"

#include "ofThreadChannel.h"
#include "ofPixels.h"
#include "ofMesh.h"

namespace ofxSquashBuddies {
	class Sender : public ThingsInCommon {
	public:
		Sender();
		~Sender();
		void init(string ipAddress, int port);
		void close();
		
		void setCodec(const ofxSquash::Codec &) override;
		const ofxSquash::Codec & getCodec() const override;

		bool send(const void *, size_t);
		bool send(const string &);
		bool send(const ofPixels &);
		bool send(const ofMesh &);
		bool send(const Message &);
		bool send(Message &&);

		template<typename PodType, typename std::enable_if<std::is_pod<PodType>::value, void>::type>
		void send(PodType & data) {
			this->send(&data, sizeof(PodType));
		}

		float getSendFramerate() const;

		/// The sender will not send any frames to the compressor whilst the socket buffer's size is greater than maxSocketBufferSize
		void setMaxSocketBufferSize(size_t maxSocketBufferSize);
		size_t getMaxSocketBufferSize() const;
		size_t getCurrentSocketBufferSize() const;

		size_t getPacketSize() const;
		void setPacketSize(size_t);

		ofxAsio::UDP::EndPoint getEndPoint();
	protected:
		void compressLoop();
		void socketLoop();

		ofxSquash::Codec codec;

		bool threadsRunning = false;
		std::thread compressThread;
		std::thread socketThread;

		shared_ptr<ofxAsio::UDP::Client> socket;

		shared_ptr<ofThreadChannel<Message>> appToCompressor;
		shared_ptr<ThreadChannel<Packet>> compressorToSocket;

		struct Config {
			ofxAsio::UDP::EndPoint endPoint;
		} config;
		mutex configMutex;

		size_t maxSocketBufferSize = 300;
		size_t packetSize = Packet::DefaultPacketSize;

		Utils::FramerateCounter sendFramerateCounter;
	};
}