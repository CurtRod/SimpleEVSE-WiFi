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
#include <ESP8266WiFi.h>              // Whole thing is about using Wi-Fi networks
#include <ESP8266mDNS.h>              // Zero-config Library (Bonjour, Avahi)
#include <ESPAsyncTCP.h>              // Async TCP Library is mandatory for Async Web Server
#include <SPI.h>                      // RFID MFRC522 Module uses SPI protocol
#include <MFRC522.h>                  // Library for Mifare RC522 Devices
#include <ArduinoJson.h>              // JSON Library for Encoding and Parsing Json object to send browser
#include <FS.h>                       // SPIFFS Library for storing web files to serve to web browsers
#include <ESPAsyncWebServer.h>        // Async Web Server with built-in WebSocket Plug-in
#include <SPIFFSEditor.h>             // This creates a web page on server which can be used to edit text based files
#include <TimeLib.h>                  // Library for converting epochtime to a date
#include <WiFiUdp.h>                  // Library for manipulating UDP packets which is used by NTP Client to get Timestamps
#include <SoftwareSerial.h>           // Using GPIOs for Serial Modbus communication
#include <ModbusMaster.h>

#include <string.h>
#include "proto.h"
#include "ntp.h"
#include "websrc.h"
#include "config.h"
#include "rfid.h"

#ifdef ESP8266
extern "C" {
#include "user_interface.h"  // Used to get Wifi status information
}
#endif

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
AsyncWebParameter* awp;
const char * initLog = "{\"type\":\"latestlog\",\"list\":[]}";

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

//objects and instances
SoftwareSerial sSerial(D1, D2); //SoftwareSerial object (RX, TX)
ModbusMaster evseNode;
ModbusMaster meterNode;
AsyncWebServer server(80);    // Create AsyncWebServer instance on port "80"
AsyncWebSocket ws("/ws");     // Create WebSocket instance on URL "/ws"
NtpClient ntp;
EvseWiFiConfig config = EvseWiFiConfig();
//MFRC522 mfrc522 = MFRC522();  // Create MFRC522 RFID instance
EvseWiFiRfid rfid;

unsigned long lastModbusAction = 0;
unsigned long evseQueryTimeOut = 0;
unsigned long buttonTimer = 0;

//Loop
unsigned long previousMillis = 0;
unsigned long previousLoopMillis = 0;
//unsigned long cooldown = 0;
unsigned long previousLedAction = 0;
uint16_t toChangeLedOnTime = 0;
uint16_t toChangeLedOffTime = 0;
uint16_t ledOnTime = 1000;
uint16_t ledOffTime = 2000;
bool ledStatus = false;
bool toSetEVSEcurrent = false;
bool toActivateEVSE = false;
bool toDeactivateEVSE = false;
bool toSendStatus = false;
bool toReboot = false;

//EVSE Modbus Registers
uint16_t evseAmpsConfig;     //Register 1000
uint16_t evseAmpsOutput;     //Register 1001
uint16_t evseVehicleStatus;  //Register 1002
uint16_t evseAmpsPP;         //Register 1003
uint16_t evseTurnOff;        //Register 1004
uint16_t evseFirmware;       //Register 1005
uint16_t evseState;          //Register 1006

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
uint8_t ledPin = D0;
uint8_t maxCurrent = 0;

//Others
String msg = ""; //WS communication

//////////////////////////////////////////////////////////////////////////////////////////
///////       Auxiliary Functions
//////////////////////////////////////////////////////////////////////////////////////////

void ICACHE_FLASH_ATTR doChangeLedTimes() {
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

String ICACHE_FLASH_ATTR printIP(IPAddress adress) {
  return (String)adress[0] + "." + (String)adress[1] + "." + (String)adress[2] + "." + (String)adress[3];
}

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
    meterImpMillis = millis();
    meterInterrupt ++;
    numberOfMeterImps ++;
  }
}

