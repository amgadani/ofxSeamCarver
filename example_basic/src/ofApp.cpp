#include "ofApp.h"

void ofApp::setup() {
    img.load("/Users/aashish/Desktop/Jump.jpg");
    seamCarver.setup(img.getWidth(), img.getHeight(), img.getImageType());
}

void ofApp::update() {
    img.setFromPixels(seamCarver.seamCarve(img, true,true));
}

void ofApp::draw() {
    img.draw(0,0);
    ofDrawBitmapString(ofToString(ofGetFrameRate())+"fps", 10, 15);
}


