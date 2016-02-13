#include "ofApp.h"

#include "ofxSquash.h"

//--------------------------------------------------------------
void ofApp::setup(){
	auto port = 4444;
	string ipAddress = "127.0.0.1";

	auto result = ofSystemTextBoxDialog("Target IP Address (Default : " + ipAddress + ")");
	if (!result.empty()) {
		ipAddress = result;
	}

	this->sender.init("127.0.0.1", 4444);


	ofSetWindowTitle("Sending to : " + ipAddress + ":" + ofToString(port));
	ofSetFrameRate(60);
	this->video.initGrabber(1280, 720);
}

//--------------------------------------------------------------
void ofApp::update(){
	this->video.update();
	if (this->video.isFrameNew()) {
		this->sender.send(this->video.getPixels());
	}
}

//--------------------------------------------------------------
void ofApp::draw(){
	this->video.draw(0, 0);
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
