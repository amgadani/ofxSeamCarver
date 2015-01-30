#pragma once

#include "ofMain.h"
#define STRINGIFY(A) #A
class ofxSeamCarver  {
    public :
        void setup(int width, int height, int num_channels);
        void computeGradient(ofBaseHasTexture& tex);
        void renderFrame(int _width, int _height);
        void computeSeamFitness();
        unsigned char * seamCarve(ofImage img, int newWidth, int newHeight);
        unsigned char * removeVerticalSeam(unsigned char * pixels, int w, int h);
        unsigned char * removeHorizontalSeam(unsigned char * pixels, int w, int h);

    string gradientShaderString;
    ofShader gradientShader;
    ofFbo gradientFbo;
    float * seamFitness;
    int width, height;
    ofPixels pix;
    ofTexture tex;
    int components;
};