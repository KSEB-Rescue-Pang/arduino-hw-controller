#ifndef LED_CONTROL_H
#define LED_CONTROL_H

void setLED();
void flickerLED(int ledPin, int delayTime);
void manageHardWare(int rssi, const char* led);
void turnOffLED(const char* ledType);
void turnOnLED(const char* ledType);
int getPinForLED(const char* ledType);
void ledControl(const char* jsonBuffer);
extern char ledType[10];
extern char workerId[10];
#endif