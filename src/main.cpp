/*
  Copyright (c) 2018 CurtRod

  Released to Public Domain

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/
#include <Arduino.h>

#ifdef ESP8266
#include <ESP8266WiFi.h>              // Whole thing is about using Wi-Fi networks
#include <ESP8266mDNS.h>              // Zero-config Library (Bonjour, Avahi)
#include <ESPAsyncTCP.h>              // Async TCP Library is mandatory for Async Web Server
#include <FS.h>                       // SPIFFS Library for storing web files to serve to web browsers
#include <WiFiUdp.h>                  // Library for manipulating UDP packets which is used by NTP Client to get Timestamps
#include <SoftwareSerial.h>           // Using GPIOs for Serial Modbus communication

#else
#include <WiFi.h>
#include <ESPmDNS.h>
#include <SPIFFS.h>
#include <HardwareSerial.h>
#include "Update.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "esp_wifi.h"
#include "oled.h"

#include <TimeLib.h>                  // Library for converting epochtime to a date
#include <SPI.h>                      // SPI protocol
#include <MFRC522.h>                  // Library for Mifare RC522 Devices
#include <ArduinoJson.h>              // JSON Library for Encoding and Parsing Json object to send browser
#include <ESPAsyncWebServer.h>        // Async Web Server with built-in WebSocket Plug-in
#include <SPIFFSEditor.h>             // This creates a web page on server which can be used to edit text based files

#include <ModbusMaster.h>
#include <ModbusIP_ESP8266.h>

#include <string.h>
#include "proto.h"
#include "ntp.h"
#include "websrc.h"
#include "config.h"
#include "templates.h"
#include "rfid.h"

uint8_t sw_min = 0; //Firmware Minor Version
uint8_t sw_rev = 4; //Firmware Revision

#ifdef ESP8266
uint8_t sw_maj = 1; //Firmware Major Version
String swVersion = String(sw_maj) + "." + String(sw_min) + "." + String(sw_rev);
#else
uint8_t sw_maj = 2; //Firmware Major Version
String swVersion = String(sw_maj) + "." + String(sw_min) + "." + String(sw_rev);

//////////////////////////////////////////////////////////////////////////////////////////
///////       Variables For Whole Scope
//////////////////////////////////////////////////////////////////////////////////////////
//EVSE Variables
unsigned long millisStartCharging = 0;
unsigned long millisStopCharging = 0;
bool manualStop = false;
uint8_t currentToSet = 6;
uint8_t evseStatus = 0;
bool evseSessionTimeOut = false;
bool evseActive = false;
bool vehicleCharging = false;
int buttonState = HIGH;
int prevButtonState = HIGH;
AsyncWebParameter* awp;
AsyncWebParameter* awp2;
const char * initLog = "{\"type\":\"latestlog\",\"list\":[]}";
bool sliderStatus = true;
uint8_t evseErrorCount = 0;
bool doCpInterruptCp = false;

#ifndef ESP8266
unsigned long millisInterruptCp = 0;
bool rseActive = false;
uint8_t currentBeforeRse = 0;
#endif

//RFID
bool showLedRfidDecline = false;
bool showLedRfidGrant = false;
unsigned long millisRfidLedAction = 0;
unsigned long millisRfidReset = 0;

//Metering
float meterReading = 0.0;
float meteredKWh = 0.0;
float currentKW = 0.0;

//Metering S0
uint8_t meterTimeout = 10; //sec
volatile unsigned long numberOfMeterImps = 0;
unsigned long meterImpMillis = 0;
unsigned long previousMeterMillis = 0;
volatile uint8_t meterInterrupt = 0;

//Metering Modbus
unsigned long millisUpdateMMeter = 0;
unsigned long millisUpdateSMeter = 0;
bool mMeterTypeSDM120 = false;
bool mMeterTypeSDM630 = false;
float startTotal;
float currentP1 = 0.0;
float currentP2 = 0.0;
float currentP3 = 0.0;
float voltageP1 = 0.0;
float voltageP2 = 0.0;
float voltageP3 = 0.0;

//objects and instances
#ifdef ESP8266
SoftwareSerial SoftSer(D1, D2); //SoftwareSerial object (RX, TX)
#else
HardwareSerial SoftSer(2);
//SoftwareSerial SoftSer(22, 21); //SoftwareSerial object (RX, TX)
//oLED
unsigned long millisUpdateOled = 0;
U8G2_SSD1327_WS_128X128_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 12, /* dc=*/ 13, /* reset=*/ 33);
EvseWiFiOled oled;
#endif

ModbusMaster evseNode;
ModbusMaster meterNode;
ModbusIP modbusTCPServerNode;  //ModbusIP object
AsyncWebServer server(80);    // Create AsyncWebServer instance on port "80"
AsyncWebSocket ws("/ws");     // Create WebSocket instance on URL "/ws"
NtpClient ntp;
EvseWiFiConfig config = EvseWiFiConfig();
EvseWiFiRfid rfid;


unsigned long lastModbusAction = 0;
unsigned long evseQueryTimeOut = 0;
unsigned long buttonTimer = 0;

//Loop
unsigned long currentMillis = 0;
unsigned long previousMillis = 0;
unsigned long previousLoopMillis = 0;
unsigned long previousLedAction = 0;
unsigned long reconnectTimer = 0;
uint16_t toChangeLedOnTime = 0;
uint16_t toChangeLedOffTime = 0;
uint16_t ledOnTime = 100;
uint16_t ledOffTime = 4000;
bool wifiInterrupted = false;
bool ledStatus = false;
bool toSetEVSEcurrent = false;
bool toActivateEVSE = false;
bool toDeactivateEVSE = false;
bool toSendStatus = false;
bool toReboot = false;
bool updateRunning = false;
bool fsWorking = false;

//EVSE Modbus Registers
uint16_t evseAmpsConfig;     //Register 1000
uint16_t evseAmpsOutput;     //Register 1001
uint16_t evseVehicleState;  //Register 1002
uint16_t evseAmpsPP;         //Register 1003
uint16_t evseTurnOff;        //Register 1004
uint16_t evseFirmware;       //Register 1005
uint16_t evseEvseState;          //Register 1006
uint16_t evseRcdStatus;          //Register 1007
uint16_t evseAmpsAfterboot; 

//Settings
bool useRFID = false;
bool dontUseWsAuthentication = false;
bool dontUseLED = false;
bool resetCurrentAfterCharge = false;
bool inAPMode = false;
bool inFallbackMode = false;
bool isWifiConnected = false;
String lastUsername = "";
String lastUID = "";
char * deviceHostname = NULL;
uint8_t maxCurrent = 0;

//Others
String msg = ""; //WS communication

//////////////////////////////////////////////////////////////////////////////////////////
///////       Auxiliary Functions
//////////////////////////////////////////////////////////////////////////////////////////

#ifndef ESP8266
void ICACHE_FLASH_ATTR handleRse() {
  if (rseActive) { //RSE goes activated
    toSetEVSEcurrent = true;
    currentBeforeRse = evseAmpsConfig;
    currentToSet = int(float(evseAmpsConfig) / 100.0 * float(config.getEvseRseValue(0)));
    if (currentToSet > 0 && currentToSet < 6) currentToSet = 6;
    if (config.getEvseRseValue(0) == 0) currentToSet = 0;
    sliderStatus = false;
    if (config.getSystemDebug()) Serial.print("[ SYSTEM ] RSE Interrupted! Setting current to ");
    if (config.getSystemDebug()) Serial.println(currentToSet);
  }
  else { //RSE goes deactivated
    toSetEVSEcurrent = true;
    currentToSet = currentBeforeRse;
    if (!config.getEvseRemote(0)) sliderStatus = true;
    if (config.getSystemDebug()) Serial.print("[ SYSTEM ] RSE Released! Setting current back to ");
    if (config.getSystemDebug()) Serial.println(currentToSet);
  }
}
#endif

uint16_t get16bitOfFloat32 (float float_number, uint8_t offset){
  union {
    float f_number;
    uint16_t uint16_arr[2];
  } union_for_conv;  
  union_for_conv.f_number = float_number;
  uint16_t ret = union_for_conv.uint16_arr[offset];
  return ret;
}

void ICACHE_FLASH_ATTR handleLed() {
  if (showLedRfidGrant && millis() < millisRfidLedAction) {
    digitalWrite(config.getEvseLedPin(0), HIGH);
    return;
  }
  else if (showLedRfidGrant && millis() >= millisRfidLedAction) {
    showLedRfidGrant = false;
    digitalWrite(config.getEvseLedPin(0), LOW);
    return;
  }
  else if (showLedRfidDecline && millis() >= millisRfidLedAction) {
    showLedRfidDecline = false;
    changeLedTimes(100, 10000);
    return;
  }

  if (currentMillis >= previousLedAction && config.getEvseLedConfig(0) != 1) {
    if (ledStatus == false) {
      if (currentMillis >= previousLedAction + ledOffTime) {
        digitalWrite(config.getEvseLedPin(0), HIGH);
        ledStatus = true;
        previousLedAction = currentMillis;
      }
    }
    else {
      if (currentMillis >= previousLedAction + ledOnTime) {
        digitalWrite(config.getEvseLedPin(0), LOW);
        ledStatus = false;
        previousLedAction = currentMillis;
      }
    }
  }
}

void ICACHE_FLASH_ATTR changeLedStatus() {
  if (ledOnTime != toChangeLedOnTime) {
    ledOnTime = toChangeLedOnTime;
  }
  if (ledOffTime != toChangeLedOffTime) {
    ledOffTime = toChangeLedOffTime;
  }
}

void ICACHE_FLASH_ATTR changeLedTimes(uint16_t onTime, uint16_t offTime) {
  if (ledOnTime != onTime) {
    toChangeLedOnTime = onTime;
  }
  if (ledOffTime != offTime) {
    toChangeLedOffTime = offTime;
  }
}

String ICACHE_FLASH_ATTR printIP(IPAddress address) {
  return (String)address[0] + "." + (String)address[1] + "." + (String)address[2] + "." + (String)address[3];
}

#ifndef ESP8266
String ICACHE_FLASH_ATTR printSubnet(uint8_t mask) {
  String ret = "";
  while (mask >= 8) {
    if (mask >= 8) {
      ret += "255.";
      mask -= 8;
    }
  }
  if (mask == 0) {
    ret += "0";
  }
  else {
    int lastOct = 8 * mask;
    ret += (String)lastOct;
  }
  if (ret.substring(ret.length() - 1) == ".") ret.remove(ret.length() - 1);
  return ret;
}
#endif

void ICACHE_FLASH_ATTR parseBytes(const char* str, char sep, byte* bytes, int maxBytes, int base) {
  for (int i = 0; i < maxBytes; i++) {
    bytes[i] = strtoul(str, NULL, base);
    str = strchr(str, sep);
    if (str == NULL || *str == '\0') {
      break;
    }
    str++;
  }
}

void ICACHE_RAM_ATTR handleMeterInt() {  //interrupt routine for metering
  if (meterImpMillis < millis()) {
    meterImpMillis = millis() + (config.getMeterImpLen(0) + 10);
    meterInterrupt ++;
    numberOfMeterImps ++;
  }
}

void ICACHE_FLASH_ATTR updateS0MeterData() {
  if (vehicleCharging) {
    currentKW = 3600.0 / float(meterImpMillis - previousMeterMillis) / float(config.getMeterImpKwh(0) / 1000.0) * (float)config.getMeterFactor(0) ;  //Calculating kW
    previousMeterMillis = meterImpMillis;
    meteredKWh = float(numberOfMeterImps) / float(config.getMeterImpKwh(0) / 1000.0) / 1000.0 * float(config.getMeterFactor(0));
  }
  meterInterrupt = 0;
}

void ICACHE_FLASH_ATTR updateMMeterData() {
  if (config.mMeterTypeSDM120 == true) {
    currentKW = readMeter(0x000C) / 1000.0;
    meterReading = readMeter(0x0156);
  }
  else if (config.mMeterTypeSDM630 == true) {
    currentKW = readMeter(0x0034) / 1000.0;
    meterReading = readMeter(0x0156);
  }
  if (meterReading != 0.0 &&
      vehicleCharging == true) {
    meteredKWh = meterReading - startTotal;
  }
  if (startTotal == 0) {
    meteredKWh = 0.0;
  }
  updateSDMMeterCurrent();
  millisUpdateMMeter = millis() + 5000;
}

