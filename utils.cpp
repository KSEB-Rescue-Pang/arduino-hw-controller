#include <ArduinoJson.h>
#include "utils.h"

bool parseJson(const char* jsonBuffer, StaticJsonDocument<100>& doc) {
  DeserializationError error = deserializeJson(doc, jsonBuffer);

  if (error) {
    Serial.print(F("  deserializeJson() failed: "));
    Serial.println(error.f_str());
    return false;
  }
  return true;
}