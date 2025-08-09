//#include <ArduinoJson.h>
#include "utils.h"

bool parseJson(const char* jsonBuffer, StaticJsonDocument<100>& doc) {
  DeserializationError error = deserializeJson(doc, jsonBuffer);

  if (error) {
    #ifdef DEBUG
      Serial.print(F("  deserializeJson() failed: "));
      Serial.println(error.f_str());
    #endif
    return false;
  }
  return true;
}