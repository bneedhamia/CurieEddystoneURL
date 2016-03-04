# CurieEddystoneURL
Arduino 101 Sketch to Advertise an [Eddystone](https://github.com/google/eddystone) URL

This sketch is intended to be an Arduino/Genuino 101 Eddystone-compatible Sketch

The Eddystone BLE protocol is used by the [Physical Web(tm)](https://github.com/google/physical-web) project. The Physical Web project is about BLE (Bluetooth Low Energy) beacons that broadcast URLs so your users don't have to download an app just to interact with your device.

NOTE: this version doesn't support Eddystone Config mode. To change the beacon URL, you need to modify the Sketch and reload the beacon.

NOTE: this Sketch requires [my modifications to the CurieBLE library](https://github.com/bneedhamia/CurieBLEServiceData) to support beacon Advertising.

See the comments in the .ino file for more details.
