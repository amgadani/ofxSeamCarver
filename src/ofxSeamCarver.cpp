#include "ofxSeamCarver.h"

void ofxSeamCarver::setup(int width, int height, int num_channels) {
    gradientShaderString = STRINGIFY(
                                     uniform sampler2DRect pic;
                                     
                                     void main() {
                                         vec2 x1 = vec2(1.0, 0.);
                                         vec2 y1 = vec2(0., 1.0);
                                         vec2 st = gl_TexCoord[0].st;
                                         
                                         float gradx = length(texture2DRect(pic, st+x1)-texture2DRect(pic, st-x1));
                                         float grady = length(texture2DRect(pic, st+y1)- texture2DRect(pic, st-y1));
                                         gl_FragColor = vec4(vec3(sqrt(gradx*gradx + grady*grady)),1.0);
                                         
                                         
                                     });
    gradientShader.setupShaderFromSource(GL_FRAGMENT_SHADER, gradientShaderString);
    gradientShader.linkProgram();

    components = num_channels;
    


}

unsigned char * ofxSeamCarver::seamCarve(ofImage img, bool x, bool y) {
    cout << img.width << " " << img.height << " \n";
    unsigned char * pixels = img.getPixels();
    int width = img.width;
    int height = img.height;
    pix.allocate(width, height, img.bpp/8);
    if (x) {
        gradientFbo.allocate(width, height, GL_RGB);

        computeGradient(pixels, width, height, gradientFbo);
        float * verticalSeams = computeVerticalSeamFitness(gradientFbo,width, height);
        pixels = removeVerticalSeam(pixels, verticalSeams, width--, height);
    }
    if (y) {
        gradientFbo.allocate(width, height, GL_RGB);

        computeGradient(pixels, width, height, gradientFbo);
        float * horizontalSeams = computeHorizontalSeamFitness(gradientFbo,width, height);
        pixels = removeHorizontalSeam(pixels, horizontalSeams, width, height--);

    }
    pix.clear();
    return pixels;
    
}

void ofxSeamCarver::computeGradient(unsigned char * pixels, int width, int height, ofFbo gradFbo) {
    ofTexture texture;
    texture.loadData(pixels, width, height, GL_RGB);
    gradFbo.begin();
    gradientShader.begin();
    gradientShader.setUniformTexture("pic", texture, 1);
    renderFrame(width, height);
    gradientShader.end();
    gradFbo.end();
}

float * ofxSeamCarver::computeVerticalSeamFitness(ofFbo gradFbo, int width, int height) {
    gradFbo.getTextureReference().readToPixels(pix);
    int channels = pix.getNumChannels();
    int w = width;
    int h = height;

    float * seamFitness = new float[w*h];
    for (int x = 0; x < w; x++) {
        seamFitness[x] = pix[x*channels];
    }
    for (int y = 1; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int index = x + w * y;
            seamFitness[index] = pix[index*channels];
            int left = max(x - 1,0) + w * (y-1);
            int right = min(x + 1, w-1) + w * (y-1);
            int center = x + w * (y-1);
            seamFitness[index] += min(min(seamFitness[left], seamFitness[center]), seamFitness[right]);
        }
    }
    //pix.clear();
    return seamFitness;
}

float * ofxSeamCarver::computeHorizontalSeamFitness(ofFbo gradFbo, int width, int height) {
    gradFbo.getTextureReference().readToPixels(pix);
    int channels = pix.getNumChannels();
    int w = width;
    int h = height;

    float * seamFitness = new float[w*h];
    for (int y = 0; y < h; y++) {
        seamFitness[y*w] = pix[y*w*channels];
    }
    for (int x = 1; x < w; x++) {
        for (int y = 0; y < h; y++) {
            int index = x + w * y;
            seamFitness[index] = pix[index*channels];
            int below = x-1 + min(y+1,h-1)*w;
            int above = x-1 + max(y-1,0)*w;
            int center = x-1 + w * y;
            seamFitness[index] += min(min(seamFitness[below], seamFitness[center]), seamFitness[above]);
        }
    }
  //  pix.clear();
    return seamFitness;
}

unsigned char * ofxSeamCarver::removeVerticalSeam(unsigned char * pixels, float * seamFitness, int w, int h) {
    unsigned char * trimmed = new unsigned char[(w-1)* h * components];
    int minColumn = 0;
    for (int i = 0 ;  i < w; i++) {
        if(seamFitness[minColumn + w*(h-1)] > seamFitness[i+ w*(h-1)])
        {
            minColumn = i;
        }
    }
    for (int y = h-1; y >= 0; y--) {
        bool skippedColumn = false;
        
        for (int x = 0; x < w-1; x++) {
            if (x == minColumn) {
                skippedColumn = true;
                
            }
            int newIndex = (x + y*(w-1)) * components;
            int oldIndex = ((skippedColumn ? x+1 : x) + y*w) * components;


            trimmed[newIndex] = pixels[oldIndex];
            trimmed[newIndex+1] = pixels[oldIndex+1];
            trimmed[newIndex+2] = pixels[oldIndex+2];
        }
        
        if (y > 0) {
            float theMin = seamFitness[minColumn+w*(y-1)];
            if (minColumn > 0 && seamFitness[minColumn-1 + w * (y-1)] <= theMin) {
                minColumn = minColumn - 1;
            } else if (minColumn < w-1 && seamFitness[minColumn+1 + w * (y-1)] <= theMin) {
                minColumn = minColumn + 1;
            }
        }
    }
    return trimmed;
}

unsigned char * ofxSeamCarver::removeHorizontalSeam(unsigned char * pixels, float * seamFitness, int w, int h) {
    unsigned char * trimmed = new unsigned char[w * (h-1) * components];
    
    int minRow = 0;
    for (int i = 0 ;  i < h; i++) {
        if(seamFitness[w-1 + w*minRow] > seamFitness[w-1 + w*i])
        {
            minRow = i;
        }
    }
    for (int x = w-1; x >= 0; x--) {
        bool skippedBestRow = false;
        
        for (int y = 0; y < h-1; y++) {
            if (y == minRow) {
                skippedBestRow = true;
            }
            int newIndex = (x + y*w) * components;
            int oldIndex = (x + (skippedBestRow ? y+1 : y)*w) * components;
            trimmed[newIndex] = pixels[oldIndex];
            trimmed[newIndex+1] = pixels[oldIndex+1];
            trimmed[newIndex+2] = pixels[oldIndex+2];
        }
        if (x > 0) {
            float theMin = seamFitness[x-1+w*(minRow)];
            if (minRow > 0 && seamFitness[x-1 + w * (minRow-1)] <= theMin) {
                minRow = minRow - 1;
            } else if (minRow < h-1 && seamFitness[x-1 + w * (minRow+1)] <= theMin) {
                minRow = minRow + 1;
            }
        }
    }
    return trimmed;
}


void ofxSeamCarver::renderFrame(int _width, int _height) {
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(0, 0, 0);
    glTexCoord2f(_width, 0); glVertex3f(_width, 0, 0);
    glTexCoord2f(_width, _height); glVertex3f(_width, _height, 0);
    glTexCoord2f(0,_height);  glVertex3f(0,_height, 0);
    glEnd();
}