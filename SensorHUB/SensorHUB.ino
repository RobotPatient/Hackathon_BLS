#include <Wire.h>
#include "wiring_private.h"  // pinPeripheral() function
#include "pinTools.h"

#include "Sensor.hpp"

// i2c system bus
#define W0_SCL 27  // PA22
#define W0_SDA 26  // PA23

#define W1_SCL 39  // PA13
#define W1_SDA 28  // PA12

#define W2_SCL 13  // PA17
#define W2_SDA 11  // PA16

TwiPinPair portBackbone = TwiPinPair(W0_SCL, W0_SDA);
TwiPinPair portSensorsA = TwiPinPair(W1_SCL, W1_SDA);
TwiPinPair portSensorsB = TwiPinPair(W2_SCL, W2_SDA);

#define ECG_MODULE_ADDR 0x2A
#define TEMP_PRES_MODULE_ADDR 0x68
#define ledHb 14

unsigned long lastLedFlash = 0;

bool ledLevel = false;

// SDA will be on SERCOM1.0 D11 and SCL will be on SERCOM1.1 D13

// Default Wire:
// I2C / SERCOM 0:
// PA22 / SDA on pin 11;
// PA23 / SCL on pin 12;

// On the Development Board:
// D13_SCL_PA17 /﻿D11_SDA_PA16
// D3_SCL_PA09 / D4_SDA_PA08
// SCL_PA23 / SDA_PA22 are they mixed up? Needs checking Nov 5, 2021. SFM Could be switched using jumper wires on the pads JP9/JP10

TwoWire WireBackbone(&sercom3, W0_SDA, W0_SCL);  // Main
TwoWire WireSensorA(&sercom1, W1_SDA, W1_SCL);   // Sensor A
TwoWire WireSensorB(&sercom4, W2_SDA, W2_SCL);   // Sensor B
// And of course standard Wire


void setup() {
  delay(500);
  Serial.begin(115200);
  delay(500);

  Wire.begin(0x2B);
  WireSensorA.begin();
  WireSensorB.begin();

  pinMode(ledHb, OUTPUT);
  digitalWrite(ledHb, HIGH);

  portSensorsA.setPinPeripheralAltStates();
  portSensorsB.setPinPeripheralStates();

  int channel = 1;
  // Configuration byte: 16-bit resolution, single-ended mode (channel), continuous conversion mode
  byte configByte = B10000000 | (channel << 4);

  //WireSensorA.beginTransmission(TEMP_PRES_MODULE_ADDR);
  //WireSensorA.write(configByte);
  //WireSensorA.endTransmission();
  //WireSensorB.beginTransmission(TEMP_PRES_MODULE_ADDR);
  //WireSensorB.write(configByte);
  //WireSensorB.endTransmission();
  Wire.onRequest(sendData);

  Serial.println("Ready...");
  delay(500);
  digitalWrite(ledHb, LOW);


}

Sensor mySensor(0X2A, &WireSensorA);

void loop() {

  mySensor.wireObject->requestFrom(mySensor.deviceAddress, 3);
  while (mySensor.wireObject->available()) {
    Serial.println(mySensor.wireObject->read());
  }



  digitalWrite(ledHb, HIGH);
  delay(500);
  digitalWrite(ledHb, LOW);
  delay(500);
  //readADC(&WireSensorA);
  //  sendmessage();
  //readADC(&WireSensorB);
}

void sendData() {
  //unsigned char dataToSend = 42;  // Verander dit naar de waarde die je wilt verzenden

  // Verzend de unsigned char naar de I2C-master
  Wire.write("test");

  Serial.print("Verzonden naar master: ");
  //Serial.println(dataToSend);
}
/*
  void sendmessage() {
  for(int i = 0; i <= 3; i++) {
    if (ECG[i] = 0) {
      bit[i] = 0;
      bit[i+3] = 0;
    else if (ECG[i] = 1) {
      bit[i] = 0;
      bit[i+3] = 1;
    }
    else if (ECG[i] = 2) {
      bit[i] = 1;
      bit[i+3] = 1;
    }
  }
  // Een byte maken door bitwise OR te gebruiken
  // Hier wordt aangenomen dat de bits op volgorde zijn, van hoogste naar laagste significantie.
  unsigned char resultByte = (bit0 << 7) |(bit1 << 6) | (bit2 << 5) | (bit3 << 4) | (bit4 << 3) | (bit5 << 2) | (bit6 << 1) | bit7;
  WireBackbone.write(resultByte);
  }
*/
void readADC(TwoWire *wire) {

  wire->requestFrom(ECG_MODULE_ADDR, 3);
  if (wire->available() >= 3) {
    int ECG1 = wire->read();
    int ECG2 = wire->read();
    int ECG3 = wire->read();

    // Print the ADC value for the specific channel
    Serial.println(ECG1);
    Serial.println(ECG2);
    Serial.println(ECG3);
  }

  int channel = 1;
  wire->requestFrom(TEMP_PRES_MODULE_ADDR, 3);  // Read 3 bytes from MCP3426
  if (wire->available() >= 3) {
    byte firstByte = wire->read();
    byte secondByte = wire->read();
    //byte thirdByte = wire->read();

    // Combine the received bytes to form the 18-bit ADC value
    int16_t rawValue = ((firstByte & 0x0F) << 8) | (secondByte);

    // Print the ADC value for the specific channel
    Serial.print("ADC Value (CH");
    Serial.print(channel);
    Serial.print("+): ");
    Serial.println(rawValue);
  }
}