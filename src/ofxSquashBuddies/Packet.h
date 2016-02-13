#pragma once

#include <stdint.h>
#include "ofxAsio/UDP/DataGram.h"

struct Packet {
	//constants
	enum {
		PacketSize = 1024 * 9,
		HeaderSize = 4 + 4 + 4 + 4,
		MaxPayloadSize = PacketSize - HeaderSize
	};

	struct Header {
		uint32_t packetIndex;
		uint32_t frameIndex;
		uint32_t payloadSize;
		uint32_t isLastPacket;
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

	//header
	union {
		Header header;
		uint8_t headerBuffer[16];
	};

	//payload
	uint8_t payload[MaxPayloadSize];
};
