#pragma once

#include "ofLog.h"

#ifndef __func__
#define __func__ __FUNCTION__
#endif
#define OFXSQUASHBUDDIES_NOTICE ofLogNotice(string(__func__))
#define OFXSQUASHBUDDIES_WARNING ofLogWarning(string(__func__))
#define OFXSQUASHBUDDIES_ERROR ofLogError(string(__func__))
#define OFXSQUASHBUDDIES_FATAL ofLogFatalError(string(__func__))

namespace ofxSquashBuddies {
	enum MessageType {
		Raw = 0,
		Pixels = 1,
		Mesh = 2
	};

	namespace Header {
		struct Raw {
			MessageType messageType = MessageType::Raw;
		};

		struct Pixels {
			MessageType messageType = MessageType::Pixels;
			uint16_t width;
			uint16_t height;
			ofPixelFormat format;
		};

		struct Mesh {
			MessageType messageType = MessageType::Mesh;
		};
	}
}