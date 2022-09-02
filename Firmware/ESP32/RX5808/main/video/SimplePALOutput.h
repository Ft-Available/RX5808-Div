#pragma once
#include "Graphics.h"
const int lineSamples = 854;
const int memSamples = 856;
const int syncSamples = 64;
const int burstSamples = 38;

const int burstStart = 70;
const int frameStart = 160; //must be even to simplify buffer word swap
const int imageSamples = 640;

const int syncLevel = 0;
const int blankLevel = 23;
const int burstAmp = 12;
const int maxLevel = 54;
class SimplePALOutput {
    SimplePALOutput();
public:    
    unsigned int lineCounts;
    unsigned short longSync[memSamples];
    unsigned short shortSync[memSamples];
    unsigned short line[2][memSamples];
    unsigned short blank[memSamples];
    short SIN[imageSamples];
    short COS[imageSamples];

    short YLUT[16];
    short UVLUT[16];
    Graphics *graph;

    static SimplePALOutput *GetInstance() {
        static SimplePALOutput comp;
        return &comp;
    }
    void start(Graphics *gra);
    void stop();
    bool iterator();
};


