#include "ofApp.h"

void ofApp::setup() {
    img.load("test.jpg");
    seamCarver.setup(img.getWidth(), img.getHeight(), img.getImageType());

}

void ofApp::update() {
    //img.setFromPixels(seamCarver.seamAdd(img, 10, 1));
    img.setFromPixels(seamCarver.seamCarve(img, true, true));
    //drawn.setFromPixels(seamCarver.drawSeam(img, false, true));
    cout << "IMG PIXELS NOW: " << img.getWidth() << "\t" << img.getHeight() << endl;
    fitnessData.allocate(img.getWidth(), img.getHeight(), GL_LUMINANCE);
    fitnessData.loadData(seamCarver.getNormalizedVerticalSeamFitness(img.getPixels()), img.getWidth(),
                         img.getHeight(), GL_LUMINANCE);
}

void ofApp::draw() {
    ofBackground(0,0,0);
//    seamCarver.gradientFbo.draw(0,0,400,300);
//    fitnessData.draw(400, 0, 400, 300);
//    img.draw(0,300, 400, 300);
    fitnessData.draw(0, 0, 640, 480);
    ofDrawEllipse(fitnessData.getWidth(), fitnessData.getHeight(), 10, 10);
    img.draw(640,0, 640, 480);

    ofDrawBitmapString(ofToString(ofGetFrameRate())+"fps", 10, 15);
}