void ICACHE_RAM_ATTR updateS0MeterData() {
  if (vehicleCharging) {
    currentKW = 3600.0 / float(meterImpMillis - previousMeterMillis) / float(config.getMeterImpKwh(0) / 1000.0) * (float)config.getMeterFactor(0) ;  //Calculating kW
    previousMeterMillis = meterImpMillis;
    meterImpMillis = meterImpMillis + (config.getMeterImpLen(0) + 3);
    meterInterrupt = 0;
    meteredKWh = float(numberOfMeterImps) / float(config.getMeterImpKwh(0) / 1000.0) / 1000.0 * float(config.getMeterFactor(0));
  }
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
  const int regsToRead = 6;
  uint8_t result;
  uint16_t iaRes[regsToRead];
  meterNode.clearTransmitBuffer();
  meterNode.clearResponseBuffer();
  delay(50);
  result = meterNode.readInputRegisters(0x0006, regsToRead); // read 6 registers starting at 0x0000

  if (config.getSystemDebug()) Serial.println("");
  if (result != 0) {
    Serial.print("[ ModBus ] Error ");
    Serial.print(result, HEX);
    Serial.println(" occured while getting current Meter Data");
    changeLedTimes(300, 300);
    return;
  }

  for (int i = 0; i < regsToRead; i++) {
    iaRes[i] = meterNode.getResponseBuffer(i);
  }

  ((uint16_t*)&currentP1)[1]= iaRes[0];
  ((uint16_t*)&currentP1)[0]= iaRes[1];
  ((uint16_t*)&currentP2)[1]= iaRes[2];
  ((uint16_t*)&currentP2)[0]= iaRes[3];
  ((uint16_t*)&currentP3)[1]= iaRes[4];
  ((uint16_t*)&currentP3)[0]= iaRes[5];
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
  Dir userdir = SPIFFS.openDir("/P/");
  while(userdir.next()){
    Serial.println(userdir.fileName());
    SPIFFS.remove(userdir.fileName());
  }
  return true;
}

bool ICACHE_FLASH_ATTR factoryReset() {
  if (config.getSystemDebug()) Serial.println("[ SYSTEM ] Factory Reset...");
  SPIFFS.remove("/config.json");
  SPIFFS.remove("/latestlog.json");
  if (resetUserData()) {
    if (config.getSystemDebug()) Serial.println("[ SYSTEM ] ...successfully done - going to reboot");
  }
  toReboot = true;
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
      if (scan.valid) { // PICC valid
        Serial.println("PICC valid");
        if (evseActive) {
          toDeactivateEVSE = true;
        }
        else {
          toActivateEVSE = true;
        }
      }
      jsonDoc["known"] = 1;
      jsonDoc["user"] = scan.user;
    }
    else { // Unknown PICC
      lastUsername = "Unknown";
      jsonDoc["known"] = 0;
    }

    size_t len = measureJson(jsonDoc);
    AsyncWebSocketMessageBuffer * buffer = ws.makeBuffer(len);
    if (buffer) {
      serializeJson(jsonDoc, (char *)buffer->get(), len + 1);
      ws.textAll(buffer);
    }
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
    evseVehicleStatus = 0;
    Serial.print("[ ModBus ] Error ");
    Serial.print(result, HEX);
    Serial.println(" occured while getting additional EVSE data");
    changeLedTimes(300, 300);
    lastModbusAction = millis();
    return addEvseData;
  }
  else {
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
  }
  return addEvseData;
}

