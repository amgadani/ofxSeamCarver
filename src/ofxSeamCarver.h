#pragma once

#include "ofMain.h"
#define STRINGIFY(A) #A
class ofxSeamCarver  {
    public :
        void setup(int width, int height, ofImageType imgType);
        void computeGradient(ofPixels pixels, int width, int height, ofFbo gradFbo);
        void renderFrame(int _width, int _height);
        float * computeVerticalSeamFitness(ofFbo gradFbo, int width, int height);
        float * computeHorizontalSeamFitness(ofFbo gradFbo, int width, int height);
        ofPixels seamCarve(ofImage img, bool x, bool y);
        ofPixels removeVerticalSeam(ofPixels pixels, float * seamFitness, int w, int h);

        ofPixels removeHorizontalSeam(ofPixels pixels, float * seamFitness, int w, int h);

    string gradientShaderString;
    ofShader gradientShader;
    ofFbo gradientFbo;
    
    //float * seamFitness;
   // ofPixels pix;
   // ofTexture tex;
    ofPixels pix, vPix;
    ofImageType imageType;

};