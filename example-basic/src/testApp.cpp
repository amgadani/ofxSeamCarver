#include "testApp.h"

void testApp::setup() {
    img.loadImage("Jump.jpg");
    seamCarver.setup(img.width, img.height, img.bpp / 8);
}

void testApp::update() {
    
}

void testApp::draw() {
    img.draw(0,0);
    ofDrawBitmapString(ofToString(ofGetFrameRate())+"fps", 10, 15);
}
void testApp::mousePressed(int x, int y, int button){
    if(x < img.width && y < img.height) {
        img.setFromPixels(seamCarver.seamCarve(img, x, y),
                          x, y, OF_IMAGE_COLOR);
    }
}

