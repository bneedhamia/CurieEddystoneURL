/*
 * Eddystone URL for Arduino/Genuino 101.
 * 
 * This is an Intel Curie Eddystone-compatible Sketch
 * (https://github.com/google/eddystone).
 * 
 * The Eddystone BLE protocol is used by the Physical Web(tm) project
 * (https://github.com/google/physical-web).
 * The Physical Web project is about BLE beacons that broadcast URLs
 * so your users don't have to download an app just to interact with
 * your device.
 * 
 * Note: this version doesn't support Eddystone Config mode.
 * To change the URL, you need to modify MY_URL and reload the Arduino.
 * 
 * Parts required:
 *   Arduino/Genuino 101.  This Sketch runs only on Curie-based devices.
 *   No other hardware is required.
 * 
 * To Use:
 * - Setup your CurieBLE libraries to support Eddystone.
 *   Doing this may be a little complicated until the Eddystone support
 *   is added to the CurieBLE library.  Until then...
 *   There are two ways to do that:
 *   1) Pull code from https://github.com/bneedhamia/corelibs-arduino101/tree/support-eddystone-url
 *   or
 *   2) Replace CurieBLE/BLEPeripheral.* and CurieBLE/keywords.txt 
 *   with a modified version that supports Eddystone Advertising packets.
 *   See https://github.com/bneedhamia/CurieBLEServiceData
 *   once that's done...
 * - Set MY_URL to the url for the beacon you wish to broadcast.
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
 * Version 1.0.4 of CurieBle doesn't support Eddystone-format
 * BLE Advertising packets, for 2 reasons:
 * - It uses the BLE code for Incomplete Service UUID list
 *   instead of Complete Service UUID list code required by Eddystone.
 * - It doesn't support Service Data, which is required by Eddystone.
 * 
 * Note: the name of the Curie BLE library changed between 1.0.4 and
 * the latest (as of March 10, 2016) version:
 * In 1.0.4, the name is CurieBle.h; in later versions, it's CurieBLE.h
 */
#include <CurieBLE.h>        // BHN-Modified Curie-specific BLE Library

/*
 * Pinout:
 *   PIN_BLINK = digital Output. The normal Arduino 101 LED.
 *     If everything is working, the LED blinks.
 *     If instead there is a startup error, the LED is solid.
 */
const int PIN_BLINK = 13;        // Pin 13 is the on-board LED

/*
 * The (unencoded) URL to put into the
 * Eddystone-URL beacon frame.
 * 
 * NOTE: the encoded length must be under 18 bytes long.
 * See initEddystoneUrlFrame().
 *
 * NOTE: This Sketch supports only lower-case URLs.
 * See initEddystoneUrlFrame().
 */
const char* MY_URL = "http://www.intel.com";

/*
 * The reported Tx Power value to put into the Eddystone-URL beacon frame.
 * 
 * From the Eddystone-URL Protocol specification
 * (https://github.com/google/eddystone/blob/master/eddystone-url/README.md#tx-power-level)
 * "the best way to determine the precise value
 * to put into this field is to measure the actual output
 * of your beacon from 1 meter away and then add 41dBm to that.
 * 41dBm is the signal loss that occurs over 1 meter."
 * 
 * I used the Nordic nRF Master Control Panel Android app
 * (https://play.google.com/store/apps/details?id=no.nordicsemi.android.mcp&hl=en)
 * to measure the beacon power at 1 meter.
 */
const int8_t TX_POWER_DBM = (-70 + 41);

/*
 * Maximum number of bytes in an Eddystone-URL frame.
 * The Eddystone-URL frame contains
 * the frame type, Tx Power, Url Prefix, and up to 17 bytes of Url.
 * (The spec isn't completely clear.  That might be 18 rather than 17.)
 */
const uint8_t MAX_URL_FRAME_LENGTH = 1 + 1 + 1 + 17;

/*
 * Eddystone-URL frame type
 */
const uint8_t FRAME_TYPE_EDDYSTONE_URL = 0x10;

/*
 * Eddystone-URL url prefix types
 * representing the starting characters of the URL.
 * 0x00 = http://www.
 * 0x01 = https://www.
 * 0x02 = http://
 * 0x03 = https://
 */
const uint8_t URL_PREFIX_HTTP_WWW_DOT = 0x00;
const uint8_t URL_PREFIX_HTTPS_WWW_DOT = 0x01;
const uint8_t URL_PREFIX_HTTP_COLON_SLASH_SLASH = 0x02;
const uint8_t URL_PREFIX_HTTPS_COLON_SLASH_SLASH = 0x03;

/*
 * eddyService = Our BLE Eddystone Service.
 * See https://developer.bluetooth.org/gatt/services/Pages/ServicesHome.aspx 
 * and https://github.com/google/eddystone/blob/master/protocol-specification.md
 * NOTE: as of this writing, Eddystone doesn't seem to be part of the Standard BLE Services list.
 * 
 */
BLEService eddyService("FEAA");

BLEPeripheral ble;              // Root of our BLE Peripheral (server) capability

/*
 * The Service Data to Advertise.
 * See initEddystoneUrlFrame().
 * 
 * urlFrame[] = the Service Data for our Eddystone-URL frame.
 * urlFrameLength = the number of bytes in urlFrame[].
 */
uint8_t urlFrame[MAX_URL_FRAME_LENGTH];
uint8_t urlFrameLength;

/*
 * If false after setup(), setup failed.
 */
bool setupSucceeded;


