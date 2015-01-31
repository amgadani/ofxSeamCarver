#include "testApp.h"

void testApp::setup() {
    img.loadImage("Jump.jpg");
    seamCarver.setup(img.width, img.height, img.bpp / 8);
    ofSetFrameRate(30);
}

void testApp::update() {
    img.setFromPixels(seamCarver.seamCarve(img, true,false), img.getWidth()-1, img.getHeight(), OF_IMAGE_COLOR);

}

void testApp::draw() {
    img.draw(0,0);
    ofDrawBitmapString(ofToString(ofGetFrameRate())+"fps", 10, 15);
}


