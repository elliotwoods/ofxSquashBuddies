#include "ThingsInCommon.h"

namespace ofxSquashBuddies {
	//----------
	void ThingsInCommon::setCodec(const string & codecName) {
		this->setCodec(ofxSquash::Codec(codecName));
	}

	//----------
	ofxSquash::Codec ThingsInCommon::getDefaultCodec() {
		auto codec = ofxSquash::Codec("lz4");
		return codec;
	}
}