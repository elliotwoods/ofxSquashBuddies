#pragma once

#include <stdint.h>
#include "ofxAsio/UDP/DataGram.h"

struct Packet {
	struct Header {
		uint32_t packetIndex;
		uint32_t frameIndex;
		uint32_t payloadSize;
		uint32_t isLastPacket;
	};

	//constants
	enum {
		DefaultPacketSize = 4 * 1024,
		PacketAllocationSize = 9000,
		HeaderSize = sizeof(Header),
		MaxPayloadSize = PacketAllocationSize - HeaderSize
	};

	Packet() {
		this->header.packetIndex = 0;
		this->header.frameIndex = 0;
		this->header.payloadSize = 0;
		this->header.isLastPacket = false;
	}

	Packet(const ofxAsio::UDP::DataGram::Message & message) {
		if (message.size() >= HeaderSize) {
			memcpy(this->headerBuffer, message.data(), HeaderSize);
		}
		if (message.size() >= HeaderSize + this->header.payloadSize) {
			memcpy(this->payload, message.data() + HeaderSize, this->header.payloadSize);
		}
	}

	Packet(const string & message) {
		if (message.size() >= HeaderSize) {
			memcpy(this->headerBuffer, message.data(), HeaderSize);
		}
		if (message.size() >= HeaderSize + this->header.payloadSize) {
			memcpy(this->payload, message.data() + HeaderSize, this->header.payloadSize);
		}
	}

	size_t size() const {
		return sizeof(Header) + header.payloadSize;
	}

	//header
	union {
		Header header;
		uint8_t headerBuffer[16];
	};

	//payload
	uint8_t payload[MaxPayloadSize];
};
