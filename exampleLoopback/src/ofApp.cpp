#include "ofApp.h"

#include "ofxSquash.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetFrameRate(60.0f);

	auto codecs = ofxSquash::getCodecList();

	this->video.initGrabber(1280, 720);

	ofSetWindowShape(this->video.getWidth() * 2, this->video.getHeight());

	this->sender.init("127.0.0.1", 4444);

	this->receiver.init(4444);

}

//--------------------------------------------------------------
void ofApp::update(){
	this->video.update();
	if (this->video.isFrameNew()) {
		this->sender.send(this->video.getPixels());
	}

	// In this example, 'receive' will be called directly after 'send' in the ofApp.
	// Which means that there's no time for the frame to arrive in-between, so this example will likely always have 1 frame of delay.
	// If we use 
	this->receiver.update();
	if (this->receiver.isFrameNew()) {
		this->receiver.receive(this->receivedPreview.getPixels());
		this->receivedPreview.update();
	}
}

//--------------------------------------------------------------
void ofApp::draw(){
	this->video.draw(0, 0);
	this->receivedPreview.draw(1280, 0);
	ofDrawBitmapStringHighlight(ofToString(ofGetFrameRate()), 20, 20);
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
