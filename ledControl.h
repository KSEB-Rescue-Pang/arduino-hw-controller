#ifndef LED_CONTROL_H
#define LED_CONTROL_H

void setLED();
void flickerLED(int ledPin, int delayTime);
void manageHardWare(int rssi);
void ledControl(const char* jsonBuffer);
#endif