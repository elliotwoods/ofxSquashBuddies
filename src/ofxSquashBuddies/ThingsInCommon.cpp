#include "ThingsInCommon.h"

namespace ofxSquashBuddies {
	//----------
	void ThingsInCommon::setCodec(const ofxSquash::Codec & codec) {
		this->codec = codec;
	}

	//----------
	void ThingsInCommon::setCodec(const string & codecName) {
		this->setCodec(ofxSquash::Codec(codecName));
	}

	//----------
	const ofxSquash::Codec & ThingsInCommon::getCodec() cosnt {
		return this->codec;
	}
}