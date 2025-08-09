#pragma once
#include <ArduinoJson.h>
bool parseJson(const char* jsonBuffer, StaticJsonDocument<200>& doc);