void ICACHE_FLASH_ATTR updateSDMMeterCurrent() {
  const int regsToRead = 12;
  uint8_t result;
  uint16_t iaRes[regsToRead];
  meterNode.clearTransmitBuffer();
  meterNode.clearResponseBuffer();
  delay(50);
  result = meterNode.readInputRegisters(0x0000, regsToRead); // read 6 registers starting at 0x0000

  if (result != 0) {
    Serial.print("[ ModBus ] Error ");
    Serial.print(result, HEX);
    Serial.println(" occured while getting current Meter Data");
    if (config.getEvseLedConfig(0) == 3) changeLedTimes(300, 300);
    return;
  }

  for (int i = 0; i < regsToRead; i++) {
    iaRes[i] = meterNode.getResponseBuffer(i);
  
  }
  ((uint16_t*)&voltageP1)[1]= iaRes[0];
  ((uint16_t*)&voltageP1)[0]= iaRes[1];
  ((uint16_t*)&voltageP2)[1]= iaRes[2];
  ((uint16_t*)&voltageP2)[0]= iaRes[3];
  ((uint16_t*)&voltageP3)[1]= iaRes[4];
  ((uint16_t*)&voltageP3)[0]= iaRes[5];

  ((uint16_t*)&currentP1)[1]= iaRes[6];
  ((uint16_t*)&currentP1)[0]= iaRes[7];
  ((uint16_t*)&currentP2)[1]= iaRes[8];
  ((uint16_t*)&currentP2)[0]= iaRes[9];
  ((uint16_t*)&currentP3)[1]= iaRes[10];
  ((uint16_t*)&currentP3)[0]= iaRes[11];
}

unsigned long ICACHE_FLASH_ATTR getChargingTime() {
  unsigned long iTime;
  if (vehicleCharging == true) {
    iTime = millis() - millisStartCharging;
  }
  else {
    iTime = millisStopCharging - millisStartCharging;
  }
  return iTime;
}

bool ICACHE_FLASH_ATTR resetUserData() {

  //SoftSer.end();
  //SPIFFS.begin();
  #ifdef ESP8266
  Dir userdir = SPIFFS.openDir("/P/");
  while(userdir.next()){
    Serial.println(userdir.fileName());
    SPIFFS.remove(userdir.fileName());
  }
  #else
  File userdir = SPIFFS.open("/P/");
  while(userdir.openNextFile()){
    Serial.println(userdir.name());
    SPIFFS.remove(userdir.name());
  }
  #endif
  //SPIFFS.end();
  delay(10);
  //SoftSer.begin(9600);
  return true;
}

bool ICACHE_FLASH_ATTR factoryReset() {
  //SoftSer.end();
  //SPIFFS.begin();
  if (config.getSystemDebug()) Serial.println("[ SYSTEM ] Factory Reset...");
  SPIFFS.remove("/config.json");
  initLogFile();
  if (resetUserData()) {
    if (config.getSystemDebug()) Serial.println("[ SYSTEM ] ...successfully done - going to reboot");
  }
  toReboot = true;
  //SPIFFS.end();
  delay(100);
  //SoftSer.begin(9600);
  return true;
}

bool ICACHE_FLASH_ATTR reconnectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return true;
  WiFi.disconnect();
  delay(100);
  WiFi.mode(WIFI_STA);
  WiFi.begin(config.getWifiSsid(), config.getWifiPass(), 0);
  if (config.getSystemDebug())Serial.print(F("[ INFO ] Trying to reconnect WiFi without given BSSID: "));
  if (config.getSystemDebug())Serial.print(config.getWifiSsid());
  return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
///////       RFID Functions
//////////////////////////////////////////////////////////////////////////////////////////
void ICACHE_FLASH_ATTR rfidloop() {
  
  scanResult scan = rfid.readPicc();
  
  if (scan.read) {
    Serial.print("UID: ");
    Serial.println(scan.uid);
    Serial.print("User: ");
    Serial.println(scan.user);
    Serial.print("Type: ");
    Serial.println(scan.type);
    Serial.print("Known: ");
    Serial.println(scan.known);
    Serial.print("Valid: ");
    Serial.println(scan.valid);

    StaticJsonDocument<230> jsonDoc;
    jsonDoc["command"] = "piccscan";
    jsonDoc["uid"] = scan.uid;
    jsonDoc["type"] = scan.type;
    
    lastUID = scan.uid;
    if (scan.known) { // PICC known
      Serial.println("PICC known");
      lastUsername = scan.user;
      jsonDoc["known"] = 1;
      jsonDoc["user"] = scan.user;
    }
    else { // Unknown PICC
      lastUsername = "Unknown";
      jsonDoc["known"] = 0;
    }

    if (scan.valid) {  // PICC valid
      Serial.println("PICC valid");
      if (evseActive) {
        toDeactivateEVSE = true;
      }
      else {
        toActivateEVSE = true;
      }
      #ifndef ESP8266
      millisUpdateOled = millis() + 3000;
      oled.showLock(false);
      #endif
      showLedRfidGrant = true;
    }
    else {
      #ifndef ESP8266
      millisUpdateOled = millis() + 3000;
      oled.showLock(true);
      #endif
      showLedRfidDecline = true;
      changeLedTimes(70, 70);
    }
    millisRfidLedAction = millis() + 1000;

    size_t len = measureJson(jsonDoc);
    AsyncWebSocketMessageBuffer * buffer = ws.makeBuffer(len);
    if (buffer) {
      serializeJson(jsonDoc, (char *)buffer->get(), len + 1);
      ws.textAll(buffer);
    }
  }
  if (millisRfidReset < millis() && config.getRfidActive() && !config.getEvseAlwaysActive(0)) {
    rfid.reset();
    millisRfidReset = millis() + 1000;
  }
}

s_addEvseData ICACHE_FLASH_ATTR getAdditionalEVSEData() {
  // Getting additional Modbus data
  s_addEvseData addEvseData;
  evseNode.clearTransmitBuffer();
  evseNode.clearResponseBuffer();
  uint8_t result = evseNode.readHoldingRegisters(0x07D0, 10);  // read 10 registers starting at 0x07D0 (2000)
  
  if (result != 0) {
    // error occured
    evseErrorCount ++;
    evseVehicleState = 0;
    Serial.print("[ ModBus ] Error ");
    Serial.print(result, HEX);
    Serial.println(" occured while getting additional EVSE data");
    if (config.getEvseLedConfig(0) == 3) changeLedTimes(300, 300);
    lastModbusAction = millis();
    return addEvseData;
  }
    evseErrorCount = 0;
    // register successfully read
    if (config.getSystemDebug()) Serial.println("[ ModBus ] got additional EVSE data successfully ");

    //process answer
    for (int i = 0; i < 10; i++) {
      switch(i) {
      case 0:
        addEvseData.evseAmpsAfterboot  = evseNode.getResponseBuffer(i);    //Register 2000
        evseAmpsAfterboot = addEvseData.evseAmpsAfterboot;
        break;
      case 1:
        addEvseData.evseModbusEnabled = evseNode.getResponseBuffer(i);     //Register 2001
        break;
      case 2:
        addEvseData.evseAmpsMin = evseNode.getResponseBuffer(i);           //Register 2002
        break;
      case 3:
        addEvseData.evseAnIn = evseNode.getResponseBuffer(i);             //Reg 2003
        break;
      case 4:
        addEvseData.evseAmpsPowerOn = evseNode.getResponseBuffer(i);      //Reg 2004
        break;
      case 5:
        addEvseData.evseReg2005 = evseNode.getResponseBuffer(i);          //Reg 2005
        break;
      case 6:
        addEvseData.evseShareMode = evseNode.getResponseBuffer(i);        //Reg 2006
        break;
      case 7:
        addEvseData.evsePpDetection = evseNode.getResponseBuffer(i);       //Register 2007
        break;
      case 9:
        addEvseData.evseBootFirmware = evseNode.getResponseBuffer(i);       //Register 2009
        break;
      }
    }
  return addEvseData;
}

void ICACHE_FLASH_ATTR sendStatus() {
  // Getting additional Modbus data
  //SoftSer.end();
  //SPIFFS.begin();
  fsWorking = true;
  #ifdef ESP8266
  struct ip_info info;
  FSInfo fsinfo;
  if (!SPIFFS.info(fsinfo)) {
  #else
  size_t total = 0, used = 0;
  esp_err_t spiffsret = ESP_FAIL; //= esp_spiffs_info(NULL, &total, &used);
  if (spiffsret != ESP_OK) {
    total = SPIFFS.totalBytes();
    used = SPIFFS.usedBytes();
    if (total != 0 || used != 0) {
      Serial.println("[ FILE SYSTEM ] Got data successfully");
      spiffsret = ESP_OK;
    }
  }
  if (spiffsret != ESP_OK) {
  #endif
    Serial.print(F("[ WARN ] Error getting info on SPIFFS, trying another way"));
  }
  //SPIFFS.end();
  delay(10);
  //SoftSer.begin(9600);
  fsWorking = false;
  StaticJsonDocument<1000> jsonDoc;
  jsonDoc["command"] = "status";
  jsonDoc["heap"] = ESP.getFreeHeap();
  jsonDoc["availsize"] = ESP.getFreeSketchSpace();
  jsonDoc["cpu"] = ESP.getCpuFreqMHz();
  jsonDoc["uptime"] = ntp.getDeviceUptimeString();
  
  #ifdef ESP8266
  jsonDoc["chipid"] = String(ESP.getChipId(), HEX);
  jsonDoc["availspiffs"] = fsinfo.totalBytes - fsinfo.usedBytes;
  jsonDoc["spiffssize"] = fsinfo.totalBytes;
  jsonDoc["hardwarerev"] = "ESP8266";
  #else
  jsonDoc["chipid"] = String((uint16_t)(ESP.getEfuseMac()>>32) + (uint32_t)ESP.getEfuseMac(), HEX);
  jsonDoc["availspiffs"] = total - used;
  jsonDoc["spiffssize"] = total;
  jsonDoc["hardwarerev"] = "ESP32";
  #endif


  #ifdef ESP8266
  if (inAPMode) {
    wifi_get_ip_info(SOFTAP_IF, &info);
    struct softap_config conf;
    wifi_softap_get_config(&conf);
    jsonDoc["ssid"] = String(reinterpret_cast<char*>(conf.ssid));
    jsonDoc["dns"] = printIP(WiFi.softAPIP());
    jsonDoc["mac"] = WiFi.softAPmacAddress();
  }
  else {
    wifi_get_ip_info(STATION_IF, &info);
    struct station_config conf;
    wifi_station_get_config(&conf);
    jsonDoc["ssid"] = String(reinterpret_cast<char*>(conf.ssid));
    jsonDoc["rssi"] = String(WiFi.RSSI());
    jsonDoc["dns"] = printIP(WiFi.dnsIP());
    jsonDoc["mac"] = WiFi.macAddress();
  }
  IPAddress ipaddr = IPAddress(info.ip.addr);
  IPAddress gwaddr = IPAddress(info.gw.addr);
  IPAddress nmaddr = IPAddress(info.netmask.addr);

  jsonDoc["ip"] = printIP(ipaddr);
  jsonDoc["netmask"] = printIP(nmaddr);

  #else
  wifi_config_t conf;
  tcpip_adapter_ip_info_t info;
  tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_ETH, &info);
  IPAddress ipaddr;
  IPAddress nmaddr;
  if (inAPMode) {
    esp_wifi_get_config(WIFI_IF_AP, &conf);
    jsonDoc["ssid"] = String(reinterpret_cast<char*>(conf.ap.ssid));
    jsonDoc["dns"] = printIP(WiFi.softAPIP());
    jsonDoc["mac"] = WiFi.softAPmacAddress();
    jsonDoc["ip"] = WiFi.softAPIP().toString();
    jsonDoc["netmask"] = printSubnet(WiFi.softAPSubnetCIDR());
  }
  else {
    esp_wifi_get_config(WIFI_IF_STA, &conf);
    jsonDoc["ssid"] = String(reinterpret_cast<char*>(conf.sta.ssid));
    jsonDoc["rssi"] = String(WiFi.RSSI());
    jsonDoc["dns"] = printIP(WiFi.dnsIP());
    jsonDoc["mac"] = WiFi.macAddress();
    jsonDoc["ip"] = WiFi.localIP().toString();
    jsonDoc["netmask"] = WiFi.subnetMask().toString();
  }
  IPAddress gwaddr = WiFi.gatewayIP();
  //jsonDoc["int_temp"] = String(((temprature_sens_read() - 32) / 1.8), 2);
  #endif

  s_addEvseData addEvseData = getAdditionalEVSEData();
  jsonDoc["gateway"] = printIP(gwaddr);
  jsonDoc["evse_amps_conf"] = evseAmpsConfig;          //Reg 1000
  jsonDoc["evse_amps_out"] = evseAmpsOutput;           //Reg 1001
  jsonDoc["evse_vehicle_state"] = evseVehicleState;   //Reg 1002
  jsonDoc["evse_pp_limit"] = evseAmpsPP;               //Reg 1003
  jsonDoc["evse_turn_off"] = evseTurnOff;              //Reg 1004
  jsonDoc["evse_firmware"] = evseFirmware;             //Reg 1005
  jsonDoc["evse_state"] = evseEvseState;                   //Reg 1006
  jsonDoc["evse_rcd"] = 0;                   //Reg 1007
  jsonDoc["evse_amps_afterboot"] = addEvseData.evseAmpsAfterboot;  //Reg 2000
  jsonDoc["evse_modbus_enabled"] = addEvseData.evseModbusEnabled;  //Reg 2001
  jsonDoc["evse_amps_min"] = addEvseData.evseAmpsMin;              //Reg 2002
  jsonDoc["evse_analog_input"] = addEvseData.evseAnIn;             //Reg 2003
  jsonDoc["evse_amps_poweron"] = addEvseData.evseAmpsPowerOn;      //Reg 2004
  jsonDoc["evse_2005"] = addEvseData.evseReg2005;                  //Reg 2005
  jsonDoc["evse_sharing_mode"] = addEvseData.evseShareMode;        //Reg 2006
  jsonDoc["evse_pp_detection"] = addEvseData.evsePpDetection;      //Reg 2007
  if (config.useMMeter) {
      delay(10);
      updateMMeterData();
      jsonDoc["meter_total"] = meterReading;
      jsonDoc["meter_p1"] = currentP1;
      jsonDoc["meter_p2"] = currentP2;
      jsonDoc["meter_p3"] = currentP3;
      jsonDoc["meter_p1_v"] = voltageP1;
      jsonDoc["meter_p2_v"] = voltageP2;
      jsonDoc["meter_p3_v"] = voltageP3;
  }
  size_t len = measureJson(jsonDoc);
  AsyncWebSocketMessageBuffer * buffer = ws.makeBuffer(len); //  creates a buffer (len + 1) for you.
  if (buffer) {
    serializeJson(jsonDoc, (char *)buffer->get(), len + 1);
    ws.textAll(buffer);
  }
}

