/*
 * Eddystone URL for Arduino/Genuino 101.
 * 
 * This sketch is an Intel Curie Eddystone-compatible Sketch
 * (https://github.com/google/eddystone).
 * 
 * The Eddystone BLE protocol is used by the Physical Web(tm) project
 * (https://github.com/google/physical-web).
 * The Physical Web project is about BLE beacons that broadcast URLs
 * so your users don't have to download an app just to interact with
 * your device.
 * 
 * Note: this version doesn't support Eddystone Config mode.
 * To change the URL, you need to modify the Sketch and reload the beacon.
 * 
 * Parts required:
 *   Arduino/Genuino 101.  This Sketch runs only on Curie-based devices.
 *   No other hardware is required.
 * 
 * To Use:
 * - Replace CurieBLE/BLEPeripheral.* and CurieBLE/keywords.txt 
 *   with a modified version that supports Eddystone Advertising packets.
 *   See (TODO ADD GITHUB CURIEBLE CHANGES URL).
 * - Modify the URL in this sketch, keeping in mind
 *   the Eddystone encoded URL limit of 18 bytes.
 * - Download this Sketch to an Arduino/Genuino 101.
 * - Download a Physical Web app to your phone.
 *   For example, Google's Physical Web Android app is at
 *   https://play.google.com/store/apps/details?id=physical_web.org.physicalweb
 *   
 * Copyright (c) 2016 Bradford Needham, North Plains, Oregon, USA
 * @bneedhamia, https://www.needhamia.com
 * Licensed under the Apache 2.0 License, a copy of which
 * should have been included with this software.
 */

/*
 * Note: as of this writing, the documentation of the CurieBLE library
 * is the library source, at (on Linux)
 * ~/.arduino15/packages/Intel/hardware/arc32/1.0.4/libraries/CurieBle/src
 * 
 * NOTE: Version 1.0.4 of CurieBle doesn't support Eddystone-format
 * BLE Advertising packets, for 2 reasons:
 * - It uses the BLE code for Incomplete Service UUID list
 *   instead of Complete Service UUID list required by Eddystone.
 * - It doesn't support Service Data, which is required by Eddystone.
 */
#include <CurieBle.h>        // BHN-Modified Curie-specific BLE Library

/*
 * Pinout:
 *   PIN_BLINK = digital Output. The normal Arduino 101 LED.
 *     TODO explain what it indicates.  TODO make it blink regularly to show it's advertising.
 */
const int PIN_BLINK = 13;        // Pin 13 is the on-board LED

BLEPeripheral ble;              // Root of our BLE Peripheral (server) capability

/*
 * eddyService = Our BLE Eddystone Service.
 * See https://developer.bluetooth.org/gatt/services/Pages/ServicesHome.aspx 
 * and https://github.com/google/eddystone/blob/master/protocol-specification.md
 * NOTE: as of this writing, Eddystone doesn't seem to be part of the Standard BLE Services list.
 * 
 */
BLEService eddyService("FEAA");

/*
 * Hard-coded Eddystone URL frame for our device.
 * See https://github.com/google/eddystone/blob/master/eddystone-url/README.md
 */
const int MAX_SVC_DATA_LENGTH = 18;  // Maximum number of bytes in the UriData attribute value.
byte DEFAULT_SVC_DATA[MAX_SVC_DATA_LENGTH + 1] = {
  "\x10\xEB\x01google\x07"  // Eddystone URL frame, power, URL scheme, url, URL suffix.
  };
unsigned short DEFAULT_SVC_DATA_LENGTH = 10;    // number of value bytes in DEFAULT_SVC_DATA. No null terminator counted

void setup() {
  Serial.begin(9600);

  // Set up the I/O pins
  pinMode(PIN_BLINK, OUTPUT);
  digitalWrite(PIN_BLINK, LOW);

  //TODO build the Eddystone URL Service Data: with power and url from EEPROM.
  /*
   * Set up Bluetooth Low Energy Advertising
   * of the Eddystone URL frame.
   */
  ble.setAdvertisedServiceUuid(eddyService.uuid());
  ble.setAdvertisedServiceData(eddyService.uuid(), DEFAULT_SVC_DATA, DEFAULT_SVC_DATA_LENGTH);
  
  // Start Advertising our Eddystone URL.
  ble.begin();
}

unsigned long prevReportMillis = 0L;

void loop() {
  digitalWrite(PIN_BLINK, HIGH);
  delay(33);
  digitalWrite(PIN_BLINK, LOW);
  delay(33);

  unsigned long now = millis();
  if ((long) (now - (prevReportMillis + 10000L))>= 0L) {
    prevReportMillis = now;
    //report();  // uncomment this line to see the completed advertising block.

  }
}

/*
 * Prints the complete adversiting block
 * that was assembled by CurieBLE.
 * For debugging.
 */
void report() {
  uint8_t len = ble.getAdvertisingLength();
  uint8_t* pAdv = ble.getAdvertising();

  Serial.print("Advertising block length: ");
  Serial.println(len);
  for (int i = 0; i < len; ++i) {
    Serial.print("0x");
    Serial.println(pAdv[i], HEX);
  }
}

