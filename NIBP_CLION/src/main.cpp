//
// Created by emiel on 25-1-2024.
//

// --------------------------------------
// i2c_scanner for SAMDxx microcontrollers with Arduino bootloader
// (e.g. Feather M0 Express by Adafruit)
//
// This sketch tests the standard 7-bit addresses
// Devices with higher bit address might not be seen properly.
//
// Changes for RobotPatient Simulators and HAN University of Applied Sciences
// Nov 5, 2021 J.A. Korten
// Scanner for 3 i2c channels:
// Escpecially for DevBoard for Ventilations / Soft Robotics actuators.
//
// Based on earlier work by Krodal and Nick Gammon
// See also: https://learn.adafruit.com/using-atsamd21-sercom-to-add-more-spi-i2c-serial-ports/creating-a-new-wire
//
// This sketch tests the standard 7-bit addresses
// Devices with higher bit address might not be seen properly.
//
//
// Changes for RobotPatient Simulators and HAN University of Applied Sciences
// Nov 5, 2021 J.A. Korten
// Scanner for 3 i2c channels:
// Escpecially for DevBoard for Ventilations / Soft Robotics actuators.
//
// This version has extra functionality to test the SimModule ECG - Jan 2024
//

#include <Wire.h>
#include "wiring_private.h"  // pinPeripheral() function
#include "pinTools.h"
#include "Actuator.h"

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

#define TEMP_MODULE_ADDR 0x68
#define ledHb 14

#define TRIGGERAMOUNT 3

void reportDevicesOn(TwoWire *wire, String label);
boolean reportDevicesWithAddressOn(TwoWire *wire, byte deviceAddress);
void printDeviceName(int address);
int readADC(TwoWire *wire, byte channel);
void requestEvent();
void sim();

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
    delay(1500);
    Serial.begin(115200);
    delay(1500);


    Wire.begin(TEMP_MODULE_ADDR);
    //Wire.begin();
    //WireBackbone.begin(T);
    WireSensorA.begin();
    WireSensorB.begin();

    pinMode(ledHb, OUTPUT);
    digitalWrite(ledHb, HIGH);

    portBackbone.setPinPeripheralStates();
    portSensorsA.setPinPeripheralAltStates();
    portSensorsB.setPinPeripheralStates();

    while (!Serial)
        ;

    Wire.onReceive(reinterpret_cast<void (*)(int)>(requestEvent));  // register event maybe for future usage

    Serial.println("Ready...");
    delay(1500);
    digitalWrite(ledHb, LOW);
}


void loop() {
    digitalWrite(ledHb, HIGH);
    //boolean result = reportDevicesWithAddressOn(&WireBackbone, 0x60, true);  // 0x10 is maybe finger position sensor.

    //reportDevicesOn(&WireBackbone, "Mainbus");
//    reportDevicesOn(&WireSensorA, "Sensors Ax");
//    reportDevicesOn(&WireSensorB, "Sensors Bx");

    //actuatorBuzz();
    //readADC(&WireSensorA, 1);

    delay(500);  // wait 5 seconds for next scan
    digitalWrite(ledHb, LOW);
    delay(500);  // wait 5 seconds for next scan
}

void requestEvent()
{
    sim();
}

void reportDevicesOn(TwoWire *wire, String label) {
    byte error, address;
    int nDevices;
    Serial.print("Scanning bus ");
    Serial.println(label);
    for (address = 1; address < 127; address++) {
        digitalWrite(ledHb, LOW);
        // The i2c_scanner uses the return value of
        // the Write.endTransmisstion to see if
        // a device did acknowledge to the address.

        //wire->beginTransmission(address);
        //error = wire->endTransmission();

        if (reportDevicesWithAddressOn(wire, address)) {
            digitalWrite(ledHb, HIGH);
            Serial.print("I2C device found at address 0x");
            if (address < 16)
                Serial.print("0");
            Serial.print(address, HEX);
            Serial.println("  !");
            printDeviceName(address);

            if (address == TEMP_MODULE_ADDR) {
                readADC(wire, 1);
            }

            nDevices++;
        } else if (error == 4) {
            Serial.print("Unknow error at address 0x");
            if (address < 16)
                Serial.print("0");
            Serial.println(address, HEX);
        }
    }
    if (nDevices == 0)
        Serial.println("No I2C devices found on bus 1\n");
    else
        Serial.println("done\n");
}

void printDeviceName(int address) {
    // Print recognized i2c device names:
    if (address == 0x25) {
        Serial.println("Sensirion SDP800-500Pa found...");
    }
    if (address == 0x26) {
        Serial.println("Sensirion SDP800-501Pa found...");
    }
    if (address == 0x29) {
        Serial.println("DLC-L01G-U2 or VL6180 found...");
    }
    if (address == 0x40) {
        Serial.println("Sensirion SDP610-500Pa found...");
    }
    if (address == 0x50) {
        Serial.println("FRAM/EEPROM found...");
    }
    if (address == 0x51) {
        Serial.println("More memory found? Could be 1M FRAM.");
    }
    if (address == 0x7C) {
        Serial.println("RESERVED...");
    }
}

boolean reportDevicesWithAddressOn(TwoWire *wire, byte deviceAddress) {
    wire->beginTransmission(deviceAddress);
    byte error = wire->endTransmission();
    boolean result = (error == 0);
    return result;
}

boolean reportDevicesWithAddressOn(TwoWire *wire, byte deviceAddress, boolean report) {
    wire->beginTransmission(deviceAddress);
    byte error = wire->endTransmission();
    boolean result = (error == 0);
    char buffer[40];
    if (report) {
        if (result) {
            sprintf(buffer, "Device was found on address 0x%02X", deviceAddress);
        } else {
            sprintf(buffer, "No device was found on address 0x%02X", deviceAddress);
        }
        Serial.println(buffer);
    }
    return result;
}


int readADC(TwoWire *wire, byte channel) {
    // Configuration byte: 18-bit resolution, single-ended mode (channel), continuous conversion mode
    byte configByte = B10000000 | (channel << 3);

    wire->beginTransmission(TEMP_MODULE_ADDR);
    wire->write(configByte);
    wire->endTransmission();

    delay(100);  // Allow some time for conversion (adjust as needed)

    wire->requestFrom(TEMP_MODULE_ADDR, 2);  // Read 3 bytes from MCP3426
    if (wire->available() >= 2) {
        byte firstByte = wire->read();
        byte secondByte = wire->read();

        // Combine the received bytes to form the 18-bit ADC value
        int16_t rawValue = ((firstByte & 0x0F) << 8) | secondByte;

        // Print the ADC value for the specific channel
        Serial.print("ADC Value (CH");
        Serial.print(channel);
        Serial.print("+): ");
        Serial.println(rawValue);
        return rawValue;
    }

    delay(1000);  // Delay before taking the next reading
}

void sim()
{
    while (readADC(&WireSensorA, 1) > 300)
    {
        actuatorTrigger(0, 200, 500);
    }

    for (int i = 0; i < TRIGGERAMOUNT; i++)
    {
        actuatorTrigger(3,50,300);
    }
}