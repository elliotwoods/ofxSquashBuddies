#include "Message.h"
#include "Constants.h"

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
	void Message::clear() {
		this->headerAndData.clear();
	}

	//----------
	bool Message::getData(string & data) const {
		if (this->hasHeader<Header::String>()) {
			auto header = this->getHeader<Header::String>();
			data.assign((char *) this->getBodyData(), this->getBodySize());
			return true;
		}
		else {
			OFXSQUASHBUDDIES_WARNING << "Message Header doesn't match String/Basic type";
			return false;
		}
	}

	//----------
	bool Message::getData(void * data, size_t & size) const {
		if (this->hasHeader<Header::String>()) {
			auto header = this->getHeader<Header::String>();
			auto bodySize = this->getBodySize();
			if (bodySize > size) {
				OFXSQUASHBUDDIES_ERROR << "Insufficient size in your buffer. Cannot get data";
				return false;
			}
			else {
				memcpy(data, this->getBodyData(), bodySize);
				size = bodySize;
				return true;
			}
		}
		else {
			OFXSQUASHBUDDIES_WARNING << "Message Header doesn't match String/Basic type";
			return false;
		}
	}

	//----------
	bool Message::getData(ofPixels & data) const {
		auto & header = this->getHeader<Header::Pixels>();
		if (this->hasHeader<Header::Pixels>()) {
			const auto & header = this->getHeader<Header::Pixels>();
			auto bodySize = this->getBodySize();
			ofPixelFormat pixelFormat = (ofPixelFormat)header.pixelFormat;

			//reallocate if we need to
			if (data.getWidth() != header.width || data.getHeight() != header.height || data.getPixelFormat() != pixelFormat) {
				data.allocate(header.width, header.height, pixelFormat);
			}
			if (data.size() != bodySize) {
				OFXSQUASHBUDDIES_ERROR << "Message body is of wrong size to fill pixels. Maybe a bug in sender?";
				return false;
			}
			else {
				memcpy(data.getData(), this->getBodyData(), bodySize);
				return true;
			}
		}
		else {
			OFXSQUASHBUDDIES_WARNING << "Message Header doesn't match Pixels type";
			return false;
		}
	}

	//----------
	void Message::pushData(const void * data, size_t dataSize) {
		this->headerAndData.append((const char *) data, dataSize);
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
		else {
			return 0;
		}
	}

	//----------
	void * Message::getBodyData() {
		return &this->headerAndData[0] + this->getHeaderSize();
	}

	//----------
	const void * Message::getBodyData() const {
		return &this->headerAndData[0] + this->getHeaderSize();
	}

	//----------
	size_t Message::getBodySize() const {
		return this->headerAndData.size() - this->getHeaderSize();
	}

	//----------
	bool Message::empty() const {
		if (this->headerAndData.empty()) {
			return true;
		}
		else if(this->headerAndData.size() < this->getHeaderSize()) {
			return true;
		}
		else {
			return false;
		}
	}

	//----------
	const string & Message::getMessageString() const {
		return this->headerAndData;
	}
}