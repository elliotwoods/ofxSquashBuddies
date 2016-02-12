#include "Message.h"

namespace ofxSquashBuddies {
	//----------
	Message::Message() {

	}

	//----------
	Message::Message(const string & data) {
		this->setData(data);
	}

	//----------
	Message::Message(const void * data, size_t size) {
		this->setData(data, size);
	}

	//----------
	Message::Message(const ofPixels & data) {
		this->setData(data);
	}

	//----------
	void Message::setData(const string & data) {
		this->setData(data.data(), data.size());
	}

	//----------
	void Message::setData(const void * data, size_t size) {
		this->headerAndData.resize(size + sizeof(Header::String));

		auto & header = this->getHeader<Header::String>(true);
		auto body = this->getBodyData();
		memcpy(body, data, size);
	}

	//----------
	void Message::setData(const ofPixels & data) {
		const auto headerSize = sizeof(Header::Pixels);
		const auto bodySize = data.size(); // inner payload

		this->headerAndData.resize(headerSize + bodySize);

		auto & header = this->getHeader<Header::Pixels>(true);
		header.width = data.getWidth();
		header.height = data.getHeight();
		header.pixelFormat = data.getPixelFormat();

		auto body = this->getBodyData();
		memcpy(body, data.getData(), bodySize);
	}

	//----------
	void * Message::getHeaderData() {
		return (void *) this->headerAndData.data();
	}

	//----------
	const void * Message::getHeaderData() const {
		return (void *) this->headerAndData.data();
	}

	//----------
	size_t Message::getHeaderSize() const {
		if (this->headerAndData.size() > sizeof(Header::Basic)) {
			const auto & header = this->getHeader<Header::Basic>();
			return header.headerSize;
		}
	}

	//----------
	void * Message::getBodyData() {
		return &this->headerAndData[0] + this->getHeaderSize();
	}

	//----------
	const string & Message::getMessageString() const {
		return this->headerAndData;
	}
}