// Send Scanned SSIDs to websocket clients as JSON object
void ICACHE_FLASH_ATTR printScanResult(int networksFound) {
  DynamicJsonDocument jsonDoc(3500);
  jsonDoc["command"] = "ssidlist";
  JsonArray jsonScanArray = jsonDoc.createNestedArray("list");
  for (int i = 0; i < networksFound; ++i) {
    JsonObject item = jsonScanArray.createNestedObject();
    // Print SSID for each network found
    item["ssid"] = WiFi.SSID(i);
    item["bssid"] = WiFi.BSSIDstr(i);
    item["rssi"] = WiFi.RSSI(i);
    item["channel"] = WiFi.channel(i);
    item["enctype"] = WiFi.encryptionType(i);
    #ifdef ESP8266
    item["hidden"] = WiFi.isHidden(i) ? true : false;
    #endif
  }
  size_t len = measureJson(jsonDoc);
  serializeJson(jsonDoc, Serial);
  AsyncWebSocketMessageBuffer * buffer = ws.makeBuffer(len);
  if (buffer) {
    serializeJson(jsonDoc, (char *)buffer->get(), len + 1);
    ws.textAll(buffer);
  }
  WiFi.scanDelete();
}

//////////////////////////////////////////////////////////////////////////////////////////
///////       Log Functions
//////////////////////////////////////////////////////////////////////////////////////////
void ICACHE_FLASH_ATTR logLatest(String uid, String username) {
  if (!config.getSystemLogging()) {
    return;
  }
  fsWorking = true;
  //SoftSer.end();
  //SPIFFS.begin();
  delay(30);
  File logFile = SPIFFS.open("/latestlog.json", "r");
  if (!logFile) {
    // Can not open file create it.
    File logFile = SPIFFS.open("/latestlog.json", "w");
    StaticJsonDocument<35> jsonDoc;
    jsonDoc["type"] = "latestlog";
    jsonDoc.createNestedArray("list");
    deserializeJson(jsonDoc, logFile);
    logFile.close();
    logFile = SPIFFS.open("/latestlog.json", "w+");
  }
  if (logFile) {
    size_t size = logFile.size();
    if (config.getSystemDebug()) Serial.println(size);

    std::unique_ptr<char[]> buf (new char[size]);
    logFile.readBytes(buf.get(), size);
    
    #ifndef ESP8266
    DynamicJsonDocument jsonDoc2(12000);
    #else
    DynamicJsonDocument jsonDoc2(6000);
    #endif
    DeserializationError error = deserializeJson(jsonDoc2, buf.get());
    JsonArray list = jsonDoc2["list"];
    if (error) {
      if (config.getSystemDebug()) Serial.println("[ SYSTEM ] Impossible to read log file");
    }
    else {
      logFile.close();
      #ifndef ESP8266
      if (list.size() >= 100) {
      #else
      if (list.size() >= 50) {
      #endif
        list.remove(0);
      }
      logFile = SPIFFS.open("/latestlog.json", "w");
      StaticJsonDocument<200> jsonDoc3;
      jsonDoc3["uid"] = uid;
      jsonDoc3["username"] = username;
      jsonDoc3["timestamp"] = ntp.getUtcTimeNow();
      jsonDoc3["duration"] = 0;
      jsonDoc3["energy"] = 0;
      jsonDoc3["price"] = config.getMeterEnergyPrice(0);
      list.add(jsonDoc3);

      String jsonSizeCalc = "";
      serializeJson(jsonDoc2, jsonSizeCalc);
      size_t logfileSize;

      for (int i = 0; i < 2; i++) {
        logfileSize = serializeJson(jsonDoc2, logFile);
        if (logfileSize == jsonSizeCalc.length()) {
          if (config.getSystemDebug()) Serial.println("LogFile verified!");
          break;
        }
        else {
          Serial.println("Error while writing LogFile... Trying 3 times");
          delay(50);
        }
      }
    }
    logFile.close();
  }
  else {
    if (config.getSystemDebug()) Serial.println("[ SYSTEM ] Cannot create Logfile");
  }
  //SPIFFS.end();
  delay(100);
  fsWorking = false;
  if (config.getSystemDebug()) Serial.println("reactivating SoftSer and SPIFFS");
  //SoftSer.begin(9600);
}

void ICACHE_FLASH_ATTR updateLog(bool e) {
  if (!config.getSystemLogging()) {
    return;
  }
  fsWorking = true;
  //SoftSer.end();
  //SPIFFS.begin();
  delay(30);
  File logFile = SPIFFS.open("/latestlog.json", "r");
  size_t size = logFile.size();
  std::unique_ptr<char[]> buf (new char[size]);
  logFile.readBytes(buf.get(), size);
  #ifndef ESP8266
  DynamicJsonDocument jsonDoc(12000);
  #else
  DynamicJsonDocument jsonDoc(6000);
  #endif
  DeserializationError error = deserializeJson(jsonDoc, buf.get());
  JsonArray list = jsonDoc["list"];
  if (error) {
    if (config.getSystemDebug()) Serial.println("[ SYSTEM ] Impossible to update log file");
    Serial.print("[ SYSTEM ] Impossible to parse Log file: ");
    Serial.println(error.c_str());
  }
  else {
    logFile.close();
    const char* uid = list[(list.size()-1)]["uid"];
    const char* username = list[(list.size()-1)]["username"];
    long timestamp = (long)list[(list.size()-1)]["timestamp"];

    list.remove(list.size() - 1); // delete newest log

    StaticJsonDocument<270> jsonDoc2;
    jsonDoc2["uid"] = uid;
    jsonDoc2["username"] = username;
    jsonDoc2["timestamp"] = timestamp;
    if (!e) {
      jsonDoc2["duration"] = getChargingTime();
      jsonDoc2["energy"] = float(int((meteredKWh + 0.005) * 100.0)) / 100.0;
      jsonDoc2["price"] = config.getMeterEnergyPrice(0);
    }
    else {
      jsonDoc2["duration"] = String("e");
      jsonDoc2["energy"] = String("e");
      jsonDoc2["price"] = String("e");
    }
    list.add(jsonDoc2);
    logFile = SPIFFS.open("/latestlog.json", "w");
    if (logFile) {
      String jsonSizeCalc = "";
      serializeJson(jsonDoc, jsonSizeCalc);
      size_t logfileSize;

      for (int i = 0; i < 2; i++) {
        logfileSize = serializeJson(jsonDoc, logFile);
        if (logfileSize == jsonSizeCalc.length()) {
          if (config.getSystemDebug()) Serial.println("LogFile verified!");
          break;
        }
        else {
          Serial.println("Error while writing LogFile... Trying 3 times");
          delay(50);
        }
      }
    }
  }
  logFile.close();
  //millisStartCharging = 0;
  //millisStopCharging = 0;
  //meteredKWh = 0.0;
  currentKW = 0.0;
  delay(100);
  fsWorking = false;
  //SPIFFS.end();
  if (config.getSystemDebug()) Serial.println("reactivating SoftSer");
  //SoftSer.begin(9600);
}

float ICACHE_FLASH_ATTR getS0MeterReading() {
  float fMeterReading = 0.0;
  if (!config.getSystemLogging()) {
    return fMeterReading;
  }

  fsWorking = true;
  //SoftSer.end();
  //SPIFFS.begin();
  File logFile = SPIFFS.open("/latestlog.json", "r");

  if (logFile) {
    size_t size = logFile.size();
    std::unique_ptr<char[]> buf (new char[size]);
    logFile.readBytes(buf.get(), size);
    #ifndef ESP8266
    DynamicJsonDocument jsonDoc(12000);
    #else
    DynamicJsonDocument jsonDoc(6000);
    #endif
    DeserializationError error = deserializeJson(jsonDoc, buf.get());
    JsonArray list = jsonDoc["list"];
    if (error) {
      if (config.getSystemDebug()) Serial.println("[ SYSTEM ] Impossible to read log file");
    }
    else {
      logFile.close();
      for (size_t i = 0; i < list.size(); i++) {
        JsonObject line = list.getElement(i);
        if (line["energy"] != "e") {
          fMeterReading += (float)line["energy"];
        }
      }
    }
  }
  delay(100);
  fsWorking = false;
  //SPIFFS.end();
  //SoftSer.begin(9600);
  return fMeterReading;
}

bool ICACHE_FLASH_ATTR initLogFile() {
  bool ret = true;
  fsWorking = true;
  //SoftSer.end();
  //SPIFFS.begin();
  if (config.getSystemDebug())Serial.println("[ SYSTEM ] Going to delete Log File...");
  File logFile = SPIFFS.open("/latestlog.json", "w");
  if (logFile) {
    StaticJsonDocument<35> jsonDoc;
    jsonDoc["type"] = "latestlog";
    jsonDoc.createNestedArray("list");
    serializeJson(jsonDoc, logFile);
    logFile.close();
    if (config.getSystemDebug())Serial.println("[ SYSTEM ] ... Success!");
    ret = true;
  }
  else {
    if (config.getSystemDebug())Serial.println("[ SYSTEM ] ... Failure!");
    ret = false;
  }
  //SPIFFS.end();
  delay(100);
  fsWorking = false;
  //SoftSer.begin(9600);
  return ret;
}

//////////////////////////////////////////////////////////////////////////////////////////
///////       Meter Modbus functions
//////////////////////////////////////////////////////////////////////////////////////////
float ICACHE_FLASH_ATTR readMeter(uint16_t reg) {
  uint8_t result;
  uint16_t iaRes[2];
  float fResponse = 0.0;
  meterNode.clearTransmitBuffer();
  meterNode.clearResponseBuffer();
  delay(50);
  result = meterNode.readInputRegisters(reg, 2);  // read 2 registers starting at 'reg'
  
  if (result != 0) {
    Serial.print("[ ModBus ] Error ");
    Serial.print(result, HEX);
    Serial.println(" occured while getting Meter Data");
    if (config.getEvseLedConfig(0) == 3) changeLedTimes(300, 300);
  }
  else {
    iaRes[0] = meterNode.getResponseBuffer(0);
    iaRes[1] = meterNode.getResponseBuffer(1);
    ((uint16_t*)&fResponse)[1]= iaRes[0];
    ((uint16_t*)&fResponse)[0]= iaRes[1];
  }
  return (fResponse);
}

//////////////////////////////////////////////////////////////////////////////////////////
///////       EVSE Modbus functions
//////////////////////////////////////////////////////////////////////////////////////////
bool ICACHE_FLASH_ATTR queryEVSE() {
  uint8_t result;
  evseNode.clearTransmitBuffer();
  evseNode.clearResponseBuffer();
  result = evseNode.readHoldingRegisters(0x03E8, 7);  // read 7 registers starting at 0x03E8 (1000)

  if (config.getEvseLedConfig(0) != 1) changeLedTimes(100, 10000);

  if (result != 0) {
    if (evseErrorCount > 2) {
      evseVehicleState = 0;
      evseStatus = 0;
      if (config.getEvseLedConfig(0) == 3) changeLedTimes(300, 300);
    }
    evseErrorCount ++;
    Serial.print("[ ModBus ] Error ");
    Serial.print(result, HEX);
    Serial.println(" occured while getting EVSE data - trying again...");
    evseNode.clearTransmitBuffer();
    evseNode.clearResponseBuffer();
    delay(500);
    lastModbusAction = millis();
    return false;
  }
  evseErrorCount = 0;
  lastModbusAction = millis();
  // register successfully read
  // process answer
  for (int i = 0; i < 7; i++) {
    switch(i) {
    case 0:
      evseAmpsConfig = evseNode.getResponseBuffer(i);     //Register 1000
      break;
    case 1:
      evseAmpsOutput = evseNode.getResponseBuffer(i);     //Register 1001
      break;
    case 2:
      evseVehicleState = evseNode.getResponseBuffer(i);   //Register 1002
      break;
    case 3:
      evseAmpsPP = evseNode.getResponseBuffer(i);          //Register 1003
      break;
    case 4:
      evseTurnOff = evseNode.getResponseBuffer(i);          //Register 1004
      break;
    case 5:
      evseFirmware = evseNode.getResponseBuffer(i);        //Register 1005
      break;
    case 6:
      evseEvseState = evseNode.getResponseBuffer(i);      //Register 1006
      break;
    //case 7:
    //  evseRcdStatus = evseNode.getResponseBuffer(i);      //Register 1007
    //  break;
    }
  }

  // Maximum slider value is independant of PP Limit - to activate PP-Limit for slider's max value uncomment here:
  //if (evseAmpsPP > config.getSystemMaxInstall()) {
    maxCurrent = config.getSystemMaxInstall();
  //}
  //else {
  //  maxCurrent = evseAmpsPP;
  //}
  
  // Normal Mode
  if (!config.getEvseAlwaysActive(0)) {
    if (evseVehicleState == 0) {
      evseStatus = 0; //modbus communication failed
      if (config.getEvseLedConfig(0) == 3) changeLedTimes(300, 300);
    }
    if (evseEvseState == 3) {     //EVSE not Ready
      if (evseVehicleState == 2 ||
          evseVehicleState == 3 ||
          evseVehicleState == 4) {
        evseStatus = 2; //vehicle detected
        if (config.getEvseLedConfig(0) == 3) changeLedTimes(300, 2000);
      }
      else {
        evseStatus = 1; // EVSE deactivated
      }
      if (vehicleCharging == true && manualStop == false) {   //vehicle interrupted charging
        millisStopCharging = millis();
        vehicleCharging = false;
        if (config.getSystemDebug()) Serial.println("[ SYSTEM ] Vehicle interrupted charging");
        updateLog(false);
      }
      evseActive = false;
      return true;
    }

    if (evseVehicleState == 1) {
      evseStatus = 1;  // ready
    }
    else if (evseVehicleState == 2) {
      evseStatus = 2; //vehicle detected
      if (config.getEvseLedConfig(0) == 3) changeLedTimes(300, 2000);
    }
    else if (evseVehicleState == 3 || evseVehicleState == 4) {
      evseStatus = 3; //charging
      if (config.getEvseLedConfig(0) == 3) changeLedTimes(2000, 1000);
    }
    else if (evseVehicleState == 5) {
      evseStatus = 5;
    }
  }
  
  // Always Active Mode
  else {
    if (evseVehicleState == 5) {
      evseStatus = 5;
    }
    if (evseEvseState == 1) { // Steady 12V
      if (vehicleCharging) { // EV interrupted charging
        millisStopCharging = millis();
        vehicleCharging = false;
        toDeactivateEVSE = true;
        lastUID = "vehicle";
        lastUsername = "vehicle";
        if (config.getSystemDebug()) Serial.println("[ SYSTEM ] Vehicle interrupted charging");
      }
      evseStatus = 1; // ready
      evseActive = true;
    }
    else if (evseEvseState == 2) { // PWM is being generated
      if (evseVehicleState == 2) { // EV is present
        if (vehicleCharging) {  // EV interrupted charging
          millisStopCharging = millis();
          vehicleCharging = false;
          toDeactivateEVSE = true;
          lastUID = "vehicle";
          lastUsername = "vehicle";
          if (config.getSystemDebug()) Serial.println("[ SYSTEM ] Vehicle interrupted charging");
        }
        evseStatus = 2; //vehicle detected
        evseActive = true;
        if (config.getEvseLedConfig(0) == 3) changeLedTimes(300, 2000);
      }
      else if (evseVehicleState == 3 || evseVehicleState == 4) {  // EV is charging
        if (!vehicleCharging) { // EV starts charging
          millisStartCharging = millis();
          meteredKWh = 0.0;
          vehicleCharging = true;
          toActivateEVSE = true;
          lastUID = "vehicle";
          lastUsername = "vehicle";
        }
        evseStatus = 3; //charging
        evseActive = true;
        if (config.getEvseLedConfig(0) == 3) changeLedTimes(2000, 1000);
      }
    }
    else if (evseEvseState == 3) {     //EVSE not Ready
      if (evseVehicleState == 2 ||
          evseVehicleState == 3 ||
          evseVehicleState == 4) {
        evseStatus = 2; //vehicle detected
        if (config.getEvseLedConfig(0) == 3) changeLedTimes(300, 2000);
        if (vehicleCharging && evseAmpsConfig == 0) { //Current Set to 0 - deactivate
          millisStopCharging = millis();
          vehicleCharging = false;
          toDeactivateEVSE = true;
          lastUID = "API";
          lastUsername = "API";
          if (config.getSystemDebug()) Serial.println("[ SYSTEM ] API interrupted charging");
        }
      }
      else {
        evseStatus = 1; // EVSE deactivated
      }
      if (vehicleCharging == true && manualStop == false) {   //vehicle interrupted charging
        millisStopCharging = millis();
        vehicleCharging = false;
        lastUID = "vehicle";
        lastUsername = "vehicle";
        if (config.getSystemDebug()) Serial.println("[ SYSTEM ] Vehicle interrupted charging");
        updateLog(false);
      }
      evseActive = false;
      return true;
    }
  }
  return true;
}

bool ICACHE_FLASH_ATTR activateEVSE() {
  if (!config.getEvseAlwaysActive(0)) {
    static uint16_t iTransmit;

    if (config.useMMeter) {
      if (millisUpdateMMeter - millis() < 50) {
        delay(50);
      }
    }
    if (evseEvseState == 3 &&
      evseVehicleState != 0) {    //no modbus error occured
      iTransmit = 8192;         // disable EVSE after charge
      iTransmit += 32;         // auto reset EVSE after RCD error (30s)

      uint8_t result;
      evseNode.clearTransmitBuffer();
      evseNode.setTransmitBuffer(0, iTransmit); // set word 0 of TX buffer (bits 15..0)
      result = evseNode.writeMultipleRegisters(0x07D5, 1);  // write register 0x07D5 (2005)

      if (result != 0) {
        // error occured
        Serial.print("[ ModBus ] Error ");
        Serial.print(result, HEX);
        Serial.println(" occured while activating EVSE - trying again...");
        if (config.getEvseLedConfig(0) == 3) changeLedTimes(300, 300);
        delay(500);
        return false;
      }

      millisStartCharging = millis();
      manualStop = false;
      // register successfully written
      if (config.getSystemDebug()) Serial.println("[ ModBus ] EVSE successfully activated");
    }
  }
  toActivateEVSE = false;
  evseActive = true;
  logLatest(lastUID, lastUsername);
  vehicleCharging = true;
  meteredKWh = 0.0;

  #ifndef ESP8266
  millisUpdateOled = millis() + 3000;
  oled.showLock(false);
  #endif
  showLedRfidGrant = true;

  if (config.useMMeter) {
    millisUpdateMMeter += 5000;
    startTotal = meterReading;
  }
  else {
    startTotal = getS0MeterReading();
    numberOfMeterImps = 0;
  }
  sendEVSEdata();
  return true;
}

bool ICACHE_FLASH_ATTR deactivateEVSE(bool logUpdate) {
  if (!config.getEvseAlwaysActive(0)) {
    //New ModBus Master Library
    static uint16_t iTransmit = 16384;  // deactivate evse
    iTransmit += 32;         // auto reset EVSE after RCD error (30s)

    uint8_t result;

    if (config.useMMeter) {
      if (millisUpdateMMeter - millis() < 50) {
        delay(50);
      }
    }

    evseNode.clearTransmitBuffer();
    evseNode.setTransmitBuffer(0, iTransmit); // set word 0 of TX buffer (bits 15..0)
    result = evseNode.writeMultipleRegisters(0x07D5, 1);  // write register 0x07D5 (2005)

    if (result != 0) {
      // error occured
      Serial.print("[ ModBus ] Error ");
      Serial.print(result, HEX);
      Serial.println(" occured while deactivating EVSE - trying again...");
      if (config.getEvseLedConfig(0) == 3) changeLedTimes(300, 300);
      delay(500);
      return false;
    }

    // register successfully written
    if (config.getSystemDebug()) Serial.println("[ ModBus ] EVSE successfully deactivated");
    evseActive = false;
    millisStopCharging = millis();
  }
  manualStop = true;
  if (config.useMMeter) {
    meteredKWh = meterReading - startTotal;
  }
  else {
    startTotal += meteredKWh;
  }  
  if (logUpdate) {
    updateLog(false);
  }
  vehicleCharging = false;
  toDeactivateEVSE = false;
  
  if (config.getEvseResetCurrentAfterCharge(0) == true) {
    currentToSet = evseAmpsAfterboot;
    toSetEVSEcurrent = true;
  }
  sendEVSEdata();
  return true;
}

bool ICACHE_FLASH_ATTR setEVSEcurrent() {  // telegram 1: write EVSE current
  //New ModBus Master Library
  uint8_t result;

  if (config.useMMeter) {
    if (millisUpdateMMeter - millis() < 50) {
      delay(50);
    }
  }

  evseNode.clearTransmitBuffer();
  evseNode.setTransmitBuffer(0, currentToSet); // set word 0 of TX buffer (bits 15..0)
  result = evseNode.writeMultipleRegisters(0x03E8, 1);  // write register 0x03E8 (1000 - Actual configured amps value)

  if (result != 0) {
    // error occured
    Serial.print("[ ModBus ] Error ");
    Serial.print(result, HEX);
    Serial.println(" occured while setting current in EVSE - trying again...");
    if (config.getEvseLedConfig(0) == 3) changeLedTimes(300, 300);
    delay(500);
    return false;
  }

  // register successfully written
  if (config.getSystemDebug()) Serial.println("[ ModBus ] Current successfully set");
  evseAmpsConfig = currentToSet;  //foce update in WebUI
  sendEVSEdata();               //foce update in WebUI
  toSetEVSEcurrent = false;
  return true;
}

bool ICACHE_FLASH_ATTR setEVSERegister(uint16_t reg, uint16_t val) {
  uint8_t result;
  evseNode.clearTransmitBuffer();
  evseNode.setTransmitBuffer(0, val); // set word 0 of TX buffer (bits 15..0)
  result = evseNode.writeMultipleRegisters(reg, 1);  // write given register

  if (result != 0) {
    // error occured
    Serial.print("[ ModBus ] Error ");
    Serial.print(result, HEX);
    Serial.println(" occured while setting EVSE Register " + (String)reg + " to " + (String)val);
    if (config.getEvseLedConfig(0) == 3) changeLedTimes(300, 300);
    return false;
  }

  // register successfully written
  if (config.getSystemDebug()) Serial.println("[ ModBus ] Register " + (String)reg + " successfully set to " + (String)val);
  return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
///////       Websocket Functions
//////////////////////////////////////////////////////////////////////////////////////////
void ICACHE_FLASH_ATTR pushSessionTimeOut() {
  // push "TimeOut" to evse.htm!
  // Encode a JSON Object and send it to All WebSocket Clients
  StaticJsonDocument<40> jsonDoc;
  jsonDoc["command"] = "sessiontimeout";
  size_t len = measureJson(jsonDoc);
  AsyncWebSocketMessageBuffer * buffer = ws.makeBuffer(len);
  if (buffer) {
    serializeJson(jsonDoc, (char *)buffer->get(), len + 1);
    ws.textAll(buffer);
  }
}

void ICACHE_FLASH_ATTR sendEVSEdata() {
  if (evseSessionTimeOut == false) {
    StaticJsonDocument<480> jsonDoc;
    jsonDoc["command"] = "getevsedata";
    jsonDoc["evse_vehicle_state"] = evseStatus;
    jsonDoc["evse_active"] = evseActive;
    jsonDoc["evse_current_limit"] = evseAmpsConfig;
    jsonDoc["evse_slider_status"] = sliderStatus;
    #ifndef ESP8266
    jsonDoc["evse_rse_status"] = rseActive;
    jsonDoc["evse_rse_current_before"] = currentBeforeRse;
    #endif
    jsonDoc["evse_rse_value"] = config.getEvseRseValue(0);
    jsonDoc["evse_current"] = String(currentKW, 2);
    jsonDoc["evse_charging_time"] = getChargingTime();
    jsonDoc["evse_always_active"] = config.getEvseAlwaysActive(0);
    jsonDoc["evse_charged_kwh"] = String(meteredKWh, 2);
    jsonDoc["evse_charged_amount"] = String((meteredKWh * float(config.getMeterEnergyPrice(0)) / 100.0), 2);
    jsonDoc["evse_maximum_current"] = maxCurrent;
    if (meteredKWh == 0.0) {
      jsonDoc["evse_charged_mileage"] = "0.0";
    }
    else {
      jsonDoc["evse_charged_mileage"] = String((meteredKWh * 100.0 / config.getEvseAvgConsumption(0)), 1);
    }
    jsonDoc["ap_mode"] = inAPMode;
    size_t len = measureJson(jsonDoc);
    AsyncWebSocketMessageBuffer * buffer = ws.makeBuffer(len);
    if (buffer) {
      serializeJson(jsonDoc, (char *)buffer->get(), len + 1);
      ws.textAll(buffer);
    }
  }
}

void ICACHE_FLASH_ATTR sendTime() {
  StaticJsonDocument<100> jsonDoc;
  jsonDoc["command"] = "gettime";
  jsonDoc["epoch"] = now();
  jsonDoc["timezone"] = config.getNtpTimezone();
  size_t len = measureJson(jsonDoc);
  AsyncWebSocketMessageBuffer * buffer = ws.makeBuffer(len);
  if (buffer) {
    serializeJson(jsonDoc, (char *)buffer->get(), len + 1);
    ws.textAll(buffer);
  }
}

void ICACHE_FLASH_ATTR sendStartupInfo(AsyncWebSocketClient * client) {
  #ifdef ESP8266
  String message = "{\"command\":\"startupinfo\",\"hw_rev\":\"ESP8266\",\"sw_rev\":\"" + swVersion + "\",\"pp_limit\":\"" + (String)evseAmpsPP + "\"}";
  #else
  String message = "{\"command\":\"startupinfo\",\"hw_rev\":\"ESP32\",\"sw_rev\":\"" + swVersion + "\",\"pp_limit\":\"" + (String)evseAmpsPP + "\"}";
  #endif
  client->text(message);
}

void ICACHE_FLASH_ATTR sendUserList(int page, AsyncWebSocketClient * client) {
  DynamicJsonDocument jsonDoc(3000);
  jsonDoc = rfid.getUserList(page);
  size_t len = measureJson(jsonDoc);
  AsyncWebSocketMessageBuffer * buffer = ws.makeBuffer(len);
  if (buffer) {
    serializeJson(jsonDoc, (char *)buffer->get(), len + 1);
    if (client) {
      client->text(buffer);
      client->text("{\"command\":\"result\",\"resultof\":\"userlist\",\"result\": true}");
    } else {
      ws.textAll("{\"command\":\"result\",\"resultof\":\"userlist\",\"result\": false}");
    }
  }
}

#ifndef ESP8266
bool ICACHE_FLASH_ATTR interruptCp() {
  digitalWrite(config.getEvseCpIntPin(0), HIGH);
  millisInterruptCp = millis() + 3000;
  doCpInterruptCp = true;
  Serial.println("Interrupt CP started");
  return true;
}
#endif

void ICACHE_FLASH_ATTR onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_ERROR) {
    if (config.getSystemDebug()) Serial.printf("[ WARN ] WebSocket[%s][%u] error(%u): %s\r\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
  }
  else if (type == WS_EVT_DATA) {
    AwsFrameInfo * info = (AwsFrameInfo*)arg;
    if (info->final && info->index == 0 && info->len == len) {
      //the whole message is in a single frame and we got all of it's data
      for (size_t i = 0; i < info->len; i++) {
        msg += (char) data[i];
      }
      StaticJsonDocument<1800> jsonDoc;
      DeserializationError error = deserializeJson(jsonDoc, msg);
      if (error) {
        if (config.getSystemDebug()) Serial.println(F("[ WARN ] Couldn't parse WebSocket message"));
        msg = "";
        return;
      }
      processWsEvent(jsonDoc, client);
    }
    else {
      //message is comprised of multiple frames or the frame is split into multiple packets
      if (config.getSystemDebug())Serial.println("[ Websocket ] more than one Frame!");
      for (size_t i = 0; i < len; i++) {
        msg += (char) data[i];
      }
      if (info->final && (info->index + len) == info->len) {
        StaticJsonDocument<1800> jsonDoc;
        DeserializationError error = deserializeJson(jsonDoc, msg);
        if (error) {
          if (config.getSystemDebug()) Serial.println(F("[ WARN ] Couldn't parse WebSocket message"));
          msg = "";
          return;
        }
        processWsEvent(jsonDoc, client);
      }
    }
  }
}

void ICACHE_FLASH_ATTR processWsEvent(JsonDocument& root, AsyncWebSocketClient * client) {
  const char * command = root["command"];
  //File configFile;
  if (strcmp(command, "remove") == 0) {
    const char* uid = root["uid"];
    String filename = "/P/";
    filename += uid;
    //SoftSer.end();
    fsWorking = true;
    //SPIFFS.begin();
    SPIFFS.remove(filename);
    //SPIFFS.end();
    delay(10);
    //SoftSer.begin(9600);
    fsWorking = false;
  }
  else if (strcmp(command, "configfile") == 0) {
    if (config.getSystemDebug()) Serial.println("[ SYSTEM ] Try to update config.json...");
    String configString;
    serializeJson(root, configString);
    if (config.updateConfig(configString)) {
      if (config.getSystemDebug()) Serial.println("[ SYSTEM ] Success - going to reboot now");
      if (vehicleCharging) {
        deactivateEVSE(true);
        delay(100);
      }
      #ifdef ESP8266
      ESP.reset();
      #else
      ESP.restart();
      #endif
    }
    else {
      if (config.getSystemDebug()) Serial.println("[ SYSTEM ] Could not save config.json");
    }
  }
  else if (strcmp(command, "userlist") == 0) {
    int page = root["page"];
    sendUserList(page, client);
  }
  else if (strcmp(command, "status") == 0) {
    toSendStatus = true;
  }
  else if (strcmp(command, "userfile") == 0) {
    const char* uid = root["uid"];
    String filename = "/P/";
    filename += uid;
    //SoftSer.end();
    File userFile = SPIFFS.open(filename, "w+");
    // Check if we created the file
    if (userFile) {
      userFile.print(msg);
      if (config.getSystemDebug()) Serial.println("[ DEBUG ] Userfile written!");
    }
    userFile.close();
    //SoftSer.begin(9600);
    ws.textAll("{\"command\":\"result\",\"resultof\":\"userfile\",\"result\": true}");
  }
  else if (strcmp(command, "latestlog") == 0) {
    if (!fsWorking) {
      fsWorking = true;
      //SoftSer.end();
      //SPIFFS.begin();
      File logFile = SPIFFS.open("/latestlog.json", "r");
      if (logFile) {
        size_t len = logFile.size();
        AsyncWebSocketMessageBuffer * buffer = ws.makeBuffer(len);
        if (buffer) {
          logFile.readBytes((char *)buffer->get(), len + 1);
          ws.textAll(buffer);
        }
        logFile.close();
      }
      else {
        Serial.println("[ SYSTEM ] Error while reading log file");
      }
      //SPIFFS.end();
      delay(10);
      //SoftSer.begin(9600);
      fsWorking = false;
    }
  }
  else if (strcmp(command, "scan") == 0) {
    #ifdef ESP8266
    WiFi.scanNetworksAsync(printScanResult, true);
    #else
    int networks = WiFi.scanNetworks();
    printScanResult(networks);
    #endif
  }
  else if (strcmp(command, "gettime") == 0) {
    sendTime();
  }
  else if (strcmp(command, "settime") == 0) {
    unsigned long t = root["epoch"];
    //t += (config.getNtpTimezone * 3600);
    setTime(t);
    sendTime();
  }
  else if (strcmp(command, "getconf") == 0) {
    ws.textAll(config.getConfigJson());
  }
  else if (strcmp(command, "getevsedata") == 0) {
    sendEVSEdata();
    evseQueryTimeOut = millis() + 10000; //Timeout for pushing data in loop
    evseSessionTimeOut = false;
    if (config.getSystemDebug()) Serial.println("[ WebSocket ] Data sent to UI");
  }
  else if (strcmp(command, "setcurrent") == 0) {
    currentToSet = root["current"];
    if (config.getSystemDebug()) Serial.print("[ WebSocket ] Call setEVSECurrent() ");
    if (config.getSystemDebug()) Serial.println(currentToSet);
    toSetEVSEcurrent = true;
  }
  else if (strcmp(command, "activateevse") == 0) {
    toActivateEVSE = true;
    if (config.getSystemDebug()) Serial.println("[ WebSocket ] Activate EVSE via WebSocket");
    lastUID = "GUI";
    lastUsername = "GUI";
  }
  else if (strcmp(command, "deactivateevse") == 0) {
    toDeactivateEVSE = true;
    if (config.getSystemDebug()) Serial.println("[ WebSocket ] Deactivate EVSE via WebSocket");
    lastUID = "GUI";
    lastUsername = "GUI";
  }
  else if (strcmp(command, "setevsereg") == 0) {
    uint16_t reg = atoi(root["register"]);
    uint16_t val = atoi(root["value"]);
    setEVSERegister(reg, val);
  }
  else if (strcmp(command, "factoryreset") == 0) {
    factoryReset();
  }
  else if (strcmp(command, "resetuserdata") == 0) {
    if (resetUserData()) {
      if (config.getSystemDebug()) Serial.println("[ WebSocket ] User Data Reset successfully done");
    }
  }
  else if (strcmp(command, "initlog") == 0) {
    if (config.getSystemDebug())Serial.println("[ SYSTEM ] Websocket Command \"initlog\"...");
    initLogFile();
  }
  else if (strcmp(command, "getstartup") == 0) {
    sendStartupInfo(client);
  }

  #ifndef ESP8266
  else if (strcmp(command, "interruptcp") == 0) {
    if (config.getSystemDebug())Serial.println("[ SYSTEM ] Websocket Command \"interruptcp\"...");
    interruptCp();
  }
  #endif
  msg = "";
}

//////////////////////////////////////////////////////////////////////////////////////////
///////       Modbus/TCP Functions
//////////////////////////////////////////////////////////////////////////////////////////

uint16_t onSetMbTCPHreg(TRegister *reg, uint16_t val) {
  switch (reg->address.address) {
  case 40000: // Configured Current
    if (val <= config.getSystemMaxInstall() && val >= 6) {
      toSetEVSEcurrent = true;
      currentToSet = val;
      return val;
    }
    else {
      return false;
    }
    break;
  case 40001: // Station Status
    if (val == 1 && evseActive == false) {
      toActivateEVSE = true;
      return val;
    }
    else if (val == 0 && evseActive == true) {
      toDeactivateEVSE = true;
      return val;
    }
    return false;
    break;
  case 40003: // CP Interrupt
    #ifndef ESP8266
    if (val == 1) {
      interruptCp();
      return 0;
    }
    #endif
    return false;
    break;
  default:
    break;
  }
  return -1;
}

uint16_t onGetMbTCPHreg(TRegister *reg, uint16_t val) {
  switch (reg->address.address) {
  case 40000: // Configured Current
    return evseAmpsConfig;
    break;
  case 40001: // Station Status
    if (evseActive == false) return 0;
    if (evseActive == true) return 1;
    break;
  case 40002: // RSE
    if (digitalRead(config.getEvseRsePin(0)) == LOW && config.getEvseRseActive(0) == true) {
      return 1;
    }
    else {
      return 0;
    }
  default:
    break;
  }
  return -1;
}

uint16_t onGetMbTCPIreg(TRegister *reg, uint16_t val)
{

  uint16_t _currentP1;
  uint16_t _currentP2;
  uint16_t _currentP3;
  
  if (config.getMeterPhaseCount(0) == 1) {
    float fCurrent = float(int((currentKW / float(config.getMeterFactor(0)) / 0.227 + 0.005) * 100.0) / 100.0);
    if (config.getMeterFactor(0) == 1) {
      _currentP1 = fCurrent;
      _currentP2 = 0.0;
      _currentP3 = 0.0;
    }
    else if (config.getMeterFactor(0) == 2) {
      _currentP1 = fCurrent;
      _currentP2 = fCurrent;
      _currentP3 = 0.0;
    }
    else if (config.getMeterFactor(0) == 3) {
      _currentP1 = fCurrent;
      _currentP2 = fCurrent;
      _currentP3 = fCurrent;
    }
  }
  else {
    float fCurrent = float(int((currentKW / 0.227 / float(config.getMeterFactor(0)) / 3.0 + 0.005) * 100.0) / 100.0);
    _currentP1 = fCurrent;
    _currentP2 = fCurrent;
    _currentP3 = fCurrent;
  }

  switch (reg->address.address) {
  // Meter Data
  case 30001:
    if (config.useMMeter) return uint16_t(currentP1 * 100.0);
    if (config.useSMeter) return uint16_t(_currentP1 * 100.0);
    break;
  case 30002:
    if (config.useMMeter) return uint16_t(currentP2 * 100.0);
    if (config.useSMeter) return uint16_t(_currentP2 * 100.0);
    break;
  case 30003:
    if (config.useMMeter) return uint16_t(currentP3 * 100.0);
    if (config.useSMeter) return uint16_t(_currentP3 * 100.0);
    break;
  case 30004:
    if (config.useMMeter) return uint16_t(voltageP1 * 100.0);
    if (config.useSMeter) return uint16_t(230 * 100);
    break;
  case 30005:
    if (config.useMMeter) return uint16_t(voltageP2 * 100.0);
    if (config.useSMeter && (config.getMeterFactor(0) == 2 || config.getMeterFactor(0) == 3 || config.getMeterPhaseCount(0) == 3)) return uint16_t(230 * 100);
    return 0;
    break;
  case 30006:
    if (config.useMMeter) return uint16_t(voltageP3 * 100.0);
    if (config.useSMeter && (config.getMeterFactor(0) == 3 || config.getMeterPhaseCount(0) == 3)) return uint16_t(230 * 100);
    return 0;
    break;
  case 30007: // Meter Reading 1
    if (config.useMMeter == true) {
      return get16bitOfFloat32(meterReading, 0);
    }
    else {
      return get16bitOfFloat32((startTotal + meteredKWh), 0);
    }
    break;
  case 30008: // Meter Reading 2
    if (config.useMMeter == true) {
      return get16bitOfFloat32(meterReading, 1);
    }
    else {
      return get16bitOfFloat32((startTotal + meteredKWh), 1);
    }
    break;
  case 30009: //Total Power (W) 1
    return get16bitOfFloat32(currentKW, 0);
    break;
  case 30010: //Total Power (W) 2
    return get16bitOfFloat32(currentKW, 1);
    break;

  // Charging Data
  case 30100: // Vehicle State
    return uint16_t(evseStatus);
    break;
  case 30101: // PP-Limit
    return uint16_t(evseAmpsPP);
    break;
  case 30102: // Duration (s)
    return uint16_t(getChargingTime() / 1000);
    break;
  case 30103: // Charged Energy 1
    return get16bitOfFloat32(meteredKWh, 0);
    break;
  case 30104: // Charged Energy 2
    return get16bitOfFloat32(meteredKWh, 1);
    break;
  case 30105: // Mileage (km)
    return int(((meteredKWh * 100.0 / config.getEvseAvgConsumption(0)) + 0.05) * 10.0);
    break;

  // Charging Point Data
  case 30200: // Operating Mode
    if (config.getEvseRemote(0)) {
      return 3;
    }
    if (config.getEvseAlwaysActive(0)) {
      return 2;
    }
    return 1;
    break;
  case 30201: // Firmware Major
    return sw_maj;
    break;
  case 30202: // Firmware Minor
    return sw_min;
    break;
  case 30203: // Firmware Revision
    return sw_rev;
    break;
  default:
    break;
  }
  return -1;
}

void setModbusTCPRegisters() {
  modbusTCPServerNode.server();

  //Registers 30000
  modbusTCPServerNode.addIreg(30001);
  modbusTCPServerNode.addIreg(30002);
  modbusTCPServerNode.addIreg(30003);
  modbusTCPServerNode.addIreg(30004);
  modbusTCPServerNode.addIreg(30005);
  modbusTCPServerNode.addIreg(30006);
  modbusTCPServerNode.addIreg(30007);
  modbusTCPServerNode.addIreg(30008);
  modbusTCPServerNode.addIreg(30009);
  modbusTCPServerNode.addIreg(30010);
  modbusTCPServerNode.onGetIreg(30001, onGetMbTCPIreg, 10);

  modbusTCPServerNode.addIreg(30100);
  modbusTCPServerNode.addIreg(30101);
  modbusTCPServerNode.addIreg(30102);
  modbusTCPServerNode.addIreg(30103);
  modbusTCPServerNode.addIreg(30104);
  modbusTCPServerNode.addIreg(30105);
  modbusTCPServerNode.onGetIreg(30100, onGetMbTCPIreg, 6);
  
  modbusTCPServerNode.addIreg(30200);
  modbusTCPServerNode.addIreg(30201);
  modbusTCPServerNode.addIreg(30202);
  modbusTCPServerNode.addIreg(30203);
  modbusTCPServerNode.onGetIreg(30200, onGetMbTCPIreg, 4);

  //Registers 40000
  modbusTCPServerNode.addHreg(40000);
  modbusTCPServerNode.addHreg(40001);
  modbusTCPServerNode.addHreg(40002);
  modbusTCPServerNode.addHreg(40003);
  modbusTCPServerNode.onGetHreg(40000, onGetMbTCPHreg, 3);
  modbusTCPServerNode.onSetHreg(40000, onSetMbTCPHreg, 2);
  modbusTCPServerNode.onSetHreg(40003, onSetMbTCPHreg, 1);
}

//////////////////////////////////////////////////////////////////////////////////////////
///////       Setup Functions
//////////////////////////////////////////////////////////////////////////////////////////
bool ICACHE_FLASH_ATTR connectSTA(const char* ssid, const char* password, byte bssid[6]) {
  delay(100);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password, 0, bssid);
  Serial.print(F("[ INFO ] Trying to connect WiFi: "));
  Serial.print(ssid);

  unsigned long now = millis();
  uint8_t timeout = 10;
  do {
    if (WiFi.status() == WL_CONNECTED) {
      break;
    }
    delay(500);
    if (config.getSystemDebug()) Serial.print(F("."));
  }
  while (millis() - now < timeout * 1000);
  if (WiFi.status() == WL_CONNECTED) {
    isWifiConnected = true;
    return true;
  }

  //Try again without given BSSID
  WiFi.disconnect();
  delay(100);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password, 0);
  if (config.getSystemDebug()) Serial.println();
  if (config.getSystemDebug()) Serial.println(F("[ WARN ] Couldn't connect in time"));
  Serial.print(F("[ INFO ] Trying to connect WiFi without given BSSID: "));
  Serial.print(ssid);
  now = millis();
  do {
    if (WiFi.status() == WL_CONNECTED) {
      break;
    }
    delay(500);
    if (config.getSystemDebug()) Serial.print(F("."));
  }
  while (millis() - now < timeout * 1000);

  if (WiFi.status() == WL_CONNECTED) {
    isWifiConnected = true;
    return true;
  }

  if (config.getSystemDebug()) Serial.println();
  if (config.getSystemDebug()) Serial.println(F("[ WARN ] Couldn't connect in time"));
  return false;
}

