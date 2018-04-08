# SimpleEVSE-WiFi

SimpleEVSE-WiFi brings WiFi functionality to your SimpleEVSE WB to control your Charging Station very easy. It uses an ESP8266 for communicate with SimpleEVSE via ModBus and offers a Webinterface to control it. Optional there is a possibility to connect an impulse meter via S0 and an RC522 RFID reader to detect valid RFID tags.

## Preview

Control the Current your EVSE communicates to the vehicle. This page also shows additional data like metered energy and the charging time.

![Showcase 1](https://raw.githubusercontent.com/CurtRod/SimpleEVSE-WiFi/master/demo/1.png)
![Showcase 2](https://raw.githubusercontent.com/CurtRod/SimpleEVSE-WiFi/master/demo/2.png)

The log page shows the last charging activities.

![Showcase 3](https://raw.githubusercontent.com/CurtRod/SimpleEVSE-WiFi/master/demo/3.png)

At the users page you can define valid RFID tags that can unlock the EVSE.

![Showcase 4](https://raw.githubusercontent.com/CurtRod/SimpleEVSE-WiFi/master/demo/4.png)


## What You Will Need

### Hardware
* A complete Wallbox based on SimpleEVSE WB [link](https://www.elektrofahrzeug-umbau.de/shop/wallbox/)
* An ESP8266 module or a development board like **WeMos D1 mini** or **NodeMcu 1.0**
* (optional) An electricity meter with S0 interface
* (optional) A MFRC522 RFID PCD Module or PN532 NFC Reader Module or Wiegand based RFID reader
* (optional) n quantity of Mifare Classic 1KB (recommended due to available code base) PICCs (RFID Tags) equivalent to User Number

#### Wiring (WeMos D1 mini/ NodeMcu)

* D0	->	Button (optional)
* D1	->	TX EVSE
* D2	->	RX EVSE
* D3	->	Electricity Meter S0 (optional)*
* D5	->	SCK RC522 (optional)
* D6	->	MISO RC522 (optional)
* D7	->	MOSI RC522 (optional)
* D8	->	SDA RC522 (optional)

*When you use an electricity meter be sure the S0 interface switches to GND, don't use 3.3V oder 5V!

### Software

#### Use Compiled Binaries
Compiled firmware binary and flasher tool for Windows PCs are available in directory **/bin**. On Windows you can use **"flash.bat"**, it will ask you which COM port that ESP is connected and then flashes it. You can use any flashing tool and do the flashing manually. The flashing process itself has been described at numerous places on Internet.

#### Building From Source
Please install Arduino IDE if you didn't already, then add ESP8266 Core on top of it. Additional Library download links are listed below:

* [Arduino IDE](http://www.arduino.cc) - The development IDE
* [ESP8266 Core for Arduino IDE](https://github.com/esp8266/Arduino) - ESP8266 Core
* [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer) - Asyncrone Web Server with WebSocket Plug-in
* [ESPAsyncTCP](https://github.com/me-no-dev/ESPAsyncTCP) - Mandatory for ESPAsyncWebServer
* [ModbusMaster](https://github.com/CurtRod/ModbusMaster) - ModbusMaster Library for communicating with EVSE (**NOTE: Use my Fork of ModbusMaster! Otherwise the wdt reset will be triggered in case of timeout error**)
* [SoftwareSerial](https://github.com/plerup/espsoftwareserial) SoftwareSerial to communicate with EVSE (**NOTE: Use the github version! There is a buxfix for esp8266 v2.4.0 and later**)
* [MFRC522](https://github.com/miguelbalboa/rfid) - MFRC522 RFID Hardware Library for Arduino IDE
* [ArduinoJson](https://github.com/bblanchon/ArduinoJson) - JSON Library for Arduino IDE
* [NTPClientLib](https://github.com/gmag11/NtpClient/) - NTP Client Library for Arduino IDE
* [TimeLib](https://github.com/PaulStoffregen/Time) - Mandatory for NTP Client Library

You also need to upload web files to your ESP with ESP8266FS Uploader.

* [ESP8266FS Uploader](https://github.com/esp8266/arduino-esp8266fs-plugin) - Arduino ESP8266 filesystem uploader

Unlisted libraries are part of ESP8266 Core for Arduino IDE, so you don't need to download them.

