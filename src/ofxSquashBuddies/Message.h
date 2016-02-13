#pragma once

#include "ofConstants.h"
#include "ofPixels.h"
#include "ofMesh.h"

#include <string>
using namespace std;

namespace ofxSquashBuddies {
	enum MessageType : uint16_t {
		//--
		//0 to 15 are reserved for oF and native types
		//--
		//
		String = 0,
		Basic = 0, // String and Basic are the same thing (i.e. unformatted data)
		Pixels = 1,
		Mesh = 2,
		//
		//--



		//--
		//16 to 31 are reserved for MultiTrack
		//--
		//
		MultiTrack_2_2_Frame = 16,
		//
		//--



		//--
		//Use 32+ for your own types
		//--
		//
		First_User_Slot = 32,
		Example_User_Slot = First_User_Slot + 1
		//
		//--
	};

	namespace Header {
		struct Basic {
			struct {
				uint16_t headerSize = 4;
				MessageType messageType = MessageType::Basic;
			};
		};

		typedef Basic String;

		struct Pixels {
			struct {
				uint16_t headerSize = 10;
				MessageType messageType = MessageType::Pixels;
				uint16_t width;
				uint16_t height;
				uint16_t pixelFormat;
			};
		};

		struct Mesh {
			struct {
				uint16_t headerSize = 4;
				MessageType messageType = MessageType::Mesh;
			};
		};
	}

	class Message {
	public:
		Message();
		Message(const string &);
		Message(const void * data, size_t size);
		Message(const ofPixels &);
		
		void setData(const string &);
		void setData(const void * data, size_t size);
		void setData(const ofPixels &);

		void clear();

		bool getData(string &) const;

		// `data` should be a buffer which you have allocated with size `size`
		// this function modifies the `size` argument to be the actual size
		// but DOES NOT change the actual size of the buffer
		bool getData(void * data, size_t & size) const;
		bool getData(ofPixels &) const;

		void pushData(const void * data, size_t size);

		// NOTE : Some functions can be dangerous to call if no valid header is set on the Message
		void * getHeaderData();
		const void * getHeaderData() const;
		size_t getHeaderSize() const;

		void * getBodyData();
		const void * getBodyData() const;
		size_t getBodySize() const;
		bool empty() const;

		template<typename HeaderType>
		bool hasHeader() const {
			if (this->getHeaderSize() < sizeof(HeaderType)) {
				return false;
			}
			else {
				auto testHeader = HeaderType();
				return memcmp(this->getHeaderData(), &testHeader, sizeof(testHeader.headerSize) + sizeof(testHeader.messageType)) == 0;
			}
		}

		template<typename HeaderType>
		const HeaderType & getHeader() const {
			return *(const HeaderType *) this->getHeaderData();
		}

		template<typename HeaderType>
		HeaderType & getHeader(bool initializeHeader) {
			auto & header = *(HeaderType *) this->getHeaderData();
			if (initializeHeader) {
				header = HeaderType();
			}
			return header;
		}

		const string & getMessageString() const;
	protected:
		string headerAndData;
	};
}