bool ICACHE_FLASH_ATTR startAP(const char * ssid, const char * password = NULL) {
  inAPMode = true;
  WiFi.mode(WIFI_AP);
  Serial.print(F("[ INFO ] Configuring access point... "));
  bool success = WiFi.softAP(ssid, password);
  Serial.println(success ? "Ready" : "Failed!");
  // Access Point IP
  IPAddress myIP = WiFi.softAPIP();
  Serial.print(F("[ INFO ] AP IP address: "));
  Serial.println(myIP);
  Serial.printf("[ INFO ] AP SSID: %s\n", ssid);
  isWifiConnected = success;
  return success;
}

bool ICACHE_FLASH_ATTR loadConfiguration(String configString = "") {

  Serial.println("[ SYSTEM ] Loading Config File on Startup...");
  if (configString == "") {
    if (!config.loadConfig()) return false;
  }
  else {
    if (!config.loadConfig(configString)) return false;
  }
  config.loadConfiguration();

  if (config.getSystemDebug()) Serial.println("[ SYSTEM ] Check for old config version and renew it");
  config.renewConfigFile();

  delay(300); // wait a few milliseconds to prevent voltage drop...

  if (config.getSystemDebug()) {
    Serial.println("[ SYSTEM ] Debug Mode: ON!");
  }
  else {
    Serial.println("[ SYSTEM ] Debug Mode: OFF!");
  }

  byte bssid[6];
  parseBytes(config.getWifiBssid(), ':', bssid, 6, 16);
  #ifdef ESP8266
  WiFi.hostname(config.getSystemHostname());
  #else
  WiFi.setHostname(config.getSystemHostname());
  #endif
  
  if (!MDNS.begin(config.getSystemHostname())) {
    Serial.println("[ SYSTEM ] Error setting up MDNS responder!");
  }
  MDNS.addService("http", "tcp", 80);

  if (!config.getSystemWsauth()) {
    ws.setAuthentication("admin", config.getSystemPass());
    if (config.getSystemDebug())Serial.println("[ Websocket ] Use Basic Authentication for Websocket");
  }
  #ifdef ESP8266
  server.addHandler(new SPIFFSEditor("admin", config.getSystemPass()));
  #else
  server.addHandler(new SPIFFSEditor(SPIFFS, "admin", config.getSystemPass()));
  #endif

  while (!queryEVSE()) {
    delay(500);
    if (evseErrorCount > 2) {
      break;
    }
  }
  delay(50);

  s_addEvseData addEvseData = getAdditionalEVSEData();
  while (evseErrorCount != 0) {
    delay(500);
    addEvseData = getAdditionalEVSEData();
    if (evseErrorCount > 2) {
      break;
    }
  }
  delay(50);

  if (vehicleCharging) {
    updateLog(true);
  }

  if (evseAmpsConfig == 0 && 
    !config.getEvseAlwaysActive(0) && !config.getEvseRemote(0)) {
      currentToSet = evseAmpsAfterboot;
      toSetEVSEcurrent = true;
  }

  if (config.getEvseAlwaysActive(0)) {
    evseActive = true;
    if (addEvseData.evseReg2005 != 0) {
      delay(50);
      setEVSERegister(2005, 0);
    }
    if (config.getSystemDebug()) Serial.println(F("[ INFO ] EVSE-WiFi runs in always active mode"));
  }

  if (config.getRfidActive() == true && config.getEvseAlwaysActive(0) == false) {
    if (config.getSystemDebug()) Serial.println(F("[ INFO ] Trying to setup RFID hardware"));
    rfid.begin(config.getRfidPin(), config.getRfidUsePN532(), config.getRfidGain(), &ntp, config.getSystemDebug());
  }

  deactivateEVSE(false);  //initial deactivation
  millisStopCharging = 0;
  vehicleCharging = false;
  
  if (config.getWifiWmode() == 1) {
    if (config.getSystemDebug()) Serial.println(F("[ INFO ] EVSE-WiFi is running in AP Mode "));
    WiFi.disconnect(true);
    return startAP(config.getWifiSsid(), config.getWifiPass());
  }
  if (!connectSTA(config.getWifiSsid(), config.getWifiPass(), bssid)) {
    return false;
  }

  if (config.getWifiStaticIp() == true) {
    IPAddress clientip;
    IPAddress subnet;
    IPAddress gateway;
    IPAddress dns;

    clientip.fromString(config.getWifiIp());
    subnet.fromString(config.getWiFiSubnet());
    gateway.fromString(config.getWiFiGateway());
    dns.fromString(config.getWiFiDns());

    WiFi.config(clientip, gateway, subnet, dns);
  }

  Serial.println();
  Serial.print(F("[ INFO ] Client IP address: "));
  Serial.println(WiFi.localIP());

  //Check internet connection
  delay(100);
  if (config.getSystemDebug()) Serial.print("[ NTP ] NTP Server - set up NTP");
  const char * ntpserver = config.getNtpIp();
  IPAddress timeserverip;
  WiFi.hostByName(ntpserver, timeserverip);
  String ip = printIP(timeserverip);
  if (config.getSystemDebug()) Serial.println(" IP: " + ip);
  uint8_t tz = config.getNtpTimezone();
  if (config.getNtpDst()) {
    tz = tz + 1;
    if (config.getSystemDebug()) Serial.print("[ NTP ] Timezone:");
    if (config.getSystemDebug()) Serial.println(tz);
    if (config.getSystemDebug()) Serial.println("[ NTP ] Using DST");
  }
  else {
    if (config.getSystemDebug()) Serial.print("Timezone :");
    if (config.getSystemDebug()) Serial.println(tz);
    if (config.getSystemDebug()) Serial.println("[ NTP ] No DST");
  }
  ntp.Ntp(config.getNtpIp(), tz, 3600);   //use NTP Server, timeZone, update every x sec
  
  return true;
}

