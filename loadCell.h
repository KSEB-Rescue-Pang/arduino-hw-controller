#ifndef LOAD_CELL_H
#define LOAD_CELL_H
#include "HX711.h"
void setLoadCell();
//void calibrateWeight(int led);
bool calibrateWeightStep(int led, int idx, float outBoundStandard);
float myFabs(float x);
float myRoundf2(float x);
#endif