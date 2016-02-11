#pragma once

#include <stdint.h>

struct Packet {
	enum {
		PacketSize = 4096,
		HeaderSize = 9,
		MaxPayloadSize = PacketSize - HeaderSize
	};

	//header
	uint32_t packetIndex = 0;
	uint32_t frameIndex;
	bool isLastPacket = false;
	size_t payloadSize = 0;

	//payload
	uint8_t payload[MaxPayloadSize];
};
