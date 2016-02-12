#pragma once

#include <stdint.h>

struct Packet {
	Packet() {
		this->packetIndex = 0;
		this->frameIndex = 0;
		this->payloadSize = 0;
		this->isLastPacket = false;
	}

	enum {
		PacketSize = 4096,
		HeaderSize = 4 + 4 + 4 + 4,
		MaxPayloadSize = PacketSize - HeaderSize
	};

	//header
	union {
		struct {
			uint32_t packetIndex;
			uint32_t frameIndex;
			uint32_t payloadSize;
			uint32_t isLastPacket;
		};
		uint8_t headerBuffer[16];
	};

	//payload
	uint8_t payload[MaxPayloadSize];
};
