#pragma once

#include "ofMain.h"
#define STRINGIFY(A) #A
class ofxSeamCarver  {
    public :
        void setup(int width, int height, int num_channels);
        void computeGradient(unsigned char * pixels, int width, int height, ofFbo gradFbo);
        void renderFrame(int _width, int _height);
        float * computeVerticalSeamFitness(ofFbo gradFbo, int width, int height);
        float * computeHorizontalSeamFitness(ofFbo gradFbo, int width, int height);
        unsigned char * seamCarve(ofImage img, bool x, bool y);
        unsigned char * removeVerticalSeam(unsigned char * pixels, float * seamFitness, int w, int h);

        unsigned char * removeHorizontalSeam(unsigned char * pixels, float * seamFitness, int w, int h);

    string gradientShaderString;
    ofShader gradientShader;
    ofFbo gradientFbo;
    
    //float * seamFitness;
   // ofPixels pix;
   // ofTexture tex;
    int components;
    ofPixels pix;

};