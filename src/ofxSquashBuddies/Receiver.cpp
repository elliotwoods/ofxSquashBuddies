#include "Receiver.h"

namespace ofxSquashBuddies {

	Receiver::~Receiver() {
		this->close();
	}

	void Receiver::init(int port) {

		for (int i = 0; i < 2; i++)
			contextPtr[i] = &context[i];

		this->close();

		this->clear(getCurrentContext());
		this->clear(getNextContext());

		this->stream = make_shared<ofxSquash::Stream>(this->getCodec(), ofxSquash::Direction::Decompress);
		this->stream->setWriteFunction([this](const ofxSquash::WriteFunctionArguments & args) {
			this->decompressStreamArrives(args);
		});

		this->decompressorToApp = make_shared<ofThreadChannel<string>>();

		this->socket = make_shared<ofxAsio::UDP::Server>(port);

		this->threadsRunning = true;
		this->socketThread = thread([this]() {
			this->socketLoop();
		});
	}

	void Receiver::close() {
		this->threadsRunning = false;
		if (this->decompressorToApp) {
			this->decompressorToApp->close();
			this->decompressorToApp.reset();
		}

		if (this->socketThread.joinable()) {
			this->socketThread.join();
		}
	}

	void Receiver::update()
	{

	}

	bool Receiver::isFrameNew() const
	{
		return false;
	}

	bool Receiver::isFrameIncoming() const
	{
		return false;
	}

	void Receiver::socketLoop() {
		cout << "thread start" << endl;

		while (this->threadsRunning) {
			auto dataGram = socket->receive();
			if (!dataGram)
				continue;

			const auto& message = dataGram->getMessage();
			if (message.empty())
				continue;

			const Packet * packet = (const Packet*)message.data();
			packetArrives(*packet);
		}

		cout << "thread end" << endl;
	}

	void Receiver::clear(FrameContext& ctx)
	{
		ctx.frameBuffer.clear();
		ctx.packetBuffer.clear();
		ctx.frameIndex = 0;
		ctx.packetIndex = -1;
	}

	void Receiver::packetArrives(const Packet& packet)
	{
		auto& ctx = getCurrentContext();

		if (packet.frameIndex != ctx.frameIndex)
		{
			clear(getCurrentContext());
			ctx.frameIndex = packet.frameIndex;
		}

		// drop old packet
		if (packet.frameIndex < ctx.frameIndex)
			return;

		// when next frame arrival push to next frame context
		if (packet.frameIndex == ctx.frameIndex + 1)
		{
			auto& next = getNextContext();
			next.frameIndex = packet.frameIndex;

			next.packetBuffer.emplace(packet.packetIndex, std::make_unique<Packet>(packet));
			return;
		}

		if (packet.packetIndex == ctx.packetIndex + 1)
		{
			decompressPacket(packet);
		}
		else
		{
			// add to buffer
			ctx.packetBuffer.emplace(packet.packetIndex, std::make_unique<Packet>(packet));
		}

		// maybe missing some packet. clear and wait next frame
		if (ctx.packetBuffer.size() > 100)
		{
			clear(getCurrentContext());
			return;
		}

		{
			// process unordered packets (NOT WELL TESTED)
			while (ctx.packetBuffer.size())
			{
				auto it = ctx.packetBuffer.begin();
				if (it->first == ctx.packetIndex + 1)
				{
					const Packet& p = *it->second.get();
					decompressPacket(p);

					ctx.packetBuffer.erase(it);
				}
				else break;
			}
		}
	}

	void Receiver::decompressPacket(const Packet& pkt)
	{
		auto& ctx = getCurrentContext();

		stream->read((const void*)pkt.payload, pkt.payloadSize);
		ctx.packetIndex++;

		if (pkt.isLastPacket)
		{
			*stream << ofxSquash::Stream::Finish();
			clear(getCurrentContext());
		}
	}

	void Receiver::decompressStreamArrives(const ofxSquash::WriteFunctionArguments & args)
	{
		auto& ctx = getCurrentContext();

		const uint8_t * ptr = (const uint8_t*)args.data;
		ctx.frameBuffer.insert(ctx.frameBuffer.end(), ptr, ptr + args.size);

		if (args.isFinished)
		{
			frameArrives(ctx.frameBuffer);
			clear(getCurrentContext());
			this->swap();
		}
	}

	void Receiver::frameArrives(const string& frameBuffer)
	{
		decompressorToApp->send(frameBuffer);
		frameNew = true;
	}

	void Receiver::swap()
	{
		std::swap(contextPtr[0], contextPtr[1]);
	}

}