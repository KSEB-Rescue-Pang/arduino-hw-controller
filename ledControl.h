#ifndef LED_CONTROL_H
#define LED_CONTROL_H

void setLED();
void flickerLED(int ledPin, int delayTime);
void getReadyBeforeFlickering(int led);
void turnOffLED(int ledType);
void turnOnLED(const char* ledType);
int getPinForLED(const char* ledType);
//void ledControl(const char* jsonBuffer);
//void ledControlComplete(const char* jsonBuffer);
#endif