void ICACHE_FLASH_ATTR sendStatus() {
  // Getting additional Modbus data
  struct ip_info info;
  FSInfo fsinfo;
  if (!SPIFFS.info(fsinfo)) {
    Serial.print(F("[ WARN ] Error getting info on SPIFFS"));
  }
  StaticJsonDocument<1000> jsonDoc;
  jsonDoc["command"] = "status";
  jsonDoc["heap"] = ESP.getFreeHeap();
  jsonDoc["chipid"] = String(ESP.getChipId(), HEX);
  jsonDoc["cpu"] = ESP.getCpuFreqMHz();
  jsonDoc["availsize"] = ESP.getFreeSketchSpace();
  jsonDoc["availspiffs"] = fsinfo.totalBytes - fsinfo.usedBytes;
  jsonDoc["spiffssize"] = fsinfo.totalBytes;
  jsonDoc["uptime"] = ntp.getDeviceUptimeString();

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

  s_addEvseData addEvseData = getAdditionalEVSEData();
  IPAddress ipaddr = IPAddress(info.ip.addr);
  IPAddress gwaddr = IPAddress(info.gw.addr);
  IPAddress nmaddr = IPAddress(info.netmask.addr);
  jsonDoc["ip"] = printIP(ipaddr);
  jsonDoc["gateway"] = printIP(gwaddr);
  jsonDoc["netmask"] = printIP(nmaddr);
  jsonDoc["evse_amps_conf"] = evseAmpsConfig;          //Reg 1000
  jsonDoc["evse_amps_out"] = evseAmpsOutput;           //Reg 1001
  jsonDoc["evse_vehicle_state"] = evseVehicleStatus;   //Reg 1002
  jsonDoc["evse_pp_limit"] = evseAmpsPP;               //Reg 1003
  jsonDoc["evse_turn_off"] = evseTurnOff;              //Reg 1004
  jsonDoc["evse_firmware"] = evseFirmware;             //Reg 1005
  jsonDoc["evse_state"] = evseState;                   //Reg 1006
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
    item["hidden"] = WiFi.isHidden(i) ? true : false;
  }
  size_t len = measureJson(jsonDoc);
  serializeJson(jsonDoc, Serial);
  //Serial.print("Länge: ");
  //Serial.println(len);
  //delay(100);
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
    Serial.print("Logfile Size: ");
    Serial.println(size);

    std::unique_ptr<char[]> buf (new char[size]);
    logFile.readBytes(buf.get(), size);
    
    DynamicJsonDocument jsonDoc2(12000);
    DeserializationError error = deserializeJson(jsonDoc2, buf.get());
    JsonArray list = jsonDoc2["list"];
    if (error) {
      if (config.getSystemDebug()) Serial.println("Impossible to read JSON file");
    }
    else {
      logFile.close();
      if (list.size() >= 100) {
        list.remove(0);
      }
      File logFile = SPIFFS.open("/latestlog.json", "w");
      StaticJsonDocument<200> jsonDoc3;
      jsonDoc3["uid"] = uid;
      jsonDoc3["username"] = username;
      jsonDoc3["timestamp"] = ntp.getUtcTimeNow();
      jsonDoc3["duration"] = 0;
      jsonDoc3["energy"] = 0;
      jsonDoc3["price"] = config.getMeterEnergyPrice(0);
      list.add(jsonDoc3);
      serializeJson(jsonDoc2, logFile);
    }
    logFile.close();
  }
  else {
    if (config.getSystemDebug()) Serial.println("Cannot create Logfile");
  }
}

void ICACHE_FLASH_ATTR updateLog(bool e) {
  File logFile = SPIFFS.open("/latestlog.json", "r");
  size_t size = logFile.size();
  std::unique_ptr<char[]> buf (new char[size]);
  logFile.readBytes(buf.get(), size);
  DynamicJsonDocument jsonDoc(12000);
  DeserializationError error = deserializeJson(jsonDoc, buf.get());
  JsonArray list = jsonDoc["list"];
  if (error) {
    if (config.getSystemDebug()) Serial.println("Impossible to read JSON file");
      Serial.print("Impossible to read Log file: ");
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
      serializeJson(jsonDoc, logFile);
    }
  }
  logFile.close();
  millisStartCharging = 0;
  millisStopCharging = 0;
  meteredKWh = 0.0;
  currentKW = 0.0;
}

