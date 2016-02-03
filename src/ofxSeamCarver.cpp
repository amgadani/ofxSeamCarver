#include "ofxSeamCarver.h"

void ofxSeamCarver::setup(int width, int height, ofImageType imgType) {
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
    
    imageType = imgType;
    pix.allocate(width, height, imageType);
    vPix.allocate(width, height, imageType);
    gradientFbo.allocate(width, height, GL_RGB32F_ARB);
}

ofPixels ofxSeamCarver::seamCarve(ofPixels pixels, bool x, bool y) {
    int width = pixels.getWidth();
    int height = pixels.getHeight();

    if(pix.size() != width*height*pix.getNumChannels()) {
        pix.allocate(width, height, imageType);
        vPix.allocate(width, height, imageType);
    }
    gradientFbo.allocate(width, height, GL_RGB32F_ARB);

    if (x) {
        computeGradient(pixels, width, height, gradientFbo);
        float * verticalSeams = computeVerticalSeamFitness(gradientFbo,width, height);
        pixels = removeVerticalSeam(pixels, verticalSeams, width--, height);
    }
    if (y) {
        computeGradient(pixels, width, height, gradientFbo);
        float * horizontalSeams = computeHorizontalSeamFitness(gradientFbo,width, height);
        pixels = removeHorizontalSeam(pixels, horizontalSeams, width, height--);
        
    }
    pix.clear();
    vPix.clear();
    return pixels;
}

ofPixels ofxSeamCarver::drawSeam(ofImage img, bool x, bool y) {
    ofPixels pixels = img.getPixels();
    int width = pixels.getWidth();
    int height = pixels.getHeight();
    if(pix.size() != width*height*pix.getNumChannels()) {
        pix.allocate(width, height, imageType);
        vPix.allocate(width, height, imageType);
        gradientFbo.allocate(width, height, GL_RGB32F_ARB);
    }
    if (x) {
        computeGradient(pixels, width, height, gradientFbo);
        float * verticalSeams = computeVerticalSeamFitness(gradientFbo, width, height);
        pixels = removeVerticalSeam(pixels, verticalSeams, width--, height);
    }
    if (y) {
        computeGradient(pixels, width, height, gradientFbo);
        float * horizontalSeams = computeHorizontalSeamFitness(gradientFbo, width, height);
        pixels = drawHorizontalSeam(pixels, horizontalSeams, width, height);
        
    }
    pix.clear();
    vPix.clear();
    return pixels;
}

ofPixels ofxSeamCarver::seamCarve(ofImage img, bool x, bool y) {
    ofPixels pixels = img.getPixels();
    return seamCarve(pixels, x, y);
}

ofPixels ofxSeamCarver::seamAdd(ofPixels pixels, int addHSeams, int addVSeams) {
    int width = pixels.getWidth();
    int height = pixels.getHeight();
        pix.allocate(width, height, imageType);
        vPix.allocate(width, height, imageType);
        gradientFbo.allocate(width, height, GL_RGB32F_ARB);
    cout << "Add Seam: Start" << endl;
    cout << width << " " << height << " " <<  addHSeams << " " << addVSeams << endl;
    if (addVSeams > 0) {
        computeGradient(pixels, width, height, gradientFbo);
        float * verticalSeams = computeVerticalSeamFitness(gradientFbo, width, height);
        pixels = addVerticalSeam(pixels, verticalSeams, addVSeams);
        width += addVSeams;
    }
    cout << width << " " << height << " " <<  addHSeams << " " << addVSeams << endl;
    gradientFbo.allocate(width, height, GL_RGB32F_ARB);
    if (addHSeams > 0) {
        computeGradient(pixels, width, height, gradientFbo);
        float * horizontalSeams = computeHorizontalSeamFitness(gradientFbo, width, height);
        pixels = addHorizontalSeam(pixels, horizontalSeams, addHSeams);
        height += addHSeams;
        
    }
    cout << width << " " << height << " " <<  addHSeams << " " << addVSeams << endl;
    cout << "Add Seam: End" << endl;
    pix.clear();
    vPix.clear();
    return pixels;
}

