/*********************************************************************
 This is an example for our nRF52 based LE modules

 Pick one up today in the adafruit shop!

 MIT license, check LICENSE for more information
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/

/* This sketches demontrates the Bluefruit.Advertising API(). When powered up,
 * the Bluefruit module will start advertising for ADV_TIMEOUT seconds (by
 * default 30 seconds in fast mode, the remaining time slow mode) and then
 * stop advertising completely. The module will start advertising again if
 * PIN_ADV is grounded.
 */
#include <bluefruit.h>
#include "pwrm.h"
#include "pca10040.h"


#define ADV_TIMEOUT   240 // seconds (6 minutes)
#define MANUFACTURER_ID   0x004C 

// AirLocate UUID: E2C56DB5-DFFB-48D2-B060-D0F5A71096E0
uint8_t beaconUuid[16] = 
{ 
  0xE2, 0xC5, 0x6D, 0xB5, 0xDF, 0xFB, 0x48, 0xD2, 
  0xB0, 0x60, 0xD0, 0xF5, 0xA7, 0x10, 0x96, 0xE0, 
};

// A valid Beacon packet consists of the following information:
// UUID, Major, Minor, RSSI @ 1M
BLEBeacon beacon(beaconUuid, 0x0000, 0x0000, -64);

void setup() 
{
  SwitchOffPeripherals();
  configure_ram_retention();
  nrf_gpio_cfg_sense_input(BUTTON_4, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);

  #if 0
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("IoThingsWare52 Advanced Advertising Example");
  Serial.println("----------------------------------------\n");
  #endif

  Bluefruit.begin();
    // off Blue LED for lowest power consumption
  Bluefruit.autoConnLed(false);
  

  // Set max power. Accepted values are: -40, -30, -20, -16, -12, -8, -4, 0, 4
  Bluefruit.setTxPower(-20);
  Bluefruit.setName("IoThingsWare");

  // Manufacturer ID is required for Manufacturer Specific Data
  beacon.setManufacturer(MANUFACTURER_ID);

  // Set up and start advertising
  startAdv();
  //Serial.println("Advertising is started");
  // Suspend Loop() to save power, since we didn't have any code there
  suspendLoop();

}

void startAdv(void)
{   
  // Advertising packet
  // Set the beacon payload using the BLEBeacon class populated
  // earlier in this example
  // Secondary Scan Response packet (optional)
  // Since there is no room for 'Name' in Advertising packet
  Bluefruit.ScanResponse.addName();

  /* 
   * Apple Beacon specs
   * - Type: Non connectable, undirected
   * - Fixed interval: 100 ms -> fast = slow = 100 ms
   */

  Bluefruit.Advertising.setBeacon(beacon);

  /* Start Advertising
   * - Enable auto advertising if disconnected
   * - Interval:  fast mode = 200 ms, slow mode = 2000 ms
   * - Timeout for fast mode is 10 seconds
   * - Start(timeout) with timeout = 240 sec (6 min) 
   * - Start(timeout) with timeout = 0 will advertise forever (until connected)
   * 
   * For recommended advertising interval
   * https://developer.apple.com/library/content/qa/qa1931/_index.html
   */
    //Bluefruit.Advertising.setType(BLE_GAP_ADV_TYPE_ADV_NONCONN_IND);
  Bluefruit.Advertising.setStopCallback(adv_stop_callback);
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(320, 3200);    // in units of 0.625 ms 
  Bluefruit.Advertising.setFastTimeout(10);      // number of seconds in fast mode
  Bluefruit.Advertising.start(ADV_TIMEOUT);      // Stop advertising entirely after ADV_TIMEOUT seconds 
}


/**
 * Callback invoked when advertising is stopped by timeout
 */
void adv_stop_callback(void)
{
  __WFE();
  //Serial.println("Advertising time passed, advertising will now stop.");
  DeepSleep();
  __WFI();
}

void loop(void)
{
  
}
