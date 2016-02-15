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
	Message::Message(const ofMesh & data) {
		this->setData(data);
	}

	//----------
	void Message::setData(const string & data) {
		this->setData(data.data(), data.size());
	}

	//----------
	void Message::setData(const void * data, size_t size) {
		this->headerAndBody.resize(size + sizeof(Header::String));

		auto & header = this->getHeader<Header::String>(true);
		auto body = this->getBodyData();
		memcpy(body, data, size);
	}

	//----------
	void Message::setData(const ofPixels & data) {
		const auto headerSize = sizeof(Header::Pixels);
		const auto bodySize = data.size(); // inner payload

		this->headerAndBody.resize(headerSize + bodySize);

		auto & header = this->getHeader<Header::Pixels>(true);
		header.width = data.getWidth();
		header.height = data.getHeight();
		header.pixelFormat = data.getPixelFormat();

		auto body = this->getBodyData();
		memcpy(body, data.getData(), bodySize);
	}

	//----------
	void Message::setData(const ofMesh & data) {
		const auto headerSize = sizeof(Header::Mesh);
		
		const auto verticesDataSize = data.getNumVertices() * sizeof(ofVec3f);
		const auto colorsDataSize = data.getNumColors() * sizeof(ofFloatColor);
		const auto normalsDataSize = data.getNumNormals() * sizeof(ofVec3f);
		const auto texCoordsDataSize = data.getNumTexCoords() * sizeof(ofVec2f);
		const auto indicesDataSize = data.getNumIndices() * sizeof(ofIndexType);

		const size_t bodySize = verticesDataSize + colorsDataSize + normalsDataSize + texCoordsDataSize + indicesDataSize;
		
		this->headerAndBody.resize(headerSize + bodySize);

		// header
		{
			auto & header = this->getHeader<Header::Mesh>(true);
			header.verticesSize = (uint32_t) data.getNumVertices();
			header.colorsSize = (uint32_t) data.getNumColors();
			header.normalsSize = (uint32_t) data.getNumNormals();
			header.texCoordsSize = (uint32_t) data.getNumTexCoords();
			header.indicesSize = (uint32_t) data.getNumIndices();

			header.primitiveMode = data.getMode();

			header.useColors = data.usingColors();
			header.useNormals = data.usingNormals();
			header.useTextures = data.usingTextures();
			header.useIndices = data.usingIndices();
		}

		// body
		{
			auto body = (uint8_t *) this->getBodyData();

			memcpy(body, data.getVerticesPointer(), verticesDataSize);
			body += verticesDataSize;

			memcpy(body, data.getColorsPointer(), colorsDataSize);
			body += colorsDataSize;

			memcpy(body, data.getNormalsPointer(), normalsDataSize);
			body += normalsDataSize;

			memcpy(body, data.getTexCoordsPointer(), texCoordsDataSize);
			body += texCoordsDataSize;

			memcpy(body, data.getIndexPointer(), indicesDataSize);
		}
	}

	//----------
	void Message::clear() {
		this->headerAndBody.clear();
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
	bool Message::getData(ofMesh & data) const {
		if (this->hasHeader<Header::Mesh>()) {
			const auto & header = this->getHeader<Header::Mesh>();
			auto bodySize = this->getBodySize();

			auto & vertices = data.getVertices();
			auto & colors = data.getColors();
			auto & normals = data.getNormals();
			auto & texCoords = data.getTexCoords();
			auto & indices = data.getIndices();

			//resize as needed
			vertices.resize(header.verticesSize);
			colors.resize(header.colorsSize);
			normals.resize(header.normalsSize);
			texCoords.resize(header.texCoordsSize);
			indices.resize(header.indicesSize);

			//data sizes
			const auto verticesDataSize = header.verticesSize * sizeof(ofVec3f);
			const auto colorsDataSize = header.colorsSize * sizeof(ofFloatColor);
			const auto normalsDataSize = header.normalsSize * sizeof(ofVec3f);
			const auto texCoordsDataSize = header.texCoordsSize * sizeof(ofVec2f);
			const auto indicesDataSize = header.indicesSize * sizeof(ofIndexType);

			//copy in data
			{
				auto body = (uint8_t *) this->getBodyData();

				memcpy(data.getVerticesPointer(), body, verticesDataSize);
				body += verticesDataSize;

				memcpy(data.getColorsPointer(), body, colorsDataSize);
				body += colorsDataSize;

				memcpy(data.getNormalsPointer(), body, normalsDataSize);
				body += normalsDataSize;

				memcpy(data.getTexCoordsPointer(), body, texCoordsDataSize);
				body += texCoordsDataSize;

				memcpy(data.getIndexPointer(), body, indicesDataSize);
				body += indicesDataSize;
			}

			//apply header
			{
				data.setMode((ofPrimitiveMode) header.primitiveMode);
				header.useColors ? data.enableColors() : data.disableColors();
				header.useNormals ? data.enableNormals() : data.disableNormals();
				header.useTextures ? data.enableTextures() : data.disableTextures();
			}

			return true;
		}
		else {
			OFXSQUASHBUDDIES_WARNING << "Message Header doesn't match Mesh type";
			return false;
		}

	}

	//----------
	void Message::pushData(const void * data, size_t dataSize) {
		this->headerAndBody.append((const char *) data, dataSize);
	}

	//----------
	void * Message::getHeaderData() {
		return (void *) this->headerAndBody.data();
	}

	//----------
	const void * Message::getHeaderData() const {
		return (void *) this->headerAndBody.data();
	}

	//----------
	size_t Message::getHeaderSize() const {
		if (this->headerAndBody.size() > sizeof(Header::Basic)) {
			const auto & header = this->getHeader<Header::Basic>();
			return header.headerSize;
		}
		else {
			return 0;
		}
	}

	//----------
	void * Message::getBodyData() {
		return &this->headerAndBody[0] + this->getHeaderSize();
	}

	//----------
	const void * Message::getBodyData() const {
		return &this->headerAndBody[0] + this->getHeaderSize();
	}

	//----------
	size_t Message::getBodySize() const {
		return this->headerAndBody.size() - this->getHeaderSize();
	}

	//----------
	bool Message::empty() const {
		if (this->headerAndBody.empty()) {
			return true;
		}
		else if(this->headerAndBody.size() < this->getHeaderSize()) {
			return true;
		}
		else {
			return false;
		}
	}

	//----------
	void Message::resizeHeaderAndBody(size_t size) {
		this->headerAndBody.resize(size);
	}

	//----------
	const string & Message::getMessageString() const {
		return this->headerAndBody;
	}
}