# SimpleEVSE-WiFi
[![Donate](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://www.paypal.com/pools/c/85c7xRbeay)

SimpleEVSE-WiFi brings WiFi functionality to your SimpleEVSE WB to control your Charging Station very easy. It uses an ESP8266 to communicate with SimpleEVSE WB via ModBus (UART) and offers a web interface to control it. Optional there is a possibility to connect an impulse meter via S0 and an RC522 RFID reader to detect valid RFID tags.

If you want to support this project, I would be very happy about a [donation](https://www.paypal.com/pools/c/85c7xRbeay).

## Main Features

* WiFi functionality (as an access point or as a WiFi client)
* Activate and deactivate the Simple EVSE WB via a web interface, RFID tags or a button
* [HTTP-API](#http-api) to let other devices control SimpleEVSE-WiFi
* Setting the charging current via the web interface
* Displays the current charging power in kW (by impulses of the S0 counter, optional)
* User management (RFID tags, optional)
* Log of the last charging processes with output of the charged energy quantity, charging time and costs of the charging process (optionally which RFID tag the EVSE was enabled by and the user assigned to it)
* Rudimentary settings (WiFi settings, password for web interface, maximum charging current, etc.)
* Output of all important parameters of the SimpleEVSE WB (Modbus)

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
* A complete Wallbox based on EVSE Wallbox [link](http://evracing.cz/simple-evse-wallbox/) (min. Software revision 8 (2017-10-31) is needed - read the manual there for updating your EVSE software)
* An ESP8266 module or a development board like **WeMos D1 mini** or **NodeMcu 1.0**
* (optional) An electricity meter with S0 interface
* (optional) A MFRC522 RFID Module (PN532 and Wiegand based RFID reader modules are not supported at this time)
* (optional) n quantity of Mifare Classic 1KB (recommended due to available code base) PICCs (RFID Tags) equivalent to User Number

#### Wiring (WeMos D1 mini/ NodeMcu)
SimpleEVSE-WiFi supports not only the control of the EVSE WB but also the use of a simple button or an RC522 RFID card reader to activate the charging process. With an optional electricity meter, the power requirements of the individual charging processes can be output.

##### Mandatory
This wiring is mandatory and absolutely needed to run SimpleEVSE-WiFi

ESP8266-Pin | ESP8266-GPIO | EVSE WB
----------- | ----------- | -----------
D1 | GPIO5 | TX
D2 | GPIO4 | RX
GND |  | GND

##### Button (optional)
ESP8266-Pin | ESP8266-GPIO | Button
----------- | ----------- | -----------
D4 | GPIO2 | Pin 1
GND | | Pin 2

##### Electicity Meter (optional)*
ESP8266-Pin | ESP8266-GPIO | electricity meter
----------- | ----------- | -----------
D3 | GPIO0 | S0+
GND | | S0-

##### RC522 RFID-Reader (optional)
ESP8266-Pin | ESP8266-GPIO | RC522
----------- | ----------- | -----------
D5 | GPIO14 | SCK
D6 | GPIO12 | MISO
D7 | GPIO13 | MOSI
D8 | GPIO15 | SDA
GND |  | GND
3.3V |  | 3.3V

Be sure to use a suitable power supply for ESP. At least 200mA is recommended!

*When you use an electricity meter be sure the S0 interface switches to GND, don't use 3.3V or 5V!

#### Preparation of EVSE Wallbox
To use SimpleEVSE-WiFi, the Modbus functionallity of EVSE Wallbox is needed! By default, Modbus functionality is disabled. To activate it, pull AN input of the EVSE Wallbox board to GND while booting for at least 5 times within 3 seconds. Modbus register 2001 will be set to 1 (Modbus is active). Attention: That change will not be saved! To save the settings, you have to give a R/W operation at a register >=2000. The easiest way to do this is to activate and deactivate EVSE through the WebUI in the "EVSE Control" page.

### Software

**Attention:** make sure that the EVSE WB is not connected to the ESP during the flash process via USB. It could cause overheating of the ESP.

#### Use Compiled Binaries
Compiled firmware binary and flasher tool for Windows PCs are available in directory **/bin**. On Windows you can use **"flash.bat"**, it will ask you which COM port that ESP is connected and then flashes it. You can use any flashing tool and do the flashing manually if you don't want to use the flash.bat.

#### Building From Source
Please install Arduino IDE if you didn't already, then add ESP8266 Core on top of it. Additional Library download links are listed below:

* [Arduino IDE](http://www.arduino.cc) - The development IDE
* [ESP8266 Core for Arduino IDE](https://github.com/esp8266/Arduino) - ESP8266 Core
* [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer) - Asyncrone Web Server with WebSocket Plug-in
* [ESPAsyncTCP](https://github.com/me-no-dev/ESPAsyncTCP) - Mandatory for ESPAsyncWebServer
* [ESPAsyncUDP](https://github.com/me-no-dev/ESPAsyncUDP) - Mandatory for ESPAsyncWebServer
* [ModbusMaster](https://github.com/CurtRod/ModbusMaster) - ModbusMaster Library for communicating with EVSE (**NOTE: Use my Fork of ModbusMaster! Otherwise the wdt reset will be triggered in case of timeout error**)
* [SoftwareSerial](https://github.com/plerup/espsoftwareserial) SoftwareSerial to communicate with EVSE (**NOTE: Use the github version! There is a buxfix for esp8266 v2.4.0 and later**)
* [MFRC522](https://github.com/miguelbalboa/rfid) - MFRC522 RFID Hardware Library for Arduino IDE
* [ArduinoJson](https://github.com/bblanchon/ArduinoJson) - JSON Library for Arduino IDE
* [NTPClientLib](https://github.com/gmag11/NtpClient/) - NTP Client Library for Arduino IDE
* [TimeLib](https://github.com/PaulStoffregen/Time) - Mandatory for NTP Client Library
* [ESP8266Ping](https://github.com/dancol90/ESP8266Ping) - Needed for checking online Status

You also need to upload web files to your ESP with ESP8266FS Uploader.

* [ESP8266FS Uploader](https://github.com/esp8266/arduino-esp8266fs-plugin) - Arduino ESP8266 filesystem uploader

Unlisted libraries are part of [ESP8266](https://github.com/esp8266/Arduino) Core for Arduino IDE, so you don't need to download them, but check that at least you have v2.4.0 or above installed.

## First boot
When SimpleEVSE-WiFi starts for the first time it sets up a WiFi access point called 'evse-wifi'. You can connect without a password. To connect, open http://192.168.4.1 in your browser. The initial password is 'admin'. You should first check the Settings to bring the ESP in Client mode and connect it to your local WiFi network. The ESP will be restarted afterwards. If it doesn't restart, press the 'RST' button once. Sometimes the ESP must first be manually reset (this only has to happen after flashing a new firmware).

## Support this Project
The development of SimpleEVSE-WiFi is very time consuming. If you want to support this project, I would be very happy about a [donation](https://www.paypal.com/pools/c/85c7xRbeay).

## HTTP API
Since version 0.2.0 there is an HTTP API implemented to let other devices control your EVSE WiFi. The API gives you the following possibilities of setting and fetching information.

### getParameters()
gives you the following information of the EVSE WB in json:

Parameter | Description
--------- | -----------
vehicleState | Vehicle state (ready / detected / charging)
evseState | EVSE State (active/not active)
actualCurrent | Actual configured current in A (e.g. 20A)
actualPower | actual power consumption (when S0 meter is used)
duration | charging duration in milliseconds
energy | charged energy of the current charging process in kWh

#### Example
`GET http://192.168.4.1/getParameters`

> returns JSON like this:

```json
{
  "type": "parameters",
  "list": [{
    "vehicleState": 2,
    "evseState": false,
    "actualCurrent": 32,
    "actualPower": 5.79,
    "duration": 1821561,
    "energy": "9.52"
  }]
}
```

### getLog()
returns the following information about the last log entries

Parameter | Description
--------- | -----------
uid | The UID of the RFID tag that was used to activate EVSE
username | The username belongs to the UID
timestamp | Timestamp in seconds since Jan 01 1970 (Unix timestamp)
duration | Duration of the charging process in milliseconds
energy | Charged energy of the charging process in kWh
price | Defined price per kWh in cent

#### Example
`GET http://192.168.4.1/getLog`

> returns JSON like this:

```json
{
  "type": "latestlog",
  "list": [{
    "uid": "ABCD1234",
    "username": "GUI",
    "timestamp": 1523295915,
    "duration": 7504266,
    "energy": "10.32",
    "price": 21
  }, {
    "uid": "-",
    "username": "GUI",
    "timestamp": 1523568920,
    "duration": 1152251,
    "energy": "2.17",
    "price": 23
  }]
}
```

### setCurrent()
using setCurrent() will set the current to the given value (e.g. 18A)

Parameter | Description
--------- | -----------
current | Current to set in EVSE WiFi

#### Example

`GET http://192.168.4.1/setCurrent?current=8`

> Sets the charging current  to 8A and returns

```text
S0_set current to 8A
```
In cases of Error, the answer would be

Answer | Description
--------- | -----------
E0_could not set current - internal error | Internal error occured (unspecified)
E1_could not set current - give a value between *x* and *y*  | Wrong value was given
E2_could not set current - wrong parameter | Wrong parameter was given

### setStatus()
will activate/deactivate EVSE WB

Parameter | Description
--------- | -----------
active | can be true (activate EVSE) or false (deactivate EVSE)

#### Example

`GET http://192.168.4.1/setStatus?active=true`

> Activates EVSE and returns

```text
S0_EVSE successfully activated
```
In cases of Error, the answer would be

Answer | Description
--------- | -----------
E0_could not activate EVSE - internal error | Internal error occured (unspecified)
E0_could not deactivate EVSE - internal error | Internal error occured (unspecified)
E1_could not process - give a valid value (true/false) | Wrong value was given
E2_could not process - wrong parameter | Wrong parameter was given



