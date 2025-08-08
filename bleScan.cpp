#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include "ledControl.h"

int lastRSSI;
BLEScan* pBLEScan;
const int scanTime = 1;

void processTasks() {
  if (!pBLEScan) {
    BLEDevice::init("");
    pBLEScan = BLEDevice::getScan(); 
    pBLEScan->setActiveScan(true); 
  }

  BLEScanResults foundDevices = *pBLEScan->start(scanTime, false);
  int maxRSSI = -100;
  for (int i = 0; i < foundDevices.getCount(); i++) {
    BLEAdvertisedDevice device = foundDevices.getDevice(i);
    int8_t rssi = device.getRSSI();
    if (device.haveName() && device.getName().indexOf("Quantum3") != -1) {
      Serial.print("Detected: "); Serial.print(device.getName());
      Serial.print(" RSSI: "); Serial.println(rssi);
      if (rssi > maxRSSI) maxRSSI = rssi;
    }
  }
  lastRSSI = maxRSSI;
  Serial.print("Selected RSSI: "); Serial.println(lastRSSI);
  manageHardWare(lastRSSI,ledType);
  pBLEScan->clearResults();
}