void ICACHE_FLASH_ATTR setWebEvents() {
  server.on("/index.htm", HTTP_GET, [](AsyncWebServerRequest * request) {
    AsyncWebServerResponse * response = request->beginResponse_P(200, "text/html", WEBSRC_INDEX_HTM, WEBSRC_INDEX_HTM_LEN);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest * request) {
    AsyncWebServerResponse * response = request->beginResponse_P(200, "text/javascript", WEBSRC_SCRIPT_JS, WEBSRC_SCRIPT_JS_LEN);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  server.on("/fonts/glyph.woff", HTTP_GET, [](AsyncWebServerRequest * request) {
    AsyncWebServerResponse * response = request->beginResponse_P(200, "font/woff", WEBSRC_GLYPH_WOFF, WEBSRC_GLYPH_WOFF_LEN);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  server.on("/fonts/glyph.woff2", HTTP_GET, [](AsyncWebServerRequest * request) {
    AsyncWebServerResponse * response = request->beginResponse_P(200, "font/woff", WEBSRC_GLYPH_WOFF2, WEBSRC_GLYPH_WOFF2_LEN);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  server.on("/required/required.css", HTTP_GET, [](AsyncWebServerRequest * request) {
    AsyncWebServerResponse * response = request->beginResponse_P(200, "text/css", WEBSRC_REQUIRED_CSS, WEBSRC_REQUIRED_CSS_LEN);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  server.on("/required/required.js", HTTP_GET, [](AsyncWebServerRequest * request) {
    AsyncWebServerResponse * response = request->beginResponse_P(200, "text/javascript", WEBSRC_REQUIRED_JS, WEBSRC_REQUIRED_JS_LEN);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  server.on("/status_charging.svg", HTTP_GET, [](AsyncWebServerRequest * request) {
    AsyncWebServerResponse * response = request->beginResponse_P(200, "image/svg+xml", WEBSRC_STATUS_CHARGING_SVG, WEBSRC_STATUS_CHARGING_SVG_LEN);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  server.on("/status_detected.svg", HTTP_GET, [](AsyncWebServerRequest * request) {
    AsyncWebServerResponse * response = request->beginResponse_P(200, "image/svg+xml", WEBSRC_STATUS_DETECTED_SVG, WEBSRC_STATUS_DETECTED_SVG_LEN);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  server.on("/status_ready.svg", HTTP_GET, [](AsyncWebServerRequest * request) {
    AsyncWebServerResponse * response = request->beginResponse_P(200, "image/svg+xml", WEBSRC_STATUS_READY_SVG, WEBSRC_STATUS_READY_SVG_LEN);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  //
  //  HTTP API
  //
  //getParameters
  if (config.getSystemApi()) {
    server.on("/getParameters", HTTP_GET, [](AsyncWebServerRequest * request) {
      AsyncResponseStream *response = request->beginResponseStream("application/json");
      StaticJsonDocument<500> jsonDoc;
      jsonDoc["type"] = "parameters";
      JsonArray list = jsonDoc.createNestedArray("list");
      JsonObject items = list.createNestedObject();
      items["vehicleState"] = evseStatus;
      items["evseState"] = evseActive;
      items["maxCurrent"] = maxCurrent;
      items["actualCurrent"] = evseAmpsConfig;
      items["actualPower"] =  float(int((currentKW + 0.005) * 100.0)) / 100.0;
      items["duration"] = getChargingTime();
      items["alwaysActive"] = config.getEvseAlwaysActive(0);
      items["lastActionUser"] = lastUsername;
      items["lastActionUID"] = lastUID;
      items["energy"] = float(int((meteredKWh + 0.005) * 100.0)) / 100.0;
      items["mileage"] = float(int(((meteredKWh * 100.0 / config.getEvseAvgConsumption(0)) + 0.05) * 10.0)) / 10.0;
      if (config.useMMeter) {
        items["meterReading"] = float(int((meterReading + 0.005) * 100.0)) / 100.0;
        items["currentP1"] = currentP1;
        items["currentP2"] = currentP2;
        items["currentP3"] = currentP3;
      }
      else {
        items["meterReading"] = float(int((startTotal + meteredKWh + 0.005) * 100.0)) / 100.0;;
        if (config.getMeterPhaseCount(0) == 1) {
          float fCurrent = float(int((currentKW / float(config.getMeterFactor(0)) / 0.227 + 0.005) * 100.0) / 100.0);
          if (config.getMeterFactor(0) == 1) {
            items["currentP1"] = fCurrent;
              items["currentP2"] = 0.0;
            items["currentP3"] = 0.0;
          }
          else if (config.getMeterFactor(0) == 2) {
            items["currentP1"] = fCurrent;
            items["currentP2"] = fCurrent;
            items["currentP3"] = 0.0;
          }
          else if (config.getMeterFactor(0) == 3) {
            items["currentP1"] = fCurrent;
            items["currentP2"] = fCurrent;
            items["currentP3"] = fCurrent;
          }
        }
        else {
          float fCurrent = float(int((currentKW / 0.227 / float(config.getMeterFactor(0)) / 3.0 + 0.005) * 100.0) / 100.0);
          items["currentP1"] = fCurrent;
          items["currentP2"] = fCurrent;
          items["currentP3"] = fCurrent;
        }
      }
      items["useMeter"] = config.getMeterActive(0);
      serializeJson(jsonDoc, *response);
      request->send(response);
    });

    //getLog 
    server.on("/getLog", HTTP_GET, [](AsyncWebServerRequest * request) {
      AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/latestlog.json", "application/json");
      request->send(response);
    }); 

    //setCurrent (0,233)
    server.on("/setCurrent", HTTP_GET, [](AsyncWebServerRequest * request) {
        awp = request->getParam(0);
        if (awp->name() == "current") {
          if ((atoi(awp->value().c_str()) <= config.getSystemMaxInstall() && atoi(awp->value().c_str()) >= 6) || 
            atoi(awp->value().c_str()) == 0) {
            currentToSet = atoi(awp->value().c_str());
            if (setEVSEcurrent()) {
              request->send(200, "text/plain", "S0_set current to given value");
            }
            else {
              request->send(200, "text/plain", "E0_could not set current - internal error");
            }
          }
          else {
            if (atoi(awp->value().c_str()) >= config.getSystemMaxInstall()) {
              currentToSet = config.getSystemMaxInstall();
              if (setEVSEcurrent()) {
                request->send(200, "text/plain", "S0_set current to maximum value");
              }
            }
            else {
              request->send(200, "text/plain", ("E1_could not set current - give a value between 6 and " + (String)config.getSystemMaxInstall()));
            }
          }
        }
        else {
          request->send(200, "text/plain", "E2_could not set current - wrong parameter");
        }
    });

    //setStatus
    server.on("/setStatus", HTTP_GET, [](AsyncWebServerRequest * request) {
      awp = request->getParam(0);
      if (awp->name() == "active" && config.getEvseAlwaysActive(0) == false) {
        if (config.getSystemDebug()) Serial.println(awp->value().c_str());
        if (strcmp(awp->value().c_str(), "true") == 0) {
          lastUID = "API";
          lastUsername = "API";
          if (!evseActive) {
            if (activateEVSE()) {
              request->send(200, "text/plain", "S0_EVSE successfully activated");
            }
            else {
              request->send(200, "text/plain", "E0_could not activate EVSE - internal error!");
            }
          }
          else {
            request->send(200, "text/plain", "E3_could not activate EVSE - EVSE already activated!");
          }
        }
        else if (strcmp(awp->value().c_str(), "false") == 0) {
          lastUID = "API";
          lastUsername = "API";
          if (evseActive) {
            toDeactivateEVSE = true;
            request->send(200, "text/plain", "S0_EVSE successfully deactivated");
          }
          else {
            request->send(200, "text/plain", "E3_could not deactivate EVSE - EVSE already deactivated!");
          }
        }
        else {
          request->send(200, "text/plain", "E1_could not process - give a valid value (true/false)");
        }
      }
      else {
        request->send(200, "text/plain", "E2_could not process - wrong parameter or EVSE-WiFi runs in always active mode");
      }
    });

    //interruptCp
    #ifndef ESP8266
    server.on("/interruptCp", HTTP_GET, [](AsyncWebServerRequest * request) {
      if (interruptCp()) {
        request->send(200, "text/plain", "S0_CP signal interrupted successfully");
      }
      else {
        request->send(200, "text/plain", "E0_Error while interrupting CP signal");
      }
    });
    #endif

    //evseHost
    server.on("/evseHost", HTTP_GET, [](AsyncWebServerRequest * request) {
      AsyncResponseStream *response = request->beginResponseStream("application/json");
      StaticJsonDocument<340> jsonDoc;
      jsonDoc["type"] = "evseHost";
      JsonArray list = jsonDoc.createNestedArray("list");
      JsonObject item = list.createNestedObject();

      #ifdef ESP8266
      struct ip_info info;
      if (inAPMode) {
        wifi_get_ip_info(SOFTAP_IF, &info);
        struct softap_config conf;
        wifi_softap_get_config(&conf);
        item["ssid"] = String(reinterpret_cast<char*>(conf.ssid));
        item["dns"] = printIP(WiFi.softAPIP());
        item["mac"] = WiFi.softAPmacAddress();
      }
      else {
        wifi_get_ip_info(STATION_IF, &info);
        struct station_config conf;
        wifi_station_get_config(&conf);
        item["ssid"] = String(reinterpret_cast<char*>(conf.ssid));
        item["rssi"] = String(WiFi.RSSI());
        item["dns"] = printIP(WiFi.dnsIP());
        item["mac"] = WiFi.macAddress();
      }
      #else
      wifi_config_t conf;
      tcpip_adapter_ip_info_t info;
      tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_ETH, &info);
      if (inAPMode) {
        esp_wifi_get_config(WIFI_IF_AP, &conf);
        item["ssid"] = String(reinterpret_cast<char*>(conf.ap.ssid));
        item["dns"] = printIP(WiFi.softAPIP());
        item["mac"] = WiFi.softAPmacAddress();
      }
      else {
        esp_wifi_get_config(WIFI_IF_STA, &conf);
        item["ssid"] = String(reinterpret_cast<char*>(conf.sta.ssid));
        item["rssi"] = String(WiFi.RSSI());
        item["dns"] = printIP(WiFi.dnsIP());
        item["mac"] = WiFi.macAddress();
      } 
      #endif

      IPAddress ipaddr = IPAddress(info.ip.addr);
      IPAddress gwaddr = IPAddress(info.gw.addr);
      IPAddress nmaddr = IPAddress(info.netmask.addr);
      item["ip"] = printIP(ipaddr);
      item["gateway"] = printIP(gwaddr);
      item["netmask"] = printIP(nmaddr);
      item["uptime"] = ntp.getUptimeSec();

      serializeJson(jsonDoc, *response);
      request->send(response);
    });

    //doReboot
    server.on("/doReboot", HTTP_GET, [](AsyncWebServerRequest * request) {
      awp = request->getParam(0);
      if (awp->name() == "reboot") {
        if (strcmp(awp->value().c_str(), "true") == 0) {
          toReboot = true;
          request->send(200, "text/plain", "S0_EVSE-WiFi is going to reboot now...");
        }
        else {
          request->send(200, "text/plain", "E1_could not do reboot - wrong value");
        }
      }
      else {
        request->send(200, "text/plain", "E2_could not do reboot - wrong parameter");
      }
    });

    //setRegister (0,233)
    server.on("/setRegister", HTTP_GET, [](AsyncWebServerRequest * request) {
      awp = request->getParam(0);
      awp2 = request->getParam(1);
      if (awp->name() == "reg") {
        if ((atoi(awp->value().c_str()) >= 1000 && atoi(awp->value().c_str()) <= 1007) || 
            (atoi(awp->value().c_str()) >= 2000 && atoi(awp->value().c_str()) <= 2017)) {
          if (awp2->name() == "val") {
            if (atoi(awp2->value().c_str()) >= 0 && atoi(awp2->value().c_str()) <= 65535) {
              uint16_t reg = atoi(awp->value().c_str());
              uint16_t val = atoi(awp2->value().c_str());
              if (setEVSERegister(reg, val)){
                request->send(200, "text/plain", "S0_EVSE Register successfully set");
              }
              else {
                request->send(200, "text/plain", "E0_could not set EVSE register - internal error");
              }
            }
            else {
              request->send(200, "text/plain", "E0_could not set EVSE register - invalid value");
            }
          }
          else {
            request->send(200, "text/plain", "E1_could not set EVSE register - invalid parameter");
          }
        }
        else {
          request->send(200, "text/plain", "E0_could not set EVSE register - invalid register");
        }
      }
      else {
        request->send(200, "text/plain", "E1_could not set EVSE register - invalid parameter");
      }
    });
  }
}

void ICACHE_FLASH_ATTR fallbacktoAPMode() {
  if (config.getSystemDebug()) Serial.println(F("[ INFO ] EVSE-WiFi is running in Fallback AP Mode"));
  WiFi.disconnect(true);
  if (startAP("EVSE-WiFi-Fallback")) {
    Serial.println("[ SYSTEM ] Fallback Mode set successfully!");
    inFallbackMode = true;
  }
  else {
    Serial.println("[ SYSTEM ] Fallback mode failed!");
  }
}

void ICACHE_FLASH_ATTR startWebserver() {
  // Start WebSocket Plug-in and handle incoming message on "onWsEvent" function
  server.addHandler(&ws);
  ws.onEvent(onWsEvent);
  server.onNotFound([](AsyncWebServerRequest * request) {
    AsyncWebServerResponse *response = request->beginResponse(404, "text/plain", "Not found");
    request->send(response);
  });

  // Simple Firmware Update Handler
  server.on("/update", HTTP_POST, [](AsyncWebServerRequest * request) {
    toReboot = !Update.hasError();
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", toReboot ? "OK" : "FAIL");
    response->addHeader("Connection", "close");
    request->send(response);
  }, [](AsyncWebServerRequest * request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    if (!index) {
      if (config.getSystemDebug()) Serial.printf("[ UPDT ] Firmware update started: %s\n", filename.c_str());
      updateRunning = true;
      #ifdef ESP8266
      Update.runAsync(true);
      #endif
      if (!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000)) {
        Update.printError(Serial);
      }
    }
    if (!Update.hasError()) {
      if (Update.write(data, len) != len) {
        Update.printError(Serial);
      }
    }
    if (final) {
      if (Update.end(true)) {
        if (config.getSystemDebug()) Serial.printf("[ UPDT ] Firmware update finished: %uB\n", index + len);
      } else {
        Update.printError(Serial);
      }
    }
  });

  setWebEvents();

  // HTTP basic authentication
  server.on("/login", HTTP_GET, [](AsyncWebServerRequest * request) {
      if (!request->authenticate("admin", config.getSystemPass())) {
        return request->requestAuthentication();
      }
      request->send(200, "text/plain", "Success");
  });

  server.rewrite("/", "/index.htm");
  server.begin();
}

//////////////////////////////////////////////////////////////////////////////////////////
///////       Setup
//////////////////////////////////////////////////////////////////////////////////////////
void ICACHE_RAM_ATTR setup() {
  Serial.begin(9600);
  if (config.getSystemDebug()) Serial.println();
  if (config.getSystemDebug()) Serial.println("[ INFO ] SimpleEVSE WiFi");
  delay(500);

  SPI.begin();
  SPIFFS.begin();
  #ifdef ESP8266
  SoftSer.begin(9600);
  meterNode.begin(2, Serial);
  #else
  SoftSer.begin(9600, SERIAL_8N1, 22, 21);
  meterNode.begin(2, SoftSer);
  #endif
  
  evseNode.begin(1, SoftSer);

  if (!loadConfiguration()) {
    Serial.println("[ WARNING ] Going to fallback mode!");
    fallbacktoAPMode();
  }

  // Setup LED
  if (config.getEvseLedConfig(0) != 1) {
    pinMode(config.getEvseLedPin(0), OUTPUT);
    changeLedTimes(100, 10000); // Heartbeat by default
    if (config.getSystemDebug()) Serial.println("[ System ] LED pin set");
  }

  //Activate the button pin with pullup in any setup to prevent bouncing pin state
    pinMode(config.getButtonPin(0), INPUT_PULLUP);
    if (config.getSystemDebug()) Serial.print("[ System ] Button Active with Internal Pullup on Pin: ");
    if (config.getSystemDebug()) Serial.println(config.getButtonPin(0));

  //Factory Reset when button pressed for 20 sec after boot
  if (digitalRead(config.getButtonPin(0)) == LOW) {
    pinMode(config.getEvseLedPin(0), OUTPUT);
    if (config.getSystemDebug()) Serial.println("Button Pressed while boot!");
    digitalWrite(config.getEvseLedPin(0), HIGH);
    unsigned long millisBefore = millis();
    int button = config.getButtonPin(0);
    while (digitalRead(button) == LOW) {  
      if (millis() > (millisBefore + 20000)) {
        factoryReset();
        Serial.println("[ SYSTEM ] System has been reset to factory settings!");
        digitalWrite(config.getEvseLedPin(0), LOW);
      }
      delay(1000);
      if (config.getSystemDebug()) Serial.println("Button is pressed...");
    }
  }

  if (config.useSMeter) {
    pinMode(config.getMeterPin(0), INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(config.getMeterPin(0)), handleMeterInt, FALLING);
    if (config.getSystemDebug()) Serial.print("[ Meter ] Use GPIO/Pin ");
    if (config.getSystemDebug()) Serial.println(config.getMeterPin(0));
    startTotal = getS0MeterReading();
  }

#ifndef ESP8266
  pinMode(config.getEvseRsePin(0), INPUT_PULLUP);
  if (config.getSystemDebug()) Serial.print("[ SYSTEM ] Use RSE GPIO ");
  if (config.getSystemDebug()) Serial.println(config.getEvseRsePin(0));

  pinMode(config.getEvseCpIntPin(0), OUTPUT);
  if (config.getSystemDebug()) Serial.println("[ SYSTEM ] OLED started");
  oled.begin(&u8g2, config.getEvseDisplayRotation(0));
  delay(100);
#endif

  now();
  startWebserver();
  if (config.getSystemDebug()) Serial.println("[ SYSTEM ] End of setup routine");
  if (config.getEvseRemote(0)) sliderStatus = false;

  setModbusTCPRegisters();
}

//////////////////////////////////////////////////////////////////////////////////////////
///////       Loop
//////////////////////////////////////////////////////////////////////////////////////////
void ICACHE_RAM_ATTR loop() {
  currentMillis = millis();
  unsigned long uptime = ntp.getUptimeSec();
  previousLoopMillis = currentMillis;
  changeLedStatus();

  //Reboot after 10 minutes in Fallback
  if (inFallbackMode && millis() > 600000) toReboot = true;

  if (uptime > 3888000) {   // auto restart after 45 days uptime
    if (vehicleCharging == false) {
      if (config.getSystemDebug()) Serial.println(F("[ UPDT ] Auto restarting..."));
      delay(1000);
      toReboot = true;
    }
  }
  if (toReboot) {
    if (config.getSystemDebug()) Serial.println(F("[ UPDT ] Rebooting..."));
    delay(100);
    ESP.restart();
  }
  if (currentMillis >= rfid.cooldown && config.getRfidActive() == true && !updateRunning) {
    rfidloop();
  }

  handleLed();

  modbusTCPServerNode.task();
  //delay(10);

  if (toActivateEVSE && !updateRunning) {
    activateEVSE();
    delay(300);
  }
  if (toDeactivateEVSE && !updateRunning) {
    deactivateEVSE(true);
    delay(300);
  }

  if (currentMillis > (lastModbusAction + 3000) && !updateRunning) { //Update Modbus data every 3000ms and send data to WebUI
    queryEVSE();
    if (evseSessionTimeOut == false) {
      sendEVSEdata();
    }
  }
  else if (currentMillis > evseQueryTimeOut &&    //Setting timeout for Evse poll / push to ws
    evseSessionTimeOut == false && !updateRunning) {
    evseSessionTimeOut = true;
    pushSessionTimeOut();
  }
  if (config.useMMeter && millisUpdateMMeter < millis() && !updateRunning) {
    updateMMeterData();
  }
  if (meterInterrupt != 0) {
    updateS0MeterData();
  }
  if (config.useSMeter && previousMeterMillis < millis() - (meterTimeout * 1000)) {  //Timeout when there is less than ~300 watt power consuption -> 10 sec of no interrupt from meter
    if (previousMeterMillis != 0) {
      currentKW = 0.0;
    }
  }
  if (toSetEVSEcurrent && !updateRunning) {
    setEVSEcurrent();
  }

  if (wifiInterrupted && reconnectTimer < millis()) {
    reconnectTimer = millis() + 30000; // 30 seconds
    reconnectWiFi();
  }

  if (!inAPMode && (WiFi.status() != WL_CONNECTED)) {
    wifiInterrupted = true;
  }
  else {
    if (wifiInterrupted) {
      if (config.getSystemDebug()) Serial.println("[ INFO ] WiFi connection successfully reconnected");
    }
    wifiInterrupted = false;
  }

  if (config.getButtonActive(0) && digitalRead(config.getButtonPin(0)) == HIGH && buttonState == LOW) {
    delay(100);
    if (digitalRead(config.getButtonPin(0)) == HIGH) {
      if (config.getSystemDebug()) Serial.println("Button released");
      buttonState = HIGH;
      if (!config.getEvseAlwaysActive(0)) {
        if (evseActive) {
          toDeactivateEVSE = true;
        }
        else {
          toActivateEVSE = true;
        }
        lastUsername = "Button";
        lastUID = "Button";
      }
    }
  }

  int buttonPin;
  if (inFallbackMode) {
    #ifdef ESP8266
    buttonPin = D4;
    #else
    buttonPin = 16;
    #endif
  }
  else {
    buttonPin = config.getButtonPin(0);
  }

  if (digitalRead(buttonPin) != buttonState) {
    delay(70);
      if (digitalRead(buttonPin) != buttonState) {
      buttonState = digitalRead(buttonPin);
      buttonTimer = millis();
      if (config.getSystemDebug()) Serial.println("Button pressed...");
    }
  }
  if (digitalRead(buttonPin) == LOW && (millis() - buttonTimer) > 10000) { //Reboot
    if (config.getSystemDebug()) Serial.println("Button Pressed > 10 sec -> Reboot");
    toReboot = true;
  }
  if (toSendStatus == true) {
    sendStatus();
    toSendStatus = false;
  }

#ifndef ESP8266
  if (millisUpdateOled < millis()) {
    delay(5);
    oled.showDemo(evseStatus, getChargingTime(), evseAmpsConfig, maxCurrent, currentKW, meteredKWh, ntp.getUtcTimeNow(), &swVersion);
    millisUpdateOled = millis() + 3000;
  }

  if (doCpInterruptCp) {
    if (millis() > millisInterruptCp) {
      doCpInterruptCp = false;
      digitalWrite(config.getEvseCpIntPin(0), LOW);
      Serial.println("Interrupt CP stopped");
    }
  }

  if (config.getEvseRseActive(0)) {
    if (digitalRead(config.getEvseRsePin(0)) == LOW && rseActive == false) {
      Serial.println("RSE Activate");
      rseActive = true;
      handleRse();
    }
    else if (digitalRead(config.getEvseRsePin(0)) == HIGH && rseActive == true) {
      rseActive = false;
      Serial.println("RSE Deactivate");
      handleRse();
    }
  }
#endif
}
