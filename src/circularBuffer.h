#ifndef __rioCachivera__circularBuffer__
#define __rioCachivera__circularBuffer__

#include "ofMain.h"

class circularBuffer{
public:
    circularBuffer(int size);
    void write(float value);
    float sumDiff();
    void randomReset();
    void clear();
    void print();
    float mean();
    float lerped();
    void resetAtPoint(int point);  //modificacion para uso en gesture tracking-detection
    
    int size;
    float* buffer;
    int pW;

};

#endif
