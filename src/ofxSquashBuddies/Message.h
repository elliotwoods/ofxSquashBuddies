#pragma once

#include <string>
#include "ofConstants.h"

#include "ofPixels.h"
#include "ofMesh.h"

using namespace std;

namespace ofxSquashBuddies {
	enum MessageType : uint16_t {
		//--
		//0 to 15 are reserved for oF and native types
		//--
		//
		String = 0, //strings are often used to represent raw data
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
				MessageType messageType = MessageType::String;
			};
		};

		typedef Basic String;

		struct Pixels {
			struct {
				uint16_t headerSize = 4;
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

		// NOTE : Some functions can be dangerous to call if no valid header is set on the Message
		void * getHeaderData();
		const void * getHeaderData() const;
		size_t getHeaderSize() const;
		void * getBodyData();

		template<typename HeaderType>
		HeaderType & getHeader(bool initializeHeader) {
			auto & header = *(HeaderType *) this->getHeaderData();
			if (initializeHeader) {
				header = HeaderType();
			}
			return header;
		}

		template<typename HeaderType>
		const HeaderType & getHeader() const {
			return *(const HeaderType *) this->getHeaderData();
		}

		const string & getMessageString() const;
	protected:
		string headerAndData;
	};
}