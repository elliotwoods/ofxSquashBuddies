#include "ThingsInCommon.h"

namespace ofxSquashBuddies {
	//----------
	void ThingsInCommon::setCodec(const string & codecName) {
		this->setCodec(ofxSquash::Codec(codecName));
	}

	//----------
	ofxSquash::Codec ThingsInCommonTCP::getDefaultCodec() {
		auto codec = ofxSquash::Codec("lz4");
		return codec;
	}

	//----------
	ofxSquash::Codec ThingsInCommonUDP::getDefaultCodec() {
		auto codec = ofxSquash::Codec("density");
		return codec;
	}
}