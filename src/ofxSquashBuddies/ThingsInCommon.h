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
		void setCodec(const ofxSquash::Codec &);
		void setCodec(const string & codecName);
		const ofxSquash::Codec & getCodec() const;

	protected:
		ofxSquash::Codec codec = ofxSquash::Codec("density");
	};
}