#pragma once
#include "ofMain.h"
#include "ofVbo.h"

#define NUM_BILLBOARDS 50000
#include "ofxUI.h"
#include "ofxDuration.h"
#include "ofxPostProcessing.h"
class testApp : public ofBaseApp {
public:
    void setup();
    void exit();
    void update();
    void draw();
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y);
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    void guiEvent(ofxUIEventArgs &e);
    ofVec3f cameraRotation;
    float zoom, zoomTarget;
    
    // billboard particles
    float billboardSizeTarget[NUM_BILLBOARDS];
    
    ofShader billboardShader;
    ofImage texture;
    
    ofVboMesh billboards;
    ofVec3f billboardVels[NUM_BILLBOARDS],billboardNormal[NUM_BILLBOARDS];
	
    float age[NUM_BILLBOARDS];
    float past,pX,pY;
    
    ofVec3f emitter,pEmitter;
    
    ofImage emitterTex;
    
    ofxUICanvas *gui;
    
//    gui variable
    float noiseStrength,noisePower,aging,colorChange,particleSizeMax, particleSizeMin;
    float noiseEmitterStrength;
    bool autoEmitt,bRecord;
    ofColor bgColor;
	ofxDuration duration;
	void trackUpdated(ofxDurationEventArgs& args);
	
	ofxPostProcessing post;
	ofEasyCam cam;
};