float ICACHE_FLASH_ATTR getS0MeterReading() {
  float fMeterReading = 0.0;
  File logFile = SPIFFS.open("/latestlog.json", "r");

  if (logFile) {
    size_t size = logFile.size();
    std::unique_ptr<char[]> buf (new char[size]);
    logFile.readBytes(buf.get(), size);
    DynamicJsonDocument jsonDoc(3000);
    DeserializationError error = deserializeJson(jsonDoc, buf.get());
    JsonArray list = jsonDoc["list"];
    if (error) {
      if (config.getSystemDebug()) Serial.println("Impossible to read Log file");
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
  return fMeterReading;
}

bool ICACHE_FLASH_ATTR initLogFile() {
  if (config.getSystemDebug())Serial.println("[ SYSTEM ] Going to delete Log File...");
  File logFile = SPIFFS.open("/latestlog.json", "w");
  if (logFile) {
    StaticJsonDocument<35> jsonDoc;
    jsonDoc["type"] = "latestlog";
    jsonDoc.createNestedArray("list");
    serializeJson(jsonDoc, logFile);
    logFile.close();
    if (config.getSystemDebug())Serial.println("[ SYSTEM ] ... Success!");
  }
  else {
    if (config.getSystemDebug())Serial.println("[ SYSTEM ] ... Failure!");
    return false;
  }
  return true;
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
  result = meterNode.readInputRegisters(reg, 2);  // read 7 registers starting at 0x0000
  
  if (config.getSystemDebug()) Serial.println("");
  if (result != 0) {
    Serial.print("[ ModBus ] Error ");
    Serial.print(result, HEX);
    Serial.println(" occured while getting Meter Data");
    changeLedTimes(300, 300);
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

  if (result != 0) {
    evseVehicleStatus = 0;
    evseStatus = 0;
    Serial.print("[ ModBus ] Error ");
    Serial.print(result, HEX);
    Serial.println(" occured while getting EVSE data - trying again...");
    evseNode.clearTransmitBuffer();
    evseNode.clearResponseBuffer();
    delay(500);
    changeLedTimes(300, 300);
    lastModbusAction = millis();
    return false;
  }

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
      evseVehicleStatus = evseNode.getResponseBuffer(i);   //Register 1002
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
      evseState = evseNode.getResponseBuffer(i);      //Register 1006
      break;
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
    if (evseVehicleStatus == 0) {
      evseStatus = 0; //modbus communication failed
      changeLedTimes(300, 300);
    }
    if (evseState == 3) {     //EVSE not Ready
      if (evseVehicleStatus == 2 ||
          evseVehicleStatus == 3 ||
          evseVehicleStatus == 4) {
        evseStatus = 2; //vehicle detected
        changeLedTimes(300, 1000);
      }
      else {
        evseStatus = 1; // EVSE deactivated
        changeLedTimes(1000, 3000);
      }
      if (vehicleCharging == true && manualStop == false) {   //vehicle interrupted charging
        millisStopCharging = millis();
        vehicleCharging = false;
        if (config.getSystemDebug()) Serial.println("Vehicle interrupted charging");
        updateLog(false);
      }
      evseActive = false;
      return true;
    }

    if (evseVehicleStatus == 1) {
      evseStatus = 1;  // ready
      changeLedTimes(1000, 1000);
    }
    else if (evseVehicleStatus == 2) {
      evseStatus = 2; //vehicle detected
      changeLedTimes(300, 1000);
    }
    else if (evseVehicleStatus == 3 || evseVehicleStatus == 4) {
      evseStatus = 3; //charging
      changeLedTimes(2000, 1000);
    }
  }
  
  // Always Active Mode
  else {
    if (evseVehicleStatus == 5) {
      evseStatus = 5;
    }
    if (evseState == 1) { // Steady 12V
      if (vehicleCharging) { // EV interrupted charging
        millisStopCharging = millis();
        vehicleCharging = false;
        toDeactivateEVSE = true;
        lastUID = "vehicle";
        lastUsername = "vehicle";
        if (config.getSystemDebug()) Serial.println("Vehicle interrupted charging");
      }
      evseStatus = 1; // ready
      evseActive = true;
      changeLedTimes(1000, 1000);
    }
    else if (evseState == 2) { // PWM is being generated
      if (evseVehicleStatus == 2) { // EV is present
        if (vehicleCharging) {  // EV interrupted charging
          millisStopCharging = millis();
          vehicleCharging = false;
          toDeactivateEVSE = true;
          lastUID = "vehicle";
          lastUsername = "vehicle";
          if (config.getSystemDebug()) Serial.println("Vehicle interrupted charging");
        }
        evseStatus = 2; //vehicle detected
        evseActive = true;
        changeLedTimes(300, 1000);
      }
      else if (evseVehicleStatus == 3 || evseVehicleStatus == 4) {  // EV is charging
        if (!vehicleCharging) { // EV starts charging
          millisStartCharging = millis();
          vehicleCharging = true;
          toActivateEVSE = true;
          lastUID = "vehicle";
          lastUsername = "vehicle";
        }
        evseStatus = 3; //charging
        evseActive = true;
        changeLedTimes(2000, 1000);
      }
    }
    else if (evseState == 3) {     //EVSE not Ready
      if (evseVehicleStatus == 2 ||
          evseVehicleStatus == 3 ||
          evseVehicleStatus == 4) {
        evseStatus = 2; //vehicle detected
        changeLedTimes(300, 1000);
      }
      else {
        evseStatus = 1; // EVSE deactivated
        changeLedTimes(1000, 3000);
      }
      if (vehicleCharging == true && manualStop == false) {   //vehicle interrupted charging
        millisStopCharging = millis();
        vehicleCharging = false;
        lastUID = "vehicle";
        lastUsername = "vehicle";
        if (config.getSystemDebug()) Serial.println("Vehicle interrupted charging");
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
    //if (config.getSystemDebug()) Serial.println("[ ModBus ] Query EVSE before activating");
    //queryEVSE();

    if (evseState == 3 &&
      evseVehicleStatus != 0) {    //no modbus error occured
      iTransmit = 8192;         // disable EVSE after charge

      uint8_t result;
      evseNode.clearTransmitBuffer();
      evseNode.setTransmitBuffer(0, iTransmit); // set word 0 of TX buffer (bits 15..0)
      result = evseNode.writeMultipleRegisters(0x07D5, 1);  // write register 0x07D5 (2005)

      if (result != 0) {
        // error occured
        Serial.print("[ ModBus ] Error ");
        Serial.print(result, HEX);
        Serial.println(" occured while activating EVSE - trying again...");
        changeLedTimes(300, 300);
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
  if (config.useMMeter) {
    delay(20);
    if (config.mMeterTypeSDM120) {
      startTotal = readMeter(0x0156);
    }
    else if (config.mMeterTypeSDM630) {
      startTotal = readMeter(0x0156);
    }
    meteredKWh = 0.0;
  }
  else {
    startTotal = getS0MeterReading();
    meteredKWh = 0.0;
  }
  numberOfMeterImps = 0;
  sendEVSEdata();
  return true;
}

bool ICACHE_FLASH_ATTR deactivateEVSE(bool logUpdate) {
  if (!config.getEvseAlwaysActive(0)) {
    //New ModBus Master Library
    static uint16_t iTransmit = 16384;  // deactivate evse
    uint8_t result;

    evseNode.clearTransmitBuffer();
    evseNode.setTransmitBuffer(0, iTransmit); // set word 0 of TX buffer (bits 15..0)
    result = evseNode.writeMultipleRegisters(0x07D5, 1);  // write register 0x07D5 (2005)

    if (result != 0) {
      // error occured
      Serial.print("[ ModBus ] Error ");
      Serial.print(result, HEX);
      Serial.println(" occured while deactivating EVSE - trying again...");
      changeLedTimes(300, 300);
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
    if (config.mMeterTypeSDM120) {
      meteredKWh = readMeter(0x0156) - startTotal;
    }
    else if (config.mMeterTypeSDM630) {
      meteredKWh = readMeter(0x0156) - startTotal;
    }
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
  //delay(500);
  //queryEVSE();
  sendEVSEdata();
  return true;
}

bool ICACHE_FLASH_ATTR setEVSEcurrent() {  // telegram 1: write EVSE current
  //New ModBus Master Library
  uint8_t result;

  evseNode.clearTransmitBuffer();
  evseNode.setTransmitBuffer(0, currentToSet); // set word 0 of TX buffer (bits 15..0)
  result = evseNode.writeMultipleRegisters(0x03E8, 1);  // write register 0x03E8 (1000 - Actual configured amps value)

  if (result != 0) {
    // error occured
    Serial.print("[ ModBus ] Error ");
    Serial.print(result, HEX);
    Serial.println(" occured while setting current in EVSE - trying again...");
    changeLedTimes(300, 300);
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
    changeLedTimes(300, 300);
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
  AsyncWebSocketMessageBuffer * buffer = ws.makeBuffer(len); //  creates a buffer (len + 1) for you.
  if (buffer) {
    serializeJson(jsonDoc, (char *)buffer->get(), len + 1);
    ws.textAll(buffer);
  }
  if (config.getSystemDebug()) Serial.println("[ WebSocket ] TimeOut sent to browser!");
}

void ICACHE_FLASH_ATTR sendEVSEdata() {
  if (evseSessionTimeOut == false) {
    StaticJsonDocument<420> jsonDoc;
    jsonDoc["command"] = "getevsedata";
    jsonDoc["evse_vehicle_state"] = evseStatus;
    jsonDoc["evse_active"] = evseActive;
    jsonDoc["evse_current_limit"] = evseAmpsConfig;
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

void ICACHE_FLASH_ATTR sendUserList(int page, AsyncWebSocketClient * client) {
  DynamicJsonDocument jsonDoc(3000);
  jsonDoc["command"] = "userlist";
  jsonDoc["page"] = page;
  JsonArray users = jsonDoc.createNestedArray("list");
  Dir dir = SPIFFS.openDir("/P/");
  int first = (page - 1) * 15;
  int last = page * 15;
  int i = 0;
  while (dir.next()) {
    if (i >= first && i < last) {
      JsonObject item = users.createNestedObject();
      String uid = dir.fileName();
      uid.remove(0, 3);
      item["uid"] = uid;
      File f = SPIFFS.open(dir.fileName(), "r");
      size_t size = f.size();
      // Allocate a buffer to store contents of the file.
      std::unique_ptr<char[]> buf(new char[size]);
      f.readBytes(buf.get(), size);
      StaticJsonDocument<200> jsonDoc2;
      DeserializationError error = deserializeJson(jsonDoc2, buf.get());
      if (!error) {
        String username = jsonDoc2["user"];
        int AccType = jsonDoc2["acctype"];
        unsigned long validuntil = jsonDoc2["validuntil"];
        item["username"] = username;
        item["acctype"] = AccType;
        item["validuntil"] = validuntil;
      }
    }
    i++;
  }
  float pages = i / 15.0;
  jsonDoc["haspages"] = ceil(pages);
  size_t len = measureJson(jsonDoc);
  AsyncWebSocketMessageBuffer * buffer = ws.makeBuffer(len); //  creates a buffer (len + 1) for you.
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

void ICACHE_FLASH_ATTR onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_ERROR) {
    if (config.getSystemDebug()) Serial.printf("[ WARN ] WebSocket[%s][%u] error(%u): %s\r\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
  }
  else if (type == WS_EVT_DATA) {
    AwsFrameInfo * info = (AwsFrameInfo*)arg;
    if (info->final && info->index == 0 && info->len == len) {
      //the whole message is in a single frame and we got all of it's data
      if (config.getSystemDebug())Serial.println("[ Websocket ] single Frame - all data is here!");
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
        //root.prettyPrintTo(Serial);
        processWsEvent(jsonDoc, client);
      }
    }
  }
}

void ICACHE_FLASH_ATTR processWsEvent(JsonDocument& root, AsyncWebSocketClient * client) {
  const char * command = root["command"];
  File configFile;
  if (strcmp(command, "remove") == 0) {
    const char* uid = root["uid"];
    String filename = "/P/";
    filename += uid;
    SPIFFS.remove(filename);
  }
  else if (strcmp(command, "configfile") == 0) {
    if (config.getSystemDebug()) Serial.println("[ SYSTEM ] Try to update config.json...");
    String configString;
    serializeJson(root, configString);
    //Serial.println(configString);
    if (config.updateConfig(configString)) {
      if (config.getSystemDebug()) Serial.println("[ SYSTEM ] Success - going to reboot now");
      if (vehicleCharging) {
        deactivateEVSE(true);
        delay(100);
      }
      ESP.reset();
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
    File userFile = SPIFFS.open(filename, "w+");
    // Check if we created the file
    if (userFile) {
      userFile.print(msg);
      if (config.getSystemDebug()) Serial.println("[ DEBUG ] Userfile written!");
    }
    userFile.close();
    ws.textAll("{\"command\":\"result\",\"resultof\":\"userfile\",\"result\": true}");
  }
  else if (strcmp(command, "latestlog") == 0) {
    File logFile = SPIFFS.open("/latestlog.json", "r");
    if (logFile) {
      size_t len = logFile.size();
      AsyncWebSocketMessageBuffer * buffer = ws.makeBuffer(len); //  creates a buffer (len + 1) for you.
      if (buffer) {
        logFile.readBytes((char *)buffer->get(), len + 1);
        ws.textAll(buffer);
      }
      logFile.close();
    }
    else {
      Serial.println("Error while reading logfile");
    }
  }
  else if (strcmp(command, "scan") == 0) {
    WiFi.scanNetworksAsync(printScanResult, true);
  }
  else if (strcmp(command, "gettime") == 0) {
    sendTime();
  }
  else if (strcmp(command, "settime") == 0) {
    unsigned long t = root["epoch"];
    setTime(t);
    sendTime();
  }
  else if (strcmp(command, "getconf") == 0) {
    Serial.println("Call getConfigJson..");
    ws.textAll(config.getConfigJson());
    Serial.println("Call getConfigJson success");
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
    delay(10);
    getAdditionalEVSEData();
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
  msg = "";
}

//////////////////////////////////////////////////////////////////////////////////////////
///////       Setup Functions
//////////////////////////////////////////////////////////////////////////////////////////
bool ICACHE_FLASH_ATTR connectSTA(const char* ssid, const char* password, byte bssid[6]) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password, 0, bssid);
  Serial.print(F("[ INFO ] Trying to connect WiFi: "));
  Serial.print(ssid);

  unsigned long now = millis();
  uint8_t timeout = 20;
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
  //WiFi.disconnect(true);
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

bool ICACHE_FLASH_ATTR loadConfiguration() {

  Serial.println("[ SYSTEM ] Loading Config File on Startup...");
  if (!config.loadConfig()) return false;
  config.loadConfiguration();

  if (config.getSystemDebug()) Serial.println("[ SYSTEM ] Check for old config version and renew it");
  config.renewConfigFile();

  delay(300); // wait a few milliseconds to prevent voltage drop...

  if (config.getSystemDebug()) {
    Serial.println("[ DEBUGGER ] Debug Mode: ON!");
  }
  else {
    Serial.println("[ DEBUGGER ] Debug Mode: OFF!");
  }

  byte bssid[6];
  parseBytes(config.getWifiBssid(), ':', bssid, 6, 16);
  WiFi.hostname(config.getSystemHostname());
  
  if (!MDNS.begin(config.getSystemHostname())) {
    Serial.println("Error setting up MDNS responder!");
  }
  MDNS.addService("http", "tcp", 80);

  if (!config.getSystemWsauth()) {
    ws.setAuthentication("admin", config.getSystemPass());
    if (config.getSystemDebug())Serial.println("[ Websocket ] Use Basic Authentication for Websocket");
  }
  server.addHandler(new SPIFFSEditor("admin", config.getSystemPass()));

  queryEVSE();
  s_addEvseData addEvseData = getAdditionalEVSEData();
  if (vehicleCharging) {
    updateLog(true);
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
    rfid.begin(config.getRfidPin(), config.getRfidGain(), &ntp, config.getSystemDebug());
  }

  deactivateEVSE(false);  //initial deactivation
  millisStopCharging = 0;
  vehicleCharging = false;
  
  if (config.getWifiWmode() == 1) {
    if (config.getSystemDebug()) Serial.println(F("[ INFO ] SimpleEVSE Wifi is running in AP Mode "));
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
  ntp.Ntp(config.getNtpIp(), config.getNtpTimezone(), 3600);   //use NTP Server, timeZone, update every x sec
  
  return true;
}

void ICACHE_FLASH_ATTR restoreDefaultConfig() {
  
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
          request->send(200, "text/plain", ("E1_could not set current - give a value between 6 and " + (String)config.getSystemMaxInstall()));
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

  //evseHost
  server.on("/evseHost", HTTP_GET, [](AsyncWebServerRequest * request) {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    StaticJsonDocument<340> jsonDoc;
    jsonDoc["type"] = "evseHost";
    JsonArray list = jsonDoc.createNestedArray("list");
    JsonObject item = list.createNestedObject();
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
      item["dns"] = printIP(WiFi.dnsIP());
      item["mac"] = WiFi.macAddress();
    }

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
}

void ICACHE_FLASH_ATTR fallbacktoAPMode() {
  WiFi.disconnect(true);
  if (config.getSystemDebug()) Serial.println(F("[ INFO ] EVSE-WiFi is running in Fallback AP Mode"));
  uint8_t macAddr[6];
  WiFi.softAPmacAddress(macAddr);
  char ssid[16];
  sprintf(ssid, "EVSE-WiFi-%02x%02x%02x", macAddr[3], macAddr[4], macAddr[5]);
  isWifiConnected = startAP(ssid);
  setWebEvents();
  inFallbackMode = true;
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
      Update.runAsync(true);
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
void ICACHE_FLASH_ATTR setup() {
  Serial.begin(9600);
  if (config.getSystemDebug()) Serial.println();
  if (config.getSystemDebug()) Serial.println("[ INFO ] SimpleEVSE WiFi");
  delay(500);

  SPIFFS.begin();
  sSerial.begin(9600);
  evseNode.begin(1, sSerial);
  meterNode.begin(2, Serial);

  if (!loadConfiguration()) {
    Serial.println("[ WARNING ] Going to fallback mode!");
    fallbacktoAPMode();
  }
  if (config.getButtonActive(0)) {
    pinMode(config.getButtonPin(0), INPUT_PULLUP);
  }

  //Factory Reset when button pressed for 20 sec after boot
  if (digitalRead(config.getButtonPin(0)) == LOW) {
    pinMode(ledPin, OUTPUT);
    Serial.println("Button Pressed while boot!");
    digitalWrite(ledPin, HIGH);
    unsigned long millisBefore = millis();
    int button = config.getButtonPin(0);
    while (digitalRead(button) == LOW) {  
      if (millis() > (millisBefore + 20000)) {
        factoryReset();
        Serial.println("[ SYSTEM ] System has been reset to factory settings!");
        digitalWrite(ledPin, LOW);
      }
      delay(100);
    }
  }

  if (config.useSMeter) {
    pinMode(config.getMeterPin(0), INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(config.getMeterPin(0)), handleMeterInt, FALLING);
    if (config.getSystemDebug()) Serial.println("[ Meter ] Use GPIO 0-15 with Pull-Up");
  }
  now();
  startWebserver();
  if (config.getSystemDebug()) Serial.println("End of setup routine");

  if (1 == 2) {
    restoreDefaultConfig();
  }
  
}

//////////////////////////////////////////////////////////////////////////////////////////
///////       Loop
//////////////////////////////////////////////////////////////////////////////////////////
void ICACHE_RAM_ATTR loop() {
  unsigned long currentMillis = millis();
  unsigned long uptime = ntp.getUptimeSec();
  previousLoopMillis = currentMillis;
  doChangeLedTimes();

  if (currentMillis >= previousLedAction) {
    if (ledStatus == false) {
      if (currentMillis >= previousLedAction + ledOffTime) {
        digitalWrite(ledPin, HIGH);
        ledStatus = true;
        previousLedAction = currentMillis;
      }
    }
    else {
      if (currentMillis >= previousLedAction + ledOnTime) {
        digitalWrite(ledPin, LOW);
        ledStatus = false;
        previousLedAction = currentMillis;
      }
    }
  }

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
  if (currentMillis >= rfid.cooldown && config.getRfidActive() == true) {
    rfidloop();
  }
  if (toActivateEVSE) {
    activateEVSE();
    delay(300);
  }
  if (toDeactivateEVSE) {
    deactivateEVSE(true);
    delay(300);
  }
  if (currentMillis > (lastModbusAction + 3000)) { //Update Modbus data every 3000ms and send data to WebUI
    queryEVSE();
    if (evseSessionTimeOut == false) {
      sendEVSEdata();
    }
  }
  else if (currentMillis > evseQueryTimeOut &&    //Setting timeout for Evse poll / push to ws
      evseSessionTimeOut == false) {
    evseSessionTimeOut = true;
    pushSessionTimeOut();
  }
  if (config.useMMeter && millisUpdateMMeter < millis()) {
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
  if (toSetEVSEcurrent) {
    setEVSEcurrent();
  }

  if (config.getButtonActive(0) && digitalRead(config.getButtonPin(0)) == HIGH && buttonState == LOW) {
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

  int buttonPin;
  if (inFallbackMode) {
    buttonPin = D4;
  }
  else {
    config.getButtonPin(0);
  }

  if (digitalRead(buttonPin) != buttonState) {
    buttonState = digitalRead(buttonPin);
    buttonTimer = millis();
    if (config.getSystemDebug()) Serial.println("Button pressed...");
  }
  if (digitalRead(buttonPin) == LOW && (millis() - buttonTimer) > 10000) { //Reboot
    if (config.getSystemDebug()) Serial.println("Button Pressed > 10 sec -> Reboot");
    toReboot = true;
  }
  if (toSendStatus == true) {
    sendStatus();
    toSendStatus = false;
  }
}
