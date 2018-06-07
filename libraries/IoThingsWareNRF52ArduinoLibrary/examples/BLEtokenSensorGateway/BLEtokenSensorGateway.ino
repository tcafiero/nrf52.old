#include <cppQueue.h>



/*
    Bluetooth Token Gateway
    IoThingsWare All Right Reserved (Toni Cafiero)
*/

// include ESP32 BLE Arduino Library https://github.com/nkolban/ESP32_BLE_Arduino
#include "BLEDevice.h"
#include "BLEAdvertisedDevice.h"
#include "BLEBeacon.h"

#include <WiFi.h>

// include Timer library https://github.com/JChristensen/Timer
#include "Timer.h"
Timer t;
uint64_t chipid;

char output[200];

char format[] = "{\"gateway\":\"%llx\",\"token\":{\"address\":\"%s\",\"name\":\"%s\",\"rssi\":%d,\"value\":%d}}";

BLEScan* pBLEScan;
BLEBeacon myBeacon;

#define  IMPLEMENTATION  FIFO

Queue  q(sizeof(output), 10, IMPLEMENTATION); // Instantiate queue

void MQTTpublish(const char*, char*);
void MQTTloop();
void MQTTconnect();
void MQTTdisconnect();

int scanTime = 10; //In seconds
unsigned long int PublishingPeriod = 2000; //In milliseconds
unsigned long int ScaningPeriod = 30000; //In milliseconds

uint16_t _bswap16(uint16_t a)
{
  a = ((a & 0x00FF) << 8) | ((a & 0xFF00) >> 8);
  return a;
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
      if (advertisedDevice.haveName() && (!strcmp(advertisedDevice.getName().c_str(), "IoThingsWare") || !strcmp(advertisedDevice.getName().c_str(), "IoThingsWare"))) {
        myBeacon.setData(advertisedDevice.getManufacturerData());
        sprintf(output, format, chipid, (char*)advertisedDevice.getAddress().toString().c_str(), (char*)advertisedDevice.getName().c_str(), advertisedDevice.getRSSI(), _bswap16(myBeacon.getMinor()));
        q.push(output);
        Serial.print("<");
      }
    }
};


void PublishCallback()
{
  BLEAdvertisedDevice advertisedDevice;
  if (q.pop(output)) {
    MQTTconnect();
    MQTTpublish("/token/", output);
    Serial.print(">");
    MQTTloop();
    MQTTdisconnect();
  }
}

void ScanCallback()
{
  int i = 10;
  while (1)
  {
    i--;
    BLEScanResults foundDevices = pBLEScan->start(scanTime);
    if (!foundDevices.getCount()) {
      if (!i)ESP.restart();
    }
    else return;
  }
}




void setup() {
  Serial.begin(115200);
  delay(1000);
  BLEDevice::init("");
  delay(1000);
  chipid = ESP.getEfuseMac();
  setup_wifi();
  delay(1000);
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  t.every(ScaningPeriod, ScanCallback);
  t.every(PublishingPeriod, PublishCallback);
}

void loop() {
  t.update();
}
