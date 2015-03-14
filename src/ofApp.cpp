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
    openNIDevice.setMaxNumHands(2);
    openNIDevice.start();
    
    ofAddListener(openNIDevice.handEvent, this, &ofApp::handEvent);
    
//    dollar.load(ofToDataPath("gestures.txt",true));
//    showMessage("Loaded! Note that saved gestures are rotated to the optimal position!",3000);
//    ofAddListener(GestureEvent::gestureDetected, this, &ofApp::gestureDetected);
//    
//    // setup OF sound stream & Pd patch
//    int ticksPerBuffer = 8; // 8 * 64 = buffer len of 512
//    int numInputs = 1;
//    ofSoundStreamSetup(2, numInputs, this, 44100, ofxPd::blockSize()*ticksPerBuffer, 3);
//    if(!pd.init(2, numInputs, 44100, ticksPerBuffer)) {
//        OF_EXIT_APP(1);
//    }
//    pd.start();
//    patch = pd.openPatch("VanillaSamplers/Exc2/Ejercicio3.pd");
//   
    for (int i = 0; i < 2; i++){
        smoothPosX.push_back(new circularBuffer(5));
        smoothPosY.push_back(new circularBuffer(5));
        percGestAnalizer.push_back(new Analizador());
    }
    ofAddListener(GestureEvent::gestureDetected, this, &ofApp::gestureDetected);
}

/*************************************************************************/
#pragma mark - Callbacks of
void ofApp::update(){
    openNIDevice.update();
    for (int i = 0; i < openNIDevice.getNumTrackedHands(); i++){
        ofxOpenNIHand & hand = openNIDevice.getTrackedHand(i);
        ofPoint & handPosition = hand.getPosition();
        mappedHandPos.x = ofMap(ofClamp(handPosition.x,1,KINECT_W),1,KINECT_W,1,ofGetWidth());
        mappedHandPos.y = ofMap(ofClamp(handPosition.y,1,KINECT_H),1,KINECT_H,1,ofGetHeight());
        smoothPosX[i]->write(mappedHandPos.x);
        smoothPosY[i]->write(mappedHandPos.y);
    }
    
    if (showText){
        if(ofGetElapsedTimeMillis() >= hide_message_on){
            showText = false;
        }
    }
    else{
        findGesture();
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
        for (int i = 0; i < openNIDevice.getNumTrackedHands(); i++){
            ofSetColor(255,0,0);
            ofRect(smoothPosX[i]->mean(), smoothPosY[i]->mean(), 20, 20);
        }
    }
}

/*************************************************************************/
#pragma mark - Funciones
/*************************************************************************/
void ofApp::findGesture(){
    for (int i = 0; i < 2; i++){
        if (percGestAnalizer[i]->calcularTodo(smoothPosY[i]->mean()) == "hit" ){
            static GestureEvent detected;
            detected.message = "PercHit" +  ofToString(i);
            ofNotifyEvent(GestureEvent::gestureDetected, detected);
            showMessage( detected.message , 50 );
        }
    }
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

/*************************************************************************/
void ofApp::countTimeReset() {
    waiting = false ;
    timeToCount = 0;
}

/*************************************************************************/
#pragma mark - Eventos
/*************************************************************************/
void ofApp::gestureDetected(GestureEvent &e){
    ofLogNotice() << "Evento de deteccion de un " << e.message << " como gesto";
    if (e.message=="PercHit0"){
        
    }
    else if (e.message=="PercHit1"){
    
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
