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
		ShortPixels = 2,
		FloatPixels = 3,

		Mesh = 8,
		//
		//--



		//--
		//16 to 31 are reserved for MultiTrack
		//--
		//
		MultiTrack_2_2_Frame = 16,
		MultiTrack_2_3_Frame = 17,
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

		template<MessageType PixelsMessageType>
		struct Pixels_ {
			struct {
				uint16_t headerSize = 10;
				MessageType messageType = PixelsMessageType;

				uint16_t width;
				uint16_t height;
				uint16_t pixelFormat;
			};
		};

		typedef Pixels_<MessageType::Pixels> Pixels;
		typedef Pixels_<MessageType::ShortPixels> ShortPixels;
		typedef Pixels_<MessageType::FloatPixels> FloatPixels;

		struct Mesh {
			struct {
				uint16_t headerSize = 32;
				MessageType messageType = MessageType::Mesh;

				uint32_t verticesSize;
				uint32_t colorsSize;
				uint32_t normalsSize;
				uint32_t texCoordsSize;
				uint32_t indicesSize;

				uint16_t primitiveMode;

				uint8_t useColors;
				uint8_t useNormals;
				uint8_t useTextures;
				uint8_t useIndices;

				uint8_t reserved[2];
			};
		};

		struct MultiTrack_2_2_Frame {
			enum Constants : size_t {
				ColorSize = 1280 * 720 * 2,
				DepthSize = 512 * 424 * 2,
				InfraredSize = 512 * 424 * 2,
				BodyIndexSize = 512 * 424 * 1,
				ColorCoordInDepthViewSize = 512 * 424 * 2 * 2,

				SkeletonBodyHeaderSize = sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint64_t) + sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint8_t),
				
				//(size_t)_JointType::JointType_Count * (sizeof(_Joint) + sizeof(_JointOrientation) + sizeof(ofVec2f));
				SkeletonJointSize = 1200,

				SkeletonBodySize = SkeletonBodyHeaderSize + 20 * SkeletonJointSize,
				SkeletonSize = SkeletonBodySize * 6 + 1,

				TotalDataSize = ColorSize + DepthSize + InfraredSize + BodyIndexSize + ColorCoordInDepthViewSize + SkeletonSize
			};

			struct {
				uint16_t headerSize = 4;
				MessageType messageType = MessageType::MultiTrack_2_2_Frame;
			};
		};

		struct MultiTrack_2_3_Frame {
			enum Constants {
				SkeletonSize = MultiTrack_2_2_Frame::Constants::SkeletonSize
			};

			enum DataAvailable : uint16_t {
				Nothing = 0,
				Color = 1 << 0,
				Depth = 1 << 1,
				Infrared = 1 << 2,
				BodyIndex = 1 << 3,
				ColorCoordInDepthView = 1 << 4,
				Bodies = 1 << 5
			};

			typedef DataAvailable DataType;

			enum PixelFormat : uint8_t {
				Unknown  = 0,
				RGB_8 = 1,
				RGBA_8 = 2,
				L_8 = 3,
				L_16 = 4,
				YUY2_8 = 5,
				RG_16 = 6
			};

			struct FrameSettings {
				size_t size() const;
				uint16_t width;
				uint16_t height;
				PixelFormat pixelFormat;
			};

			struct {
				uint16_t headerSize = 6;
				MessageType messageType = MessageType::MultiTrack_2_3_Frame;
				DataAvailable dataAvailable = DataAvailable::Nothing;
			};

			static size_t getBytesPerPixel(PixelFormat pixelFormat) {
				switch (pixelFormat) {
				case RGB_8:
					return 3;
				case RGBA_8:
					return 4;
				case L_8:
					return 1;
				case L_16:
					return 2;
				case YUY2_8:
					return 2;
				case RG_16:
					return 4;
				default:
					return 0;
				}
			}

			static ofPixelFormat toOf(PixelFormat pixelFormat) {
				switch (pixelFormat) {
				case RGB_8:
					return ofPixelFormat::OF_PIXELS_RGB;
				case RGBA_8:
					return ofPixelFormat::OF_PIXELS_RGBA;
				case L_8:
					return ofPixelFormat::OF_PIXELS_GRAY;
				case L_16:
					return ofPixelFormat::OF_PIXELS_GRAY;
				case YUY2_8:
					return ofPixelFormat::OF_PIXELS_YUY2;
				case RG_16:
					return ofPixelFormat::OF_PIXELS_RG;
				case Unknown:
				default:
					return ofPixelFormat::OF_PIXELS_UNKNOWN;
				}
			}
		};
	}

	class Message {
	public:
		Message();
		Message(const string &);
		Message(const void * data, size_t size);
		Message(const ofPixels &);
		Message(const ofShortPixels &);
		Message(const ofFloatPixels &);
		Message(const ofMesh &);
		
		void setData(const string &);
		void setData(const void * data, size_t size);
		void setData(const ofPixels &);
		void setData(const ofShortPixels &);
		void setData(const ofFloatPixels &);
		void setData(const ofMesh &);

		void clear();

		bool getData(string &) const;
		bool getData(void * data, size_t & size) const; // size in = your buffer size, size out = how much of your buffer we're now using
		bool getData(ofPixels &) const;
		bool getData(ofShortPixels &);
		bool getData(ofFloatPixels &);
		bool getData(ofMesh &) const;

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

		void resizeHeaderAndBody(size_t);
		const string & getMessageString() const;
	protected:
		string headerAndBody;
	};
}