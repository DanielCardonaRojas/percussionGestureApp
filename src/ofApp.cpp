#include "ofApp.h"
#include <Poco/Path.h>

ofEvent <GestureEvent> GestureEvent::gestureDetected;

/*********************************prueba de commit****************************************/
void ofApp::setup(){
    
    openNIDevice.setup();
    openNIDevice.addImageGenerator();
    openNIDevice.addDepthGenerator();
    openNIDevice.setRegister(true);
    openNIDevice.setMirror(true);
    // setup the hand generator
    openNIDevice.addHandsGenerator();
    openNIDevice.addHandFocusGesture("Wave");
    openNIDevice.setMaxNumHands(1);
    openNIDevice.start();
    
    ofAddListener(openNIDevice.handEvent, this, &ofApp::handEvent);
    
    dollar.load(ofToDataPath("gestures.txt",true));
    showMessage("Loaded! Note that saved gestures are rotated to the optimal position!",3000);
    ofAddListener(GestureEvent::gestureDetected, this, &ofApp::gestureDetected);
    
    // setup OF sound stream & Pd patch
    int ticksPerBuffer = 8; // 8 * 64 = buffer len of 512
    int numInputs = 1;
    ofSoundStreamSetup(2, numInputs, this, 44100, ofxPd::blockSize()*ticksPerBuffer, 3);
    if(!pd.init(2, numInputs, 44100, ticksPerBuffer)) {
        OF_EXIT_APP(1);
    }
    pd.start();
    patch = pd.openPatch("VanillaSamplers/Exc2/Ejercicio3.pd");
   
    gestureLineX = new circularBuffer(GESTURE_SIZE);
    gestureLineY = new circularBuffer(GESTURE_SIZE);
    
    estadoActual = WELCOME;
}

/*************************************************************************/
#pragma mark - Callbacks of
void ofApp::update(){
    openNIDevice.update();
    if (showText){
        if (ofGetElapsedTimeMillis() >= hide_message_on){
            showText = false;
            found_gesture.clear();
        }
    }
    else{
        for (int i = 0; i < openNIDevice.getNumTrackedHands(); i++){
            ofxOpenNIHand & hand = openNIDevice.getTrackedHand(i);
            ofPoint & handPosition = hand.getPosition();
            mappedHandPos.x = ofMap(ofClamp(handPosition.x,1,KINECT_W),1,KINECT_W,1,ofGetWidth());
            mappedHandPos.y = ofMap(ofClamp(handPosition.y,1,KINECT_H),1,KINECT_H,1,ofGetHeight());
            ofSetColor(255,0,0);
            ofRect(mappedHandPos.x, mappedHandPos.y, 5, 5);
        }
        
        gestureLineX->write(mappedHandPos.x);
        gestureLineY->write(mappedHandPos.y);
        findGesture();
    }
    
    if(waiting){
        if (ofGetElapsedTimeMillis() >= timeToCount){
            waiting = false;
        }
    }else{
        switch (estadoActual) {
            case WELCOME:
                pd.sendBang(patch.dollarZeroStr()+"-start");
                countTime(24000);
                break;
                
            case CIRCLE:
                pd.sendBang(patch.dollarZeroStr()+"-circle");
                countTime(14000);
                break;
                
            case TRIANGLE:
                pd.sendBang(patch.dollarZeroStr()+"-triangle");
                countTime(14000);
                break;
                
            case SQUARE:
                pd.sendBang(patch.dollarZeroStr()+"-rectangle");
                countTime(2000);
                estadoActual=WELCOME;
                break;
                
            default:
                break;
        }
    }
}

/*************************************************************************/
void ofApp::draw(){
    ofSetColor(255,255,255);
    
    openNIDevice.drawDepth(0,0,ofGetWidth(),ofGetHeight());

    if(showText){
        ofSetColor(255, 0, 140);
        ofDrawBitmapString(message, 10, 30);
    }else{
        ofSetColor(255, 255, 0);
        line.draw();
    }
    
    ofSetColor(0, 255, 140);
    found_gesture.draw();
}


void ofApp::findGesture(){
    ofxGesture* tmp = new ofxGesture();
    line.clear();
    for(int i=gestureLineX->pW;i>gestureLineX->pW-GESTURE_SIZE;i--){
        tmp->addPoint (gestureLineX->buffer[i%GESTURE_SIZE],gestureLineY->buffer[i%GESTURE_SIZE]);
        line.addVertex(ofVec2f(gestureLineX->buffer[i%GESTURE_SIZE],gestureLineY->buffer[i%GESTURE_SIZE]));
    }

    double score = 0.0;
    ofxGesture* match = dollar.match(tmp, &score);
    if(match != NULL && score>0.85) {
        string result = "Matching score: " +ofToString(score);
        result +=", which matches with gesture: " +match->name;
        found_gesture.clear();
        float dx = ofGetWidth()/2;
        float dy = ofGetHeight()/2;
        for(int i = 0; i < match->resampled_points.size(); ++i) {
            found_gesture.addVertex(ofVec2f(dx+match->resampled_points[i].x, dy+match->resampled_points[i].y));
        }
        gestureLineX->resetAtPoint(mappedHandPos.x);
        gestureLineY->resetAtPoint(mappedHandPos.y);
        line.clear();
        static GestureEvent detected;
        detected.message = match->name;
        ofNotifyEvent(GestureEvent::gestureDetected, detected);
        showMessage(result, 500);
    }
    delete tmp;
}

/*************************************************************************/
void ofApp::showMessage(string sMessage) {
    message = sMessage;
    showText = true ;
    hide_message_on = ofGetElapsedTimeMillis()+1000;
}

/*************************************************************************/
void ofApp::showMessage(string sMessage, int nDelay) {
    message = sMessage;
    showText = true ;
    hide_message_on = ofGetElapsedTimeMillis() + nDelay;
}

/*************************************************************************/
void ofApp::countTime(int nDelay) {
    waiting = true ;
    timeToCount = ofGetElapsedTimeMillis() + nDelay;
}

void ofApp::countTimeReset() {
    waiting = false ;
    timeToCount = 0;
}
#pragma mark - Eventos
void ofApp::gestureDetected(GestureEvent &e){
    ofLogNotice() << "Evento de deteccion de un " << e.message << " como gesto";
    if (e.message=="circulo" && estadoActual==WELCOME){
        pd.sendBang(patch.dollarZeroStr()+"-stopAll");
        estadoActual=CIRCLE;
        countTimeReset();
    }
    if (e.message=="triangulo" && estadoActual==CIRCLE){
        pd.sendBang(patch.dollarZeroStr()+"-stopAll");
        estadoActual=TRIANGLE;
        countTimeReset();
    }
    if (e.message=="cuadrado" && estadoActual==TRIANGLE){
        pd.sendBang(patch.dollarZeroStr()+"-stopAll");
        estadoActual=SQUARE;
        countTimeReset();
    }
}

//*************************************************************************/
void ofApp::handEvent(ofxOpenNIHandEvent & event){
    if (event.handStatus!=HAND_TRACKING_UPDATED){
        ofLogNotice() << getHandStatusAsString(event.handStatus) << "for hand" << event.id << "from device" << event.deviceID;
    }
}

//*************************************************************************/
void ofApp::audioReceived(float * input, int bufferSize, int nChannels) {
    pd.audioIn(input, bufferSize, nChannels);
}

//*************************************************************************/
void ofApp::audioRequested(float * output, int bufferSize, int nChannels) {
    pd.audioOut(output, bufferSize, nChannels);
}
