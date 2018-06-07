// Include the libraries we need
#define RBL_NRF51822 1
#include <OneWire.h>
#include <DallasTemperature.h>
#define DEBUG 0
#include <bluefruit.h>
#include "pwrm.h"
#include "pca10040.h"

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS ARDUINO_1_PIN

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)

OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

/*
 * The setup function. We only start the sensors here
 */
void setup(void)
{
  pinMode(ARDUINO_2_PIN, OUTPUT);
  digitalWrite(ARDUINO_2_PIN, HIGH);
  // start serial port
  Serial.begin(9600);
  Serial.println("Dallas Temperature IC Control Library Demo");

  // Start up the library
  sensors.begin();
}

/*
 * Main function, get and show the temperature
 */
void loop(void)
{ 
  // call sensors.requestTemperatures() to issue a global temperature 
  // request to all devices on the bus
  Serial.print("Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println("DONE");
  // After we got the temperatures, we can print them here.
  // We use the function ByIndex, and as an example get the temperature from the first sensor only.
  Serial.print("Temperature for the device 1 (index 0) is: ");
  Serial.println(sensors.getTempCByIndex(0));  
}
