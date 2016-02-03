#pragma once

#include "ofMain.h"
#define STRINGIFY(A) #A

class SeamFitness {
public:
    float fitness;
    int index;
    
    bool operator < (const SeamFitness& str) const
    {
        return (fitness < str.fitness);
    }
};

class ofxSeamCarver  {
    public :
        void setup(int width, int height, ofImageType imgType);
        void computeGradient(ofPixels pixels, int width, int height, ofFbo gradFbo);
        void renderFrame(int _width, int _height);
        float * computeVerticalSeamFitness(ofFbo gradFbo, int width, int height);
        float * computeHorizontalSeamFitness(ofFbo gradFbo, int width, int height);
        ofPixels seamCarve(ofImage img, bool x, bool y);
        ofPixels drawSeam(ofImage img, bool x, bool y);
        float * getNormalizedVerticalSeamFitness(ofPixels pixels);

        ofPixels seamCarve(ofPixels pixels, bool x, bool y);
        ofPixels seamAdd(ofPixels pixels, int addHSeams, int addVSeams);
    
        ofPixels addVerticalSeam(ofPixels pixels, float * seamFitness, int vSeamsToAdd);
        ofPixels removeVerticalSeam(ofPixels pixels, float * seamFitness, int w, int h);
    
        ofPixels addHorizontalSeam(ofPixels pixels, float * seamFitness, int hSeamsToAdd);
        ofPixels removeHorizontalSeam(ofPixels pixels, float * seamFitness, int w, int h);
        ofPixels drawHorizontalSeam(ofPixels pixels, float * seamFitness, int w, int h);

        bool cmpSeamFit(const SeamFitness &a, const SeamFitness &b);
    
    string gradientShaderString;
    ofShader gradientShader;
    ofFbo gradientFbo;
    
    //float * seamFitness;
   // ofPixels pix;
   // ofTexture tex;
    ofPixels pix, vPix;
    ofImageType imageType;

};