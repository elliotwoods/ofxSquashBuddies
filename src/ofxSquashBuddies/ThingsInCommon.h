#pragma once

#include "Constants.h"
#include "Packet.h"
#include "Message.h"

#include "ofxSquash/Codec.h"
#include "ofxSquash/Stream.h"

#include "ofxAsio.h"

namespace ofxSquashBuddies {
	class ThingsInCommon {
	public:
		virtual void setCodec(const ofxSquash::Codec &) = 0;
		void setCodec(const string & codecName);
		virtual const ofxSquash::Codec & getCodec() const = 0;

		static ofxSquash::Codec getDefaultCodec();
	};
}