void ofxSeamCarver::computeGradient(ofPixels pixels, int width, int height, ofFbo gradFbo) {
    gradientFbo.begin();
    ofClear(0,0,0);
    gradientFbo.end();
    
    ofTexture texture;
    texture.loadData(pixels);
    gradFbo.begin();
    gradientShader.begin();
    gradientShader.setUniformTexture("pic", texture, 1);
    renderFrame(width, height);
    gradientShader.end();
    gradFbo.end();
    
//    cout << "--Gradient: " << gradFbo.getWidth() << "\t" << gradFbo.getHeight()
//        << "\n--Texture: " << texture.getWidth() << "\t" << texture.getHeight()
//        << "\n--Texture: " << pixels.getWidth() << "\t" << pixels.getHeight()
//        << "\n--Texture: " << width << "\t" << height<< "\n";
}

float * ofxSeamCarver::computeVerticalSeamFitness(ofFbo gradFbo, int width, int height) {
    cout<<"vertical\n";
    gradFbo.readToPixels(vPix);
    
    int w = width;
    int h = height;

    float * seamFitness = new float[w*h];
    for (int x = 0; x < w; x++) {
        seamFitness[x] = vPix.getColor(x, 0).getBrightness();
    }
    for (int y = 1; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int index = x + w * y;
            seamFitness[index] = vPix.getColor(x,y).getBrightness();
            int left = max(x - 1,0) + w * (y-1);
            int right = min(x + 1, w-1) + w * (y-1);
            int center = x + w * (y-1);
            seamFitness[index] += min(min(seamFitness[left], seamFitness[center]), seamFitness[right]);
        }
    }
    //pix.clear();
    
    cout << w << "\t" << h << endl;

    return seamFitness;
}

float * ofxSeamCarver::getNormalizedVerticalSeamFitness(ofPixels pixels){
    int width = pixels.getWidth();
    int height = pixels.getHeight();
    gradientFbo.allocate(width, height, GL_RGB32F_ARB);

    computeGradient(pixels, width, height, gradientFbo);
    cout << "GRAD PIXELS NOW: " << gradientFbo.getWidth() << "\t" << gradientFbo.getHeight() << endl;

    float * fitness = computeVerticalSeamFitness(gradientFbo, width, height);
    
    float max = -10000;
    for (int x = 0; x < width*height; x++) {
        if (max < fitness[x]) {
            max = fitness[x];
        }
    }
    cout << "--Gradient: " << width << "\t" << height
        << "\n--Texture: " << pixels.getWidth() << "\t" << pixels.getHeight() << "\n";
    for (int x = 0; x < width*height; x++) {
        fitness[x]/=max;
    }
    return fitness;
}

float * ofxSeamCarver::computeHorizontalSeamFitness(ofFbo gradFbo, int width, int height) {
    cout<<"horizontal\n";
    gradFbo.readToPixels(pix);
    
    int w = width;
    int h = height;

    float * seamFitness = new float[w*h];
    for (int y = 0; y < h; y++) {
        seamFitness[y*w] = pix.getColor(w-1, y).getBrightness();
    }
    for (int x = 1; x < w; x++) {
        for (int y = 0; y < h; y++) {
            int index = x + w * y;
            seamFitness[index] = pix.getColor(x,y).getBrightness();
            int below = x-1 + min(y+1,h-1)*w;
            int above = x-1 + max(y-1,0)*w;
            int center = x-1 + w * y;
            seamFitness[index] += min(min(seamFitness[below], seamFitness[center]), seamFitness[above]);
        }
    }
    cout << w << "\t" << h << endl;

  //  pix.clear();
    return seamFitness;
}

bool ofxSeamCarver::cmpSeamFit(const SeamFitness &a, const SeamFitness &b) {
    if (a.fitness <= b.fitness) {
        return true;
    }
    return false;
}

ofPixels ofxSeamCarver::addVerticalSeam(ofPixels pixels, float * seamFitness, int vSeamsToAdd) {
    ofPixels grown;
    int w = pixels.getWidth();
    int h = pixels.getHeight();
    grown.allocate(w + vSeamsToAdd, h, imageType);
    
    vector<SeamFitness> seamFit;
    for (int i = 0 ;  i < w; i++) {
        SeamFitness temp;
        temp.index = i;
        temp.fitness = seamFitness[i + w*(h-1)];
        seamFit.push_back(temp);
    }
    sort(seamFit.begin(), seamFit.end());
    vector<SeamFitness> seamsToAdd(seamFit.end() - vSeamsToAdd, seamFit.end());
    reverse(seamsToAdd.begin(), seamsToAdd.end());
    
    for (int i = 0; i < seamsToAdd.size(); i++) {
        SeamFitness currSeamFit = seamsToAdd[i];
        int minColumn = currSeamFit.index;
        
        for (int y = h-1; y >= 0; y--) {
            bool addedColumn = false;
            
            for (int x = 0; x < w+i+1; x++) {
                ofColor newColor;
                if (x == minColumn+1) {
                    addedColumn = true;
                    newColor = pixels.getColor(x-1, y).getLerped(pixels.getColor(x+1, y), 0.5);
                } else {
                    newColor = pixels.getColor(addedColumn ? x-1 : x, y);
                }

                grown.setColor(x, y, newColor);
            }
            
            if (y > 0) {
                float theMin;
                if (minColumn > 0 && seamFitness[minColumn-1 + w * (y-1)] <= theMin) {
                    minColumn = minColumn - 1;
                } else if (minColumn < w-1 && seamFitness[minColumn+1 + w * (y-1)] <= theMin) {
                    minColumn = minColumn + 1;
                } else {
                    theMin = seamFitness[minColumn+w*(y-1)];
                }
            }
        }
    }
    return grown;
}

