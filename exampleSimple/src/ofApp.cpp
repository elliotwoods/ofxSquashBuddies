#include "ofApp.h"

#include "ofxSquash.h"

//--------------------------------------------------------------
void ofApp::setup(){
	auto codecs = ofxSquash::getCodecList();

	this->video.initGrabber(640, 480);

	this->sender.init("127.0.0.1", 4444);
	//this->sender.setCodec("density");

	this->receiver.init(4444);
	//this->reciever.setCodec("density");

	// things you might want to send
	//	* A raw buffer
	//	* ofPixels
	//	* ofSoundBuffer
	//	* ofMesh
	//	* a struct / simple type
	//	* a vector or struct / simple type

	// things we don't need to send
	//	* things which should be serialised first (e.g. your own complex classes)

}

//--------------------------------------------------------------
void ofApp::update(){
	this->video.update();
	if (this->video.isFrameNew()) {
		this->sender.send(this->video.getPixels());
	}

	this->receiver.update();
	if (this->receiver.isFrameNew()) {
		this->receiver.receive(this->receivedPreview.getPixels());
		this->receivedPreview.update();
	}
}

//--------------------------------------------------------------
void ofApp::draw(){
	this->video.draw(0, 0);

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
