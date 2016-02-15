#include "ofApp.h"

#include "ofxSquash.h"

//--------------------------------------------------------------
void ofApp::setup(){
	auto port = 4444;
	this->receiver.init(port);

	ofSetWindowTitle("Receiving : " + ofToString(port));
	ofSetFrameRate(60);
}

//--------------------------------------------------------------
void ofApp::update(){
	this->receiver.update();
	if (this->receiver.isFrameNew()) {
		this->receiver.receive(this->image.getPixels());
		this->image.update();
	}
}

//--------------------------------------------------------------
void ofApp::draw(){
	if (this->image.isAllocated())
	{
		this->image.draw(0, 0);
	}
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
