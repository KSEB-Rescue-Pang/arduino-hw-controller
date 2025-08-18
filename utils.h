#pragma once
#include <ArduinoJson.h>
bool parseJson(const char* jsonBuffer, StaticJsonDocument<512>& doc);