void setup() {  
  Serial.begin(9600);

  pinMode(PIN_BLINK, OUTPUT);
  
  setupSucceeded = false;
  digitalWrite(PIN_BLINK, HIGH);

  if (!initEddystoneUrlFrame(TX_POWER_DBM, MY_URL)) {
    return; // don't start advertising if the URL won't work.
  }
    
  /*
   * Set up Bluetooth Low Energy Advertising
   * of the Eddystone-URL frame.
   */
  ble.setAdvertisedServiceUuid(eddyService.uuid());
  ble.setAdvertisedServiceData(eddyService.uuid(), urlFrame, urlFrameLength);
  
  // Start Advertising our Eddystone URL.
  ble.begin();

  setupSucceeded = true;
  digitalWrite(PIN_BLINK, LOW);
}


unsigned long prevReportMillis = 0L;

void loop() {

  // If setup() failed, do nothing
  if (!setupSucceeded) {
    delay(1);
    return;
  }

  unsigned long now = millis();

  if ((now % 1000) < 100) {
    digitalWrite(PIN_BLINK, HIGH);
  } else {
    digitalWrite(PIN_BLINK, LOW);
  }

  if ((long) (now - (prevReportMillis + 10000L))>= 0L) {
    prevReportMillis = now;
    //report();  // uncomment this line to see the completed advertising block.

  }
}


/*
 * Prints the complete advertising block
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


/*
 * Fills urlFrame and urlFrameLength
 * based on the given Transmit power and URL.
 * 
 * txPower = the transmit power of the Arduino 101 to report.
 * See TX_POWER_DBM above.
 * 
 * url = the URL to encode.  Likely a shortened URL
 * produced by an URL shortening service such as https://goo.gl/
 * 
 * Returns true if successful; false otherwise.
 * If the encoded URL is too long, use an URL shortening service
 * before passing the (shortened) URL to setServiceData().
 * 
 * NOTE: most of the code of this function is untested.
 */
boolean initEddystoneUrlFrame(int8_t txPower, const char* url) {
  urlFrameLength = 0;

  // The frame starts with a type byte, then power byte.
  urlFrame[urlFrameLength++] = FRAME_TYPE_EDDYSTONE_URL;
  urlFrame[urlFrameLength++] = (uint8_t) txPower;

  if (url == 0 || url[0] == '\0') {
    return false;   // empty URL
  }

  const char *pNext = url;

  /*
   * the next byte of the frame is an URL prefix code.
   * See URL_PREFIX_* above.
   */
  
  if (strncmp("http", pNext, 4) != 0) {
    return false;  // doesn't start with HTTP or HTTPS.
  }
  pNext += 4;
  
  bool isHttps = false; // that is, HTTP
  if (*pNext == 's') {
    pNext++;
    isHttps = true;
  }

  if (strncmp("://", pNext, 3) != 0) {
    return false; // malformed URL
  }
  pNext += 3;

  urlFrame[urlFrameLength] = URL_PREFIX_HTTP_COLON_SLASH_SLASH;
  if (isHttps) {
    urlFrame[urlFrameLength] = URL_PREFIX_HTTPS_COLON_SLASH_SLASH;
  }

  if (strncmp("www.", pNext, 4) == 0) {
    pNext += 4;

    urlFrame[urlFrameLength] = URL_PREFIX_HTTP_WWW_DOT;
    if (isHttps) {
      urlFrame[urlFrameLength] = URL_PREFIX_HTTPS_WWW_DOT;
    }
  }
  
  urlFrameLength++;

  // Encode the URL.

  while (urlFrameLength < MAX_URL_FRAME_LENGTH && *pNext != '\0') {
    if (strncmp(".com/", pNext, 5) == 0) {
      pNext += 5;
      urlFrame[urlFrameLength++] = 0x00;
    } else if (strncmp(".org/", pNext, 5) == 0) {
      pNext += 5;
      urlFrame[urlFrameLength++] = 0x01;
    } else if (strncmp(".edu/", pNext, 5) == 0) {
      pNext += 5;
      urlFrame[urlFrameLength++] = 0x02;
    } else if (strncmp(".net/", pNext, 5) == 0) {
      pNext += 5;
      urlFrame[urlFrameLength++] = 0x03;
    } else if (strncmp(".info/", pNext, 6) == 0) {
      pNext += 6;
      urlFrame[urlFrameLength++] = 0x04;
    } else if (strncmp(".biz/", pNext, 5) == 0) {
      pNext += 5;
      urlFrame[urlFrameLength++] = 0x05;
    } else if (strncmp(".gov/", pNext, 5) == 0) {
      pNext += 5;
      urlFrame[urlFrameLength++] = 0x06;
    } else if (strncmp(".com", pNext, 4) == 0) {
      pNext += 4;
      urlFrame[urlFrameLength++] = 0x07;
    } else if (strncmp(".org", pNext, 4) == 0) {
      pNext += 4;
      urlFrame[urlFrameLength++] = 0x08;
    } else if (strncmp(".edu", pNext, 4) == 0) {
      pNext += 4;
      urlFrame[urlFrameLength++] = 0x09;
    } else if (strncmp(".net", pNext, 4) == 0) {
      pNext += 4;
      urlFrame[urlFrameLength++] = 0x0A;
    } else if (strncmp(".info", pNext, 5) == 0) {
      pNext += 5;
      urlFrame[urlFrameLength++] = 0x0B;
    } else if (strncmp(".biz", pNext, 4) == 0) {
      pNext += 4;
      urlFrame[urlFrameLength++] = 0x0C;
    } else if (strncmp(".gov", pNext, 4) == 0) {
      pNext += 4;
      urlFrame[urlFrameLength++] = 0x0D;
    } else {
      // It's not special.  Just copy the character
      urlFrame[urlFrameLength++] = (uint8_t) *pNext++;
    }
  }

  return true;
}