ofPixels ofxSeamCarver::addHorizontalSeam(ofPixels pixels, float * seamFitness, int hSeamsToAdd) {
    ofPixels grown;
    int w = pixels.getWidth();
    int h = pixels.getHeight();
    grown.allocate(w, h + hSeamsToAdd, imageType);
    
    vector<SeamFitness> seamFit;
    for (int i = 0 ;  i < h; i++) {
        SeamFitness temp;
        temp.index = i;
        temp.fitness = seamFitness[w-1 + w*i];
        seamFit.push_back(temp);
    }
    sort(seamFit.begin(), seamFit.end());
    vector<SeamFitness> seamsToAdd(seamFit.end() - hSeamsToAdd, seamFit.end());
    reverse(seamsToAdd.begin(), seamsToAdd.end());
    
    
    for (int i = 0; i < seamsToAdd.size(); i++) {
        SeamFitness currSeamFit = seamsToAdd[i];
        int minRow = currSeamFit.index;
        cout << h << " " << minRow << endl;

        for (int x = w-1; x >= 0; x--) {
            bool addedRow = false;
            
            for (int y = 0; y < h+i+1; y++) {
                ofColor newColor;
                if (y == minRow+1) {
                    addedRow = true;
                    if ( y == h + i) {
                        newColor = pixels.getColor(x, y-1);
                    } else {
                        newColor = pixels.getColor(x, y-1).getLerped(pixels.getColor(x, y+1), 0.5);
                    }
                } else {
                    newColor = pixels.getColor(x, addedRow ? y-1 : y);
                }

                grown.setColor(x, y, newColor);
            }
            if (x > 0) {
                float theMin;
                if (minRow > 0 && seamFitness[x-1 + w * (minRow-1)] <= theMin) {
                    minRow = minRow - 1;
                } else if (minRow < h-1 && seamFitness[x-1 + w * (minRow+1)] <= theMin) {
                    minRow = minRow + 1;
                } else {
                    theMin = seamFitness[x-1+w*(minRow)];
                }
            }
        }
    }
    return grown;
}

ofPixels ofxSeamCarver::removeVerticalSeam(ofPixels pixels, float * seamFitness, int w, int h) {
    ofPixels trimmed;
    trimmed.allocate(w-1,h, imageType);
    
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
            int newIndex = (x + y*(w-1));
            int oldIndex = (skippedColumn ? x+1 : x) + y*w;
            
            trimmed.setColor(x, y, pixels.getColor(skippedColumn ? x+1 : x, y));
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

ofPixels ofxSeamCarver::removeHorizontalSeam(ofPixels pixels, float * seamFitness, int w, int h) {
    ofPixels trimmed;
    trimmed.allocate(w,h-1, imageType);
    
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
            int newIndex = (x + y*w);
            int oldIndex = (x + (skippedBestRow ? y+1 : y)*w);
            
            trimmed.setColor(x, y, pixels.getColor(x, skippedBestRow ? y+1 : y));
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

ofPixels ofxSeamCarver::drawHorizontalSeam(ofPixels pixels, float * seamFitness, int w, int h) {
    ofPixels trimmed;
    trimmed.allocate(w,h, imageType);
    
    int minRow = 0;
    for (int i = 0 ;  i < h; i++) {
        if(seamFitness[w-1 + w*minRow] > seamFitness[w-1 + w*i])
        {
            minRow = i;
        }
    }
    for (int x = w-1; x >= 0; x--) {
        for (int y = 0; y < h-1; y++) {
            if (y == minRow) {
                trimmed.setColor(x, y, ofColor(255,0,0));
            } else {
                trimmed.setColor(x, y, pixels.getColor(x, y));
            }
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