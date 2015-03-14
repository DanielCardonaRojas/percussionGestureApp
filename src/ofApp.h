#pragma once

#include "ofMain.h"
#include "ofxOpenNI.h"
#include "ofxOneDollar.h"
#include "circularBuffer.h"
#include "analizador.h"
#include "ofxPd.h"

using namespace pd;

#define KINECT_W 640
#define KINECT_H 480
#define GESTURE_SIZE 100

enum State{
    WELCOME, CIRCLE, TRIANGLE, SQUARE
};

class GestureEvent : public ofEventArgs {
    
public:
    string   message;
    
    GestureEvent() {
    }
    
    static ofEvent <GestureEvent> gestureDetected;
};

class ofApp : public ofBaseApp{

    public:
        void setup();
        void update();
        void draw();
    
        void audioReceived(float * input, int bufferSize, int nChannels);
        void audioRequested(float * output, int bufferSize, int nChannels);
    
        void showMessage(string sMessage);
        void showMessage(string sMessage, int nDelay);
        void countTime(int nDelay);
        void countTimeReset();
        void findGesture();
        void gestureDetected(GestureEvent &e);
    
    private:
        void handEvent(ofxOpenNIHandEvent & event);
    
        ofxOpenNI       openNIDevice;
        ofPoint         mappedHandPos;
        ofxOneDollar    dollar;
        ofxGesture*     gesture;
        bool            showText;
        string          message;
        int             hide_message_on;
        circularBuffer* gestureLineX;
        circularBuffer* gestureLineY;
        ofPolyline      line,found_gesture;
        double          score=0.0;
        ofxPd           pd;
        Patch           patch;
        vector<float>   scopeArray;
        State           estadoActual;
        int             timeToCount;
        bool            waiting=false;
    
        vector<circularBuffer*> smoothPosX;
        vector<circularBuffer*> smoothPosY;
    
        vector<Analizador*>     percGestAnalizer;
    
};
