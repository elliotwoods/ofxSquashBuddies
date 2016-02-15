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

	this->sender.init(ipAddress, port);


	ofSetWindowTitle("Sending to : " + ipAddress + ":" + ofToString(port));
	ofSetFrameRate(60);

	mesh = ofSpherePrimitive(10.0, 50).getMesh();
}

//--------------------------------------------------------------
void ofApp::update(){
	//distort the vertices
	auto & vertices = mesh.getVertices();
	auto & colors = mesh.getColors();
	colors.resize(vertices.size());
	auto * colorPointer = &colors[0];
	for (auto & vertex : vertices) {
		auto theta = atan2f(vertex.z, vertex.x);
		auto thi = atan2f(vertex.y, vertex.x);
		
		auto length = ofNoise(theta, thi, ofGetElapsedTimef()) * 100.0f + 150.0f;

		vertex = vertex * length / vertex.length();

		auto & color = *colorPointer++;
		color.set(200, 100, 100);
		color.setHue(ofNoise(vertex / 1000.0f));
	}

	sender.send(mesh);
}

//--------------------------------------------------------------
void ofApp::draw(){
	camera.begin();
	ofDrawAxis(50.0f);
	mesh.drawWireframe();
	camera.end();

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
