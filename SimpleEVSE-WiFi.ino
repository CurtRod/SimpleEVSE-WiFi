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

// #include <Arduino.h>

#ifdef ESP8266
#include <ESP8266WiFi.h> // Whole thing is about using Wi-Fi networks
#include <ESP8266mDNS.h> // Zero-config Library (Bonjour, Avahi)
#include <ESPAsyncTCP.h> // Async TCP Library is mandatory for Async Web Server
#endif

#ifdef ESP32
#include <WiFi.h>
#include <SPIFFS.h>
#include <ESPmDNS.h>
#endif

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
#include "src/proto.h"
#include "src/ntp.h"
#include "src/websrc.h"

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
int16_t iPrice = 0;
uint8_t maxinstall = 0;
uint8_t iFactor = 0;
float consumption = 0.0;
int currentToSet = 6;
int8_t evseStatus = 0;
bool evseSessionTimeOut = false;
bool evseActive = false;
bool vehicleCharging = false;
int buttonState = HIGH;
AsyncWebParameter* awp;
const char * initLog = "{\"type\":\"latestlog\",\"list\":[]}";

//Debug
bool debug = true;

//Metering
float meterReading = 0.0;
float meteredKWh = 0.0;
float currentKW = 0.0;
uint8_t intLength = 0;

//Metering S0
uint8_t meterPin;
uint16_t meterTimeout = 10; //sec
uint16_t kwhimp;
uint8_t meterphase;
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
MFRC522 mfrc522 = MFRC522();  // Create MFRC522 RFID instance
AsyncWebServer server(80);    // Create AsyncWebServer instance on port "80"
AsyncWebSocket ws("/ws");     // Create WebSocket instance on URL "/ws"
NtpClient NTP;

uint8_t queryTimer = 5; // seconds
unsigned long lastModbusAnswer = 0;
unsigned long evseQueryTimeOut = 0;

//Loop
unsigned long previousMillis = 0;
unsigned long previousLoopMillis = 0;
unsigned long cooldown = 0;
bool toSetEVSEcurrent = false;
bool toActivateEVSE = false;
bool toDeactivateEVSE = false;
bool toQueryEVSE = false;
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

uint16_t evseAmpsAfterboot;  //Register 2000
uint16_t evseModbusEnabled;  //Register 2001
uint16_t evseAmpsMin;        //Register 2002
uint16_t evseAnIn;           //Register 2003
uint16_t evseAmpsPowerOn;    //Register 2004
uint16_t evseReg2005;        //Register 2005
uint16_t evseShareMode;      //Register 2006
uint16_t evsePpDetection;    //Register 2007
uint16_t evseBootFirmware;   //Register 2009

//Settings
bool useRFID = false;
bool useSMeter = false;
bool useMMeter = false;
bool useButton = false;
bool dontUseWsAuthentication = false;
bool inAPMode = false;
bool inFallbackMode = false;
bool isWifiConnected = false;
String lastUsername = "";
String lastUID = "";
char * deviceHostname = NULL;
uint8_t buttonPin;
char * adminpass = NULL;
int timeZone;
const char * ntpIP = "pool.ntp.org";

//Others
String msg = ""; //WS communication

//////////////////////////////////////////////////////////////////////////////////////////
///////       Auxiliary Functions
//////////////////////////////////////////////////////////////////////////////////////////

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
  if(meterImpMillis < millis()){
    meterImpMillis = millis();
    meterInterrupt += 1;
    numberOfMeterImps ++;
  }
}

void ICACHE_RAM_ATTR updateS0MeterData() {
  if (vehicleCharging){
    currentKW = 3600.0 / float(meterImpMillis - previousMeterMillis) / float(kwhimp / 1000.0) * (float)iFactor ;  //Calculating kW
    previousMeterMillis = meterImpMillis;
    meterImpMillis = meterImpMillis + intLength;
    meterInterrupt = 0;
    meteredKWh = float(numberOfMeterImps) / float(kwhimp / 1000.0) / 1000.0 * float(iFactor);
  }
}

void ICACHE_FLASH_ATTR updateMMeterData() {
  if (mMeterTypeSDM120 == true){
    currentKW = readMeter(0x000C) / 1000.0;
    meterReading = readMeter(0x0156);
  }
  else if (mMeterTypeSDM630 == true){
    currentKW = readMeter(0x0034) / 1000.0;
    meterReading = readMeter(0x0156);
  }
  if (meterReading != 0.0 &&
      vehicleCharging == true){
    meteredKWh = meterReading - startTotal;
  }
  if (startTotal == 0){
    meteredKWh = 0.0;
  }
  updateSDMMeterCurrent();
  millisUpdateMMeter = millis() + 5000;
}

void ICACHE_FLASH_ATTR updateSDMMeterCurrent(){
  uint8_t result;
  uint16_t iaRes[6];
  
  meterNode.clearTransmitBuffer();
  meterNode.clearResponseBuffer();
  delay(50);
  result = meterNode.readInputRegisters(0x0006, 6);  // read 6 registers starting at 0x0000
  if(debug) Serial.println("");

  if(result != meterNode.ku8MBSuccess){
    Serial.print("[ ModBus ] Error ");
    Serial.print(result, HEX);
    Serial.println(" occured while getting current Meter Data");
  }
  else{
    iaRes[0] = meterNode.getResponseBuffer(0);
    iaRes[1] = meterNode.getResponseBuffer(1);
    iaRes[2] = meterNode.getResponseBuffer(2);
    iaRes[3] = meterNode.getResponseBuffer(3);
    iaRes[4] = meterNode.getResponseBuffer(4);
    iaRes[5] = meterNode.getResponseBuffer(5);
    
    ((uint16_t*)&currentP1)[1]= iaRes[0];
    ((uint16_t*)&currentP1)[0]= iaRes[1];
    ((uint16_t*)&currentP2)[1]= iaRes[2];
    ((uint16_t*)&currentP2)[0]= iaRes[3];
    ((uint16_t*)&currentP3)[1]= iaRes[4];
    ((uint16_t*)&currentP3)[0]= iaRes[5];
  }
}

int ICACHE_FLASH_ATTR getChargingTime(){
  uint32_t iTime;
  if(vehicleCharging == true){
    iTime = millis() - millisStartCharging;
  }
  else {
    iTime = millisStopCharging - millisStartCharging;
  }
  return iTime;
}

//////////////////////////////////////////////////////////////////////////////////////////
///////       RFID Functions
//////////////////////////////////////////////////////////////////////////////////////////
void ICACHE_FLASH_ATTR rfidloop() {
  //If a new PICC placed to RFID reader continue
  //debugMillis = millis();
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    delay(50);
    return;
  }
  //Since a PICC placed get Serial (UID) and continue
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    delay(50);
    return;
  }
  // We got UID tell PICC to stop responding
  mfrc522.PICC_HaltA();
  cooldown = millis() + 2000;
  if(debug) Serial.print(F("[ INFO ] PICC's UID: "));
  String uid = "";
  for (int i = 0; i < mfrc522.uid.size; ++i) {
    uid += String(mfrc522.uid.uidByte[i], HEX);
  }
  if(debug) Serial.print(uid);
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  String type = mfrc522.PICC_GetTypeName(piccType);

  int AccType = 0;
  String filename = "/P/";
  filename += uid;

  File f = SPIFFS.open(filename, "r");
  if (f) {
    size_t size = f.size();
    std::unique_ptr<char[]> buf(new char[size]);
    f.readBytes(buf.get(), size);
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.parseObject(buf.get());
    
    if (json.success()) {
      String username = json["user"];
      AccType = json["acctype"];
      if(debug) Serial.println(" = known PICC");
      if(debug) Serial.print("[ INFO ] User Name: ");
      if (username == "undefined"){
        if(debug) Serial.print(uid);
      }
      else{
        if(debug) Serial.print(username);
      }
      if (AccType == 1) {
        toActivateEVSE = true;
        previousMillis = millis();
        if(debug) Serial.println(" have access");
      }
      else if (AccType == 99)
      {
        toActivateEVSE = true;
        previousMillis = millis();
        if(debug) Serial.println(" have admin access, enable wifi");
      }
      else {
        if(debug) Serial.println(" does not have access");
      }
      lastUsername = username;
      lastUID = uid;
      
      //inform administrator portal
      DynamicJsonBuffer jsonBuffer2;
      JsonObject& root = jsonBuffer2.createObject();
      root["command"] = "piccscan";
      // UID of Scanned RFID Tag
      root["uid"] = uid;
      // Type of PICC
      root["type"] = type;
      root["known"] = 1;
      root["acctype"] = AccType;
      // Username
      root["user"] = username;
      size_t len = root.measureLength();
      AsyncWebSocketMessageBuffer * buffer = ws.makeBuffer(len);
      if (buffer) {
        root.printTo((char *)buffer->get(), len + 1);
        ws.textAll(buffer);
      }
    }
    else {
      if(debug) Serial.println("");
      if(debug) Serial.println(F("[ WARN ] Failed to parse User Data"));
    }
    f.close();
  }
  else { // Unknown PICC
    lastUsername = "Unknown";
    lastUID = uid;
    
    if(debug) Serial.println(" = unknown PICC");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["command"] = "piccscan";
    root["uid"] = uid;
    root["type"] = type;
    root["known"] = 0;
    size_t len = root.measureLength();
    AsyncWebSocketMessageBuffer * buffer = ws.makeBuffer(len);
    if (buffer) {
      root.printTo((char *)buffer->get(), len + 1);
      ws.textAll(buffer);
    }
  }
}

void ICACHE_FLASH_ATTR sendStatus() {
  // Getting additional Modbus data
  uint8_t result;
  struct ip_info info;
  FSInfo fsinfo;
  if (!SPIFFS.info(fsinfo)) {
    Serial.print(F("[ WARN ] Error getting info on SPIFFS"));
  }
  DynamicJsonBuffer jsonBuffer12;
  JsonObject& root = jsonBuffer12.createObject();
  root["command"] = "status";
  root["heap"] = ESP.getFreeHeap();
  root["chipid"] = String(ESP.getChipId(), HEX);
  root["cpu"] = ESP.getCpuFreqMHz();
  root["availsize"] = ESP.getFreeSketchSpace();
  root["availspiffs"] = fsinfo.totalBytes - fsinfo.usedBytes;
  root["spiffssize"] = fsinfo.totalBytes;
  root["uptime"] = NTP.getDeviceUptimeString();

  if (inAPMode) {
    wifi_get_ip_info(SOFTAP_IF, &info);
    struct softap_config conf;
    wifi_softap_get_config(&conf);
    root["ssid"] = String(reinterpret_cast<char*>(conf.ssid));
    root["dns"] = printIP(WiFi.softAPIP());
    root["mac"] = WiFi.softAPmacAddress();
  }
  else {
    wifi_get_ip_info(STATION_IF, &info);
    struct station_config conf;
    wifi_station_get_config(&conf);
    root["ssid"] = String(reinterpret_cast<char*>(conf.ssid));
    root["dns"] = printIP(WiFi.dnsIP());
    root["mac"] = WiFi.macAddress();
  }

  IPAddress ipaddr = IPAddress(info.ip.addr);
  IPAddress gwaddr = IPAddress(info.gw.addr);
  IPAddress nmaddr = IPAddress(info.netmask.addr);
  root["ip"] = printIP(ipaddr);
  root["gateway"] = printIP(gwaddr);
  root["netmask"] = printIP(nmaddr);

  // Getting actual Modbus data
  queryEVSE();
  evseNode.clearTransmitBuffer();
  evseNode.clearResponseBuffer();
  result = evseNode.readHoldingRegisters(0x07D0, 10);  // read 10 registers starting at 0x07D0 (2000)
  
  if (result != 0){
    // error occured
    evseVehicleStatus = 0;
    Serial.print("[ ModBus ] Error ");
    Serial.print(result, HEX);
    Serial.println(" occured while getting additional EVSE data");
    //delay(100);
    return;
  }
  else{
    // register successufully read
    if(debug) Serial.println("[ ModBus ] got additional EVSE data successfully ");
    lastModbusAnswer = millis();

    //process answer
    for(int i = 0; i < 10; i++){
      switch(i){
      case 0:
        evseAmpsAfterboot  = evseNode.getResponseBuffer(i);    //Register 2000
        break;
      case 1:
        evseModbusEnabled = evseNode.getResponseBuffer(i);     //Register 2001
        break;
      case 2:
        evseAmpsMin = evseNode.getResponseBuffer(i);           //Register 2002
        break;
      case 3: 
        evseAnIn = evseNode.getResponseBuffer(i);             //Reg 2003
        break;
      case 4:
        evseAmpsPowerOn = evseNode.getResponseBuffer(i);      //Reg 2004
        break;
      case 5:
        evseReg2005 = evseNode.getResponseBuffer(i);          //Reg 2005
        break;
      case 6:
        evseShareMode = evseNode.getResponseBuffer(i);        //Reg 2006
        break;
      case 7:
        evsePpDetection = evseNode.getResponseBuffer(i);       //Register 2007
        break;
      case 9:
        evseBootFirmware = evseNode.getResponseBuffer(i);       //Register 2009
        break;    
      }
    }
    if (useMMeter){
      updateMMeterData();
    }
  }

  root["evse_amps_conf"] = evseAmpsConfig;          //Reg 1000
  root["evse_amps_out"] = evseAmpsOutput;           //Reg 1001
  root["evse_vehicle_state"] = evseVehicleStatus;   //Reg 1002
  root["evse_pp_limit"] = evseAmpsPP;               //Reg 1003
  root["evse_turn_off"] = evseTurnOff;              //Reg 1004
  root["evse_firmware"] = evseFirmware;             //Reg 1005
  root["evse_state"] = evseState;                   //Reg 1006
  root["evse_amps_afterboot"] = evseAmpsAfterboot;  //Reg 2000
  root["evse_modbus_enabled"] = evseModbusEnabled;  //Reg 2001
  root["evse_amps_min"] = evseAmpsMin;              //Reg 2002
  root["evse_analog_input"] = evseAnIn;             //Reg 2003
  root["evse_amps_poweron"] = evseAmpsPowerOn;      //Reg 2004
  root["evse_2005"] = evseReg2005;                  //Reg 2005
  root["evse_sharing_mode"] = evseShareMode;        //Reg 2006
  root["evse_pp_detection"] = evsePpDetection;      //Reg 2007
  if (useMMeter){
    root["meter_total"] = meterReading;
  }
  size_t len = root.measureLength();
  AsyncWebSocketMessageBuffer * buffer = ws.makeBuffer(len); //  creates a buffer (len + 1) for you.
  if (buffer) {
    root.printTo((char *)buffer->get(), len + 1);
    ws.textAll(buffer);
  }
}

// Send Scanned SSIDs to websocket clients as JSON object
void ICACHE_FLASH_ATTR printScanResult(int networksFound) {
  DynamicJsonBuffer jsonBuffer13;
  JsonObject& root = jsonBuffer13.createObject();
  root["command"] = "ssidlist";
  JsonArray& scan = root.createNestedArray("list");
  for (int i = 0; i < networksFound; ++i) {
    JsonObject& item = scan.createNestedObject();
    // Print SSID for each network found
    item["ssid"] = WiFi.SSID(i);
    item["bssid"] = WiFi.BSSIDstr(i);
    item["rssi"] = WiFi.RSSI(i);
    item["channel"] = WiFi.channel(i);
    item["enctype"] = WiFi.encryptionType(i);
    item["hidden"] = WiFi.isHidden(i) ? true : false;
  }
  size_t len = root.measureLength();
  AsyncWebSocketMessageBuffer * buffer = ws.makeBuffer(len); //  creates a buffer (len + 1) for you.
  if (buffer) {
    root.printTo((char *)buffer->get(), len + 1);
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
    DynamicJsonBuffer jsonBuffer3;
    JsonObject& root = jsonBuffer3.createObject();
    root["type"] = "latestlog";
    JsonArray& list = root.createNestedArray("list");
    root.printTo(logFile);
    logFile.close();

    logFile = SPIFFS.open("/latestlog.json", "r");
  }
  if (logFile) {
    size_t size = logFile.size();
    std::unique_ptr<char[]> buf (new char[size]);
    logFile.readBytes(buf.get(), size);
    DynamicJsonBuffer jsonBuffer4;
    JsonObject& root = jsonBuffer4.parseObject(buf.get());
    JsonArray& list = root["list"];
    if (!root.success()) {
      if(debug) Serial.println("Impossible to read JSON file");
    }
    else {
      logFile.close(); 
      if ( list.size() >= 1000 ) {
        list.remove(0);
      } 
      File logFile = SPIFFS.open("/latestlog.json", "w");
      DynamicJsonBuffer jsonBuffer5;
      JsonObject& item = jsonBuffer5.createObject();
      item["uid"] = uid;
      item["username"] = username;
      item["timestamp"] = NTP.getUtcTimeNow();
      item["duration"] = 0;
      item["energy"] = 0;
      item["price"] = iPrice;
      list.add(item);
      root.printTo(logFile);
    }
    logFile.close();
  }
  else {
    if(debug) Serial.println("Cannot create Logfile");
  }
}

void ICACHE_FLASH_ATTR updateLog(bool e) {
File logFile = SPIFFS.open("/latestlog.json", "r");
  size_t size = logFile.size();
  std::unique_ptr<char[]> buf (new char[size]);
  logFile.readBytes(buf.get(), size);
  DynamicJsonBuffer jsonBuffer6;
  JsonObject& root = jsonBuffer6.parseObject(buf.get());
  JsonArray& list = root["list"];
  if (!root.success()) {
    if(debug) Serial.println("Impossible to read JSON file");
  }
  else {
    logFile.close();
    const char* uid = list[(list.size()-1)]["uid"];
    const char* username = list[(list.size()-1)]["username"];
    long timestamp = (long)list[(list.size()-1)]["timestamp"];
    
    list.remove(list.size() - 1); // delete newest log
    File logFile = SPIFFS.open("/latestlog.json", "w");
    DynamicJsonBuffer jsonBuffer7;
    JsonObject& item = jsonBuffer7.createObject();
    item["uid"] = uid;
    item["username"] = username;
    item["timestamp"] = timestamp;
    if (!e){
      item["duration"] = getChargingTime();
      item["energy"] = float(int((meteredKWh + 0.005) * 100.0)) / 100.0;
      item["price"] = iPrice;
    }
    else{
      item["duration"] = String("e");
      item["energy"] = String("e");
      item["price"] = String("e");
    }
    list.add(item);
    root.printTo(logFile);
  }
  logFile.close();
  millisStartCharging = 0;
  millisStopCharging = 0;
  meteredKWh = 0.0;
  currentKW = 0.0;
}

float ICACHE_FLASH_ATTR getS0MeterReading(){
  float fMeterReading;
  File logFile = SPIFFS.open("/latestlog.json", "r");
  
  if (logFile) {
    size_t size = logFile.size();
    std::unique_ptr<char[]> buf (new char[size]);
    logFile.readBytes(buf.get(), size);
    DynamicJsonBuffer jsonBuffer4;
    JsonObject& root = jsonBuffer4.parseObject(buf.get());
    JsonArray& list = root["list"];
    if (!root.success()) {
      if(debug) Serial.println("Impossible to read Log file");
    }
    else {
      logFile.close(); 
      for(int i = 0; i < list.size(); i++){
        JsonObject& line = list.get<JsonVariant>(i);
        if (line["energy"] != "e"){
          fMeterReading += (float)line["energy"];
        }
      }
    }
  }
  return fMeterReading;
}

bool ICACHE_FLASH_ATTR initLogFile(){
  if(debug)Serial.println("[ SYSTEM ] Going to delete Log File...");
  File logFile = SPIFFS.open("/latestlog.json", "w");
  DynamicJsonBuffer jsonBuffer3;
  JsonObject& root = jsonBuffer3.createObject();
  root["type"] = "latestlog";
  JsonArray& list = root.createNestedArray("list");
  root.printTo(logFile);
  logFile.close(); 
  if(debug)Serial.println("[ SYSTEM ] ... Success!");
}


//////////////////////////////////////////////////////////////////////////////////////////
///////       Meter Modbus functions
//////////////////////////////////////////////////////////////////////////////////////////
float ICACHE_FLASH_ATTR readMeter(uint16_t reg){
  uint8_t result;
  uint16_t iaRes[2];
  float fResponse;
  
  meterNode.clearTransmitBuffer();
  meterNode.clearResponseBuffer();
  delay(50);
  result = meterNode.readInputRegisters(reg, 2);  // read 7 registers starting at 0x0000
  if(debug) Serial.println("");

  if(result != meterNode.ku8MBSuccess){
    Serial.print("[ ModBus ] Error ");
    Serial.print(result, HEX);
    Serial.println(" occured while getting Meter Data");
  }
  else{
    iaRes[0] = meterNode.getResponseBuffer(0);
    iaRes[1] = meterNode.getResponseBuffer(1);

    ((uint16_t*)&fResponse)[1]= iaRes[0];
    ((uint16_t*)&fResponse)[0]= iaRes[1];

    return (fResponse);
  }
}

//////////////////////////////////////////////////////////////////////////////////////////
///////       EVSE Modbus functions
//////////////////////////////////////////////////////////////////////////////////////////
bool ICACHE_FLASH_ATTR queryEVSE(){
  uint8_t result;
  
  evseNode.clearTransmitBuffer();
  evseNode.clearResponseBuffer();
  result = evseNode.readHoldingRegisters(0x03E8, 7);  // read 7 registers starting at 0x03E8 (1000)
  
  if (result != 0){

    evseVehicleStatus = 0;
    Serial.print("[ ModBus ] Error ");
    Serial.print(result, HEX);
    Serial.println(" occured while getting EVSE data - trying again...");
    evseNode.clearTransmitBuffer();
    evseNode.clearResponseBuffer();
    delay(500);
    return false;
  }
  else{
    // register successufully read
    if(debug) Serial.println("[ ModBus ] got EVSE data successfully ");
    lastModbusAnswer = millis();

    //process answer
    for(int i = 0; i < 7; i++){
      switch(i){
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
    if (evseVehicleStatus == 0){
      evseStatus = 0; //modbus communication failed
    }
    if (evseState == 3){     //EVSE not Ready
        if (evseVehicleStatus == 2 ||
            evseVehicleStatus == 3 ){
          evseStatus = 2; //vehicle detected
        }
        else{
          evseStatus = 1; // EVSE deactivated
        }
        if (vehicleCharging == true){   //vehicle interrupted charging
          vehicleCharging = false;
          millisStopCharging = millis();
          if(debug) Serial.println("Vehicle interrupted charging");
          updateLog(false);
        }
        evseActive = false;
        return true;
    }
    if (evseVehicleStatus == 1){
      evseStatus = 1;  // ready
    }
    else if (evseVehicleStatus == 2){
      evseStatus = 2; //vehicle detected
    }
    else if (evseVehicleStatus == 3){
      evseStatus = 3; //charging
      if (vehicleCharging == false) {
        vehicleCharging = true;
      }
    }
    return true;
  }
}

bool ICACHE_FLASH_ATTR activateEVSE() {
  static uint16_t iTransmit;
  if(debug) Serial.println("[ ModBus ] Query Modbus before activating EVSE");
  queryEVSE();
  
  if (evseState == 3 &&
      evseVehicleStatus != 0){    //no modbus error occured
      iTransmit = 8192;         //disable EVSE after charge
      
    uint8_t result;
    evseNode.clearTransmitBuffer();
    evseNode.setTransmitBuffer(0, iTransmit); // set word 0 of TX buffer (bits 15..0)
    result = evseNode.writeMultipleRegisters(0x07D5, 1);  // write register 0x07D5 (2005)
  
    if (result != 0){
      // error occured
      Serial.print("[ ModBus ] Error ");
      Serial.print(result, HEX);
      Serial.println(" occured while activating EVSE - trying again...");
      return false;
    }
    else{
      // register successufully written
      if(debug) Serial.println("[ ModBus ] EVSE successfully activated");
      toActivateEVSE = false;
      evseActive = true;
      logLatest(lastUID, lastUsername);
      vehicleCharging = true;
      if(useMMeter){
        delay(20);
        if(mMeterTypeSDM120){
          startTotal = readMeter(0x0156);
        }
        else if(mMeterTypeSDM630){
          startTotal = readMeter(0x0156);
        }
        meteredKWh = 0.0;
      }
      else{
        startTotal = getS0MeterReading();
        meteredKWh = 0.0;  
      }
      numberOfMeterImps = 0;
      millisStartCharging = millis();
      sendEVSEdata();
      return true;
    }
  }
  else if (evseVehicleStatus != 0){
    if(debug) Serial.println("[ Modbus ] EVSE already active! Going to deactivate EVSE...");
    toActivateEVSE = false;
    toDeactivateEVSE = true;
    evseActive = true;
    return true;
  }
  return false;
}

bool ICACHE_FLASH_ATTR deactivateEVSE(bool logUpdate) {
  //New ModBus Master Library
  static uint16_t iTransmit = 16384;  // deactivate evse
  uint8_t result;
  
  evseNode.clearTransmitBuffer();
  evseNode.setTransmitBuffer(0, iTransmit); // set word 0 of TX buffer (bits 15..0)
  result = evseNode.writeMultipleRegisters(0x07D5, 1);  // write register 0x07D5 (2005)
  
  if (result != 0){
    // error occured
    Serial.print("[ ModBus ] Error ");
    Serial.print(result, HEX);
    Serial.println(" occured while deactivating EVSE - trying again...");
    return false;
  }
  else{
    // register successufully written
    if(debug) Serial.println("[ ModBus ] EVSE successfully deactivated");

    if(useMMeter){
      if(mMeterTypeSDM120){
        meteredKWh = readMeter(0x0156) - startTotal;
      }
      else if(mMeterTypeSDM630){
        meteredKWh = readMeter(0x0156) - startTotal;
      }
    }
    else{
      startTotal += meteredKWh;
    }
    if(logUpdate){
      updateLog(false);
    }
    evseActive = false;
    vehicleCharging = false;
    
    queryEVSE();
    sendEVSEdata();
    
    toDeactivateEVSE = false;
    return true;
  }
}

bool ICACHE_FLASH_ATTR setEVSEcurrent(){  // telegram 1: write EVSE current
  //New ModBus Master Library
  uint8_t result;
  
  evseNode.clearTransmitBuffer();
  evseNode.setTransmitBuffer(0, currentToSet); // set word 0 of TX buffer (bits 15..0)
  result = evseNode.writeMultipleRegisters(0x03E8, 1);  // write register 0x03E8 (1000 - Actual configured amps value)
  
  if (result != 0){
    // error occured
    Serial.print("[ ModBus ] Error ");
    Serial.print(result, HEX);
    Serial.println(" occured while setting current in EVSE - trying again...");
    return false;
  }
  else{
    // register successufully written
    if(debug) Serial.println("[ ModBus ] Current successfully set");
    evseAmpsConfig = currentToSet;  //foce update in WebUI
    sendEVSEdata();               //foce update in WebUI
    toSetEVSEcurrent = false;
    return true;
  }
}

bool ICACHE_FLASH_ATTR setEVSERegister(uint16_t reg, uint16_t val){
  uint8_t result;
  evseNode.clearTransmitBuffer();
  evseNode.setTransmitBuffer(0, val); // set word 0 of TX buffer (bits 15..0)
  result = evseNode.writeMultipleRegisters(reg, 1);  // write given register
  
  if (result != 0){
    // error occured
    Serial.print("[ ModBus ] Error ");
    Serial.print(result, HEX);
    Serial.println(" occured while setting EVSE Register " + (String)reg + " to " + (String)val);
    return false;
  }
  else{   
    // register successufully written
    if(debug) Serial.println("[ ModBus ] Register " + (String)reg + " successfully set to " + (String)val);
    return true;
  }
}

//////////////////////////////////////////////////////////////////////////////////////////
///////       Websocket Functions
//////////////////////////////////////////////////////////////////////////////////////////
void ICACHE_FLASH_ATTR pushSessionTimeOut(){
  // push "TimeOut" to evse.htm!
  // Encode a JSON Object and send it to All WebSocket Clients
  DynamicJsonBuffer jsonBuffer15;
  JsonObject& root = jsonBuffer15.createObject();
  root["command"] = "sessiontimeout";
  size_t len = root.measureLength();
  AsyncWebSocketMessageBuffer * buffer = ws.makeBuffer(len); //  creates a buffer (len + 1) for you.
  if (buffer) {
    root.printTo((char *)buffer->get(), len + 1);
    ws.textAll(buffer);
  }
  if(debug) Serial.println("[ WebSocket ] TimeOut sent to browser!");
}

void ICACHE_FLASH_ATTR sendEVSEdata(){
  if (evseSessionTimeOut == false){
    DynamicJsonBuffer jsonBuffer9;
    JsonObject& root = jsonBuffer9.createObject();
    root["command"] = "getevsedata";
    root["evse_vehicle_state"] = evseStatus;
    root["evse_active"] = evseActive;
    root["evse_current_limit"] = evseAmpsConfig;
    root["evse_current"] = String(currentKW, 2);
    root["evse_charging_time"] = getChargingTime();
    root["evse_charged_kwh"] = String(meteredKWh, 2);
    root["evse_charged_amount"] = String((meteredKWh * float(iPrice) / 100.0), 2);
    root["evse_maximum_current"] = maxinstall;
    if(meteredKWh == 0.0){
      root["evse_charged_mileage"] = "0.0";  
    }
    else{
      root["evse_charged_mileage"] = String((meteredKWh * 100.0 / consumption), 1);
    }
    root["ap_mode"] = inAPMode;
    size_t len = root.measureLength();
    AsyncWebSocketMessageBuffer * buffer = ws.makeBuffer(len);
    if (buffer) {
      root.printTo((char *)buffer->get(), len + 1);
     ws.textAll(buffer);
    }
  }
}

void ICACHE_FLASH_ATTR sendTime() {
  DynamicJsonBuffer jsonBuffer10;
  JsonObject& root = jsonBuffer10.createObject();
  root["command"] = "gettime";
  root["epoch"] = now();
  root["timezone"] = timeZone;
  size_t len = root.measureLength();
  AsyncWebSocketMessageBuffer * buffer = ws.makeBuffer(len);
  if (buffer) {
    root.printTo((char *)buffer->get(), len + 1);
    ws.textAll(buffer);
  }
}

void ICACHE_FLASH_ATTR sendUserList(int page, AsyncWebSocketClient * client) {
  DynamicJsonBuffer jsonBuffer11;
  JsonObject& root = jsonBuffer11.createObject();
  root["command"] = "userlist";
  root["page"] = page;
  JsonArray& users = root.createNestedArray("list");
  Dir dir = SPIFFS.openDir("/P/");
  int first = (page - 1) * 15;
  int last = page * 15;
  int i = 0;
  while (dir.next()) {
    if (i >= first && i < last) {
      JsonObject& item = users.createNestedObject();
      String uid = dir.fileName();
      uid.remove(0, 3);
      item["uid"] = uid;
      File f = SPIFFS.open(dir.fileName(), "r");
      size_t size = f.size();
      // Allocate a buffer to store contents of the file.
      std::unique_ptr<char[]> buf(new char[size]);
      f.readBytes(buf.get(), size);
      DynamicJsonBuffer jsonBuffer16;
      JsonObject& json = jsonBuffer16.parseObject(buf.get());
      if (json.success()) {
        String username = json["user"];
        int AccType = json["acctype"];
        unsigned long validuntil = json["validuntil"];
        item["username"] = username;
        item["acctype"] = AccType;
        item["validuntil"] = validuntil;
      }
    }
    i++;
  }
  float pages = i / 15.0;
  root["haspages"] = ceil(pages);
  size_t len = root.measureLength();
  AsyncWebSocketMessageBuffer * buffer = ws.makeBuffer(len); //  creates a buffer (len + 1) for you.
  if (buffer) {
    root.printTo((char *)buffer->get(), len + 1);
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
    if(debug) Serial.printf("[ WARN ] WebSocket[%s][%u] error(%u): %s\r\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
  }
  else if (type == WS_EVT_DATA) {
    AwsFrameInfo * info = (AwsFrameInfo*)arg;
    if (info->final && info->index == 0 && info->len == len) {
      //the whole message is in a single frame and we got all of it's data
      if(debug)Serial.println("[ Websocket ] single Frame - all data is here!");
      for (size_t i = 0; i < info->len; i++) {
        msg += (char) data[i];
      }
      DynamicJsonBuffer jsonBuffer8;
      JsonObject& root = jsonBuffer8.parseObject(msg);
      if (!root.success()) {
        if(debug) Serial.println(F("[ WARN ] Couldn't parse WebSocket message"));
        msg = "";
        return;
      }
      processWsEvent(root, client);
    }
    else{
      //message is comprised of multiple frames or the frame is split into multiple packets
      if(debug)Serial.println("[ Websocket ] more than one Frame!");
      for (size_t i = 0; i < len; i++) {
        msg += (char) data[i];
      }
      if(info->final && (info->index + len) == info->len){
        DynamicJsonBuffer jsonBuffer8;
        JsonObject& root = jsonBuffer8.parseObject(msg);
        if (!root.success()) {
          if(debug) Serial.println(F("[ WARN ] Couldn't parse WebSocket message"));
          msg = "";
          return;
        }
        root.prettyPrintTo(Serial);
        processWsEvent(root, client);
      }
    }
  }
}

void ICACHE_FLASH_ATTR processWsEvent(JsonObject& root, AsyncWebSocketClient * client){
  const char * command = root["command"];
  if (strcmp(command, "remove")  == 0) {
    const char* uid = root["uid"];
    String filename = "/P/";
    filename += uid;
    SPIFFS.remove(filename);
  }
  else if (strcmp(command, "configfile")  == 0) {
    if(debug) Serial.println("[ SYSTEM ] Try to update config.json...");
    File f = SPIFFS.open("/config.json", "w+");
    if (f) {
      root.prettyPrintTo(f);
      //f.print(msg);
      f.close();
      if(vehicleCharging){
        deactivateEVSE(true);
        delay(100);
      }
      if(debug) Serial.println("[ SYSTEM ] Success - going to reboot now");
      ESP.reset();
    }
    else{
      if(debug) Serial.println("[ SYSTEM ] Could not save config.json");
    }
  }
  else if (strcmp(command, "userlist")  == 0) {
    int page = root["page"];
    sendUserList(page, client);
  }
  else if (strcmp(command, "status")  == 0) {
    toSendStatus = true;
  }
  else if (strcmp(command, "userfile")  == 0) {
    const char* uid = root["uid"];
    String filename = "/P/";
    filename += uid;
    File f = SPIFFS.open(filename, "w+");
    // Check if we created the file
    if (f) {
      f.print(msg);
      if(debug) Serial.println("[ DEBUG ] Userfile written!");
    }
    f.close();
    ws.textAll("{\"command\":\"result\",\"resultof\":\"userfile\",\"result\": true}");
  }
  else if (strcmp(command, "latestlog")  == 0) {
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
    else{
      Serial.println("Error while reading logfile");
    }
  }
  else if (strcmp(command, "scan")  == 0) {
    WiFi.scanNetworksAsync(printScanResult, true);
  }
  else if (strcmp(command, "gettime")  == 0) {
    sendTime();
  }
  else if (strcmp(command, "settime")  == 0) {
    unsigned long t = root["epoch"];
    setTime(t);
    sendTime();
  }
  else if (strcmp(command, "getconf")  == 0) {
    File configFile = SPIFFS.open("/config.json", "r");
    if (configFile) {
      size_t len = configFile.size();
      AsyncWebSocketMessageBuffer * buffer = ws.makeBuffer(len); //  creates a buffer (len + 1) for you.
      if (buffer) {
        configFile.readBytes((char *)buffer->get(), len + 1);
        ws.textAll(buffer);
      }
      configFile.close();
    }
  }
  else if (strcmp(command, "getevsedata") == 0){
    sendEVSEdata();
    evseQueryTimeOut = millis() + 10000; //Timeout for pushing data in loop
    evseSessionTimeOut = false;
    if(debug) Serial.println("[ WebSocket ] Data sent to UI");
    toQueryEVSE = true;
  }
  else if (strcmp(command, "setcurrent") == 0){
    currentToSet = root["current"];
    if(debug) Serial.print("[ WebSocket ] Call setEVSECurrent() ");
    if(debug) Serial.println(currentToSet);
    toSetEVSEcurrent = true;
  }
  else if (strcmp(command, "activateevse") == 0){
    toActivateEVSE = true;
    if(debug) Serial.println("[ WebSocket ] Activate EVSE via WebSocket");
    lastUID = "-";
    lastUsername = "GUI";
  }
  else if (strcmp(command, "deactivateevse") == 0){
    toDeactivateEVSE = true;
    if(debug) Serial.println("[ WebSocket ] Deactivate EVSE via WebSocket");
  }
  else if (strcmp(command, "setevsereg") == 0){
    uint16_t reg = atoi(root["register"]);
    uint16_t val = atoi(root["value"]);
    setEVSERegister(reg, val);
  }
  else if (strcmp(command, "factoryreset") == 0){
    SPIFFS.remove("/config.json");
    SPIFFS.remove("/latestlog.json");
  }
  else if (strcmp(command, "initlog") == 0){
    if(debug)Serial.println("[ SYSTEM ] Websocket Command \"initlog\"...");
    initLogFile();
  }
  msg = "";
}

//////////////////////////////////////////////////////////////////////////////////////////
///////       Setup Functions
//////////////////////////////////////////////////////////////////////////////////////////
void ICACHE_FLASH_ATTR ShowReaderDetails() {
  // Get the MFRC522 software version
  byte v = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  if(debug) Serial.print(F("[ INFO ] MFRC522 Version: 0x"));
  if(debug) Serial.print(v, HEX);
  if (v == 0x91)
    if(debug) Serial.print(F(" = v1.0"));
  else if (v == 0x92)
    if(debug) Serial.print(F(" = v2.0"));
  else if (v == 0x88)
    if(debug) Serial.print(F(" = clone"));
  else
    if(debug) Serial.print(F(" (unknown)"));
  if(debug) Serial.println("");
  // When 0x00 or 0xFF is returned, communication probably failed
  if ((v == 0x00) || (v == 0xFF)) {
    if(debug) Serial.println(F("[ WARN ] Communication failure, check if MFRC522 properly connected"));
  }
}

void ICACHE_FLASH_ATTR setupRFID(int rfidss, int rfidgain) {
  SPI.begin();           // MFRC522 Hardware uses SPI protocol
    mfrc522.PCD_Init(rfidss, UINT8_MAX);    // Initialize MFRC522 Hardware
    mfrc522.PCD_SetAntennaGain(rfidgain);
    if(debug) Serial.printf("[ INFO ] RFID SS_PIN: %u and Gain Factor: %u", rfidss, rfidgain);
    if(debug) Serial.println("");
    ShowReaderDetails();
}

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
    if(debug) Serial.print(F("."));
  }
  while (millis() - now < timeout * 1000);
  if (WiFi.status() == WL_CONNECTED) {
    isWifiConnected = true;
    return true;
  }
  else {
    if(debug) Serial.println();
    if(debug) Serial.println(F("[ WARN ] Couldn't connect in time"));
    return false;
  }
}

bool ICACHE_FLASH_ATTR startAP(const char * ssid, const char * password = NULL){
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
  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile) {
    if(debug) Serial.println(F("[ WARN ] Failed to open config file"));
    return false;
  }
  size_t size = configFile.size();
  std::unique_ptr<char[]> buf(new char[size]);
  configFile.readBytes(buf.get(), size);
  DynamicJsonBuffer jsonBuffer14;
  JsonObject& json = jsonBuffer14.parseObject(buf.get());
  if (!json.success()) {
    Serial.println(F("[ WARN ] Failed to parse config file"));
    return false;
  }
  Serial.println(F("[ INFO ] Config file found"));
  json.prettyPrintTo(Serial);
  Serial.println();

  if (json.containsKey("debug")){
    debug = json["debug"];
  }
  if(debug){
    Serial.println("[ DEBUGGER ] Debug Mode: ON!");
  }
  else{
    Serial.println("[ DEBUGGER ] Debug Mode: OFF!");
  }
  
  useRFID = false;
  if (json.containsKey("rfid") &&
       json.containsKey("sspin") &&
       json.containsKey("rfidgain")){
    if (json["rfid"] == true){
      int rfidss = json["sspin"];
      int rfidgain = json["rfidgain"];
      useRFID = true;
      if(debug) Serial.println(F("[ INFO ] Trying to setup RFID hardware"));
      setupRFID(rfidss, rfidgain);
    }
  }

  useMMeter = false;
  useSMeter = false;
  if(json.containsKey("meter") &&
      json.containsKey("metertype")){
    iPrice = json["price"];
    if(json["metertype"] != "S0"){    //Modbus meter
      if (json["metertype"] == "SDM120"){
        mMeterTypeSDM120 = true;
      }
      else if (json["metertype"] == "SDM630"){
        mMeterTypeSDM630 = true;
      }
      useMMeter = true;
      useSMeter = false;
      if(debug) Serial.print(F("[ INFO ] Modbus Meter is configured: "));
      if(debug) json["metertype"].printTo(Serial);
      if(debug) Serial.println();
    }
    else if(json["metertype"] == "S0"){
      if(json.containsKey("intpin") &&
          json.containsKey("kwhimp") &&
          json.containsKey("meterphase")){
        meterPin = json["intpin"];
        kwhimp = json["kwhimp"];
        meterphase = json["meterphase"];
        useMMeter = false;
        useSMeter = true;
        if(json.containsKey("implen")){
          intLength = json["implen"];
          intLength += 3;
        }
        else{
          intLength = 33;
        }
        if(debug) Serial.println(F("[ INFO ] S0 Meter is configured"));
      }
    }
  }

  useButton = false;
  if(json.containsKey("buttonactive")){
    if(json["buttonactive"] == true &&
        json.containsKey("buttonpin")){
      useButton = true;
      buttonPin = json["buttonpin"];
      if(debug) Serial.println("[ INFO ] EVSE set to \"Button active\"");
    }
  }

  if(json.containsKey("avgconsumption")){
    String sConsumption = json["avgconsumption"];
    consumption = strtof((sConsumption).c_str(),0);
  }
  
  if(json.containsKey("ntpIP")){
    ntpIP = json["ntpIP"];
  }
  
  const char * l_hostname = json["hostnm"];
  free(deviceHostname);
  deviceHostname = strdup(l_hostname);
  
  const char * bssidmac = json["bssid"];
  byte bssid[6];
  parseBytes(bssidmac, ':', bssid, 6, 16);
  WiFi.hostname(deviceHostname);

  if (!MDNS.begin(deviceHostname)) {
    Serial.println("Error setting up MDNS responder!");
  }
  MDNS.addService("http", "tcp", 80);

  if(json.containsKey("timezone")){
    timeZone = json["timezone"];
  }
  
  iFactor = json["factor"];
  maxinstall = json["maxinstall"];

  const char * ssid = json["ssid"];
  const char * password = json["pswd"];
  int wmode = json["wmode"];
  adminpass = strdup(json["adminpwd"]);

  if(json.containsKey("wsauth")){
    dontUseWsAuthentication = json["wsauth"];
  }
 
  if(!dontUseWsAuthentication){
    ws.setAuthentication("admin", adminpass);
    if(debug)Serial.println("[ Websocket ] Use Basic Authentication for Websocket");
  }
  server.addHandler(new SPIFFSEditor("admin", adminpass));

  queryEVSE();
  if(vehicleCharging){
    updateLog(true);
  }
  deactivateEVSE(false);  //initial deactivation
  vehicleCharging = false;

  if (wmode == 1) {
    if(debug) Serial.println(F("[ INFO ] SimpleEVSE Wifi is running in AP Mode "));
    WiFi.disconnect(true);
    return startAP(ssid, password);
  }
  if (!connectSTA(ssid, password, bssid)) {
    return false;
  }

  if(json.containsKey("staticip") &&
      json.containsKey("ip") &&
      json.containsKey("subnet") &&
      json.containsKey("gateway") &&
      json.containsKey("dns")){
    if (json["staticip"] == true){
      const char * clientipch = json["ip"];
      const char * subnetch = json["subnet"];
      const char * gatewaych = json["gateway"];
      const char * dnsch = json["dns"];

      IPAddress clientip;
      IPAddress subnet;
      IPAddress gateway;
      IPAddress dns;

      clientip.fromString(clientipch);
      subnet.fromString(subnetch);
      gateway.fromString(gatewaych);
      dns.fromString(dnsch);
      
      WiFi.config(clientip, gateway, subnet, dns);
    }
  }

  Serial.println();
  Serial.print(F("[ INFO ] Client IP address: "));
  Serial.println(WiFi.localIP());

//Check internet connection
  delay(100);
    if(debug) Serial.print("[ NTP ] NTP Server - set up NTP"); 
    const char * ntpserver = ntpIP;
    IPAddress timeserverip;
    WiFi.hostByName(ntpserver, timeserverip);
    String ip = printIP(timeserverip);
    if(debug) Serial.println(" IP: " + ip);
    NTP.Ntp(ntpIP, timeZone, 3600);   //use NTP Server, timeZone, update every x sec
//  }
}

void ICACHE_FLASH_ATTR setWebEvents(){
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
    DynamicJsonBuffer jsonBuffer17;
    JsonObject& root = jsonBuffer17.createObject();
    root["type"] = "parameters";
    JsonArray& list = root.createNestedArray("list");
    DynamicJsonBuffer jsonBuffer18;
    JsonObject& item = jsonBuffer18.createObject();
    item["vehicleState"] = evseStatus;
    item["evseState"] = evseActive;
    item["actualCurrent"] = evseAmpsConfig;
    item["actualPower"] =  float(int((currentKW + 0.005) * 100.0)) / 100.0;
    item["duration"] = getChargingTime();
    item["energy"] = float(int((meteredKWh + 0.005) * 100.0)) / 100.0;
    item["mileage"] = float(int(((meteredKWh * 100.0 / consumption) + 0.05) * 10.0)) / 10.0;
    if (useMMeter){
      item["meterReading"] = float(int((meterReading + 0.005) * 100.0)) / 100.0;
      item["currentP1"] = currentP1;
      item["currentP2"] = currentP2;
      item["currentP3"] = currentP3;
    }
    else{
      item["meterReading"] = float(int((startTotal + meteredKWh + 0.005) * 100.0)) / 100.0;;
      if(meterphase == 1){
        float fCurrent = float(int((currentKW / float(iFactor) / 0.227 + 0.005) * 100.0) / 100.0);
        if(iFactor == 1){
          item["currentP1"] = fCurrent;
          item["currentP2"] = 0.0;
          item["currentP3"] = 0.0;
        }
        else if(iFactor == 2){
          item["currentP1"] = fCurrent;
          item["currentP2"] = fCurrent;
          item["currentP3"] = 0.0;
        }
        else if(iFactor == 3){
          item["currentP1"] = fCurrent;
          item["currentP2"] = fCurrent;
          item["currentP3"] = fCurrent;
        }
      }
      else{
        float fCurrent = float(int((currentKW / 0.227 / float(iFactor) / 3.0 + 0.005) * 100.0) / 100.0);
        item["currentP1"] = fCurrent;
        item["currentP2"] = fCurrent;
        item["currentP3"] = fCurrent;
      }
    }
    list.add(item);
    root.printTo(*response);
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
      if(awp->name() == "current"){
        if(atoi(awp->value().c_str()) <= maxinstall && atoi(awp->value().c_str()) >= 6 ){
          currentToSet = atoi(awp->value().c_str());
          if(setEVSEcurrent()){
            request->send(200, "text/plain", "S0_set current to A");
          }          
          else{
            request->send(200, "text/plain", "E0_could not set current - internal error");
          }
        }
        else{
          request->send(200, "text/plain", ("E1_could not set current - give a value between 6 and " + (String)maxinstall));
        }
      }
      else{
        request->send(200, "text/plain", "E2_could not set current - wrong parameter");
      }
  });
  
  //setStatus
  server.on("/setStatus", HTTP_GET, [](AsyncWebServerRequest * request) {
    awp = request->getParam(0);
    if(awp->name() == "active"){
      if(debug) Serial.println(awp->value().c_str());
      if(strcmp(awp->value().c_str(), "true") == 0){
        if(!evseActive){
          if(activateEVSE()){
            request->send(200, "text/plain", "S0_EVSE successfully activated");
          }
          else{
            request->send(200, "text/plain", "E0_could not activate EVSE - internal error!");
          }
        }
        else{
          request->send(200, "text/plain", "E3_could not activate EVSE - EVSE already activated!");
        }
      }
      else if(strcmp(awp->value().c_str(), "false") == 0){
        if(evseActive){
          if(deactivateEVSE(true)){
            request->send(200, "text/plain", "S0_EVSE successfully deactivated");
          }
          else{
            request->send(200, "text/plain", "E0_could not deactivate EVSE - internal error!");
          }
        }
        else{
          request->send(200, "text/plain", "E3_could not deactivate EVSE - EVSE already deactivated!");
        }
      }
      else{
        request->send(200, "text/plain", "E1_could not process - give a valid value (true/false)");
      }
    }
    else{
      request->send(200, "text/plain", "E2_could not process - wrong parameter");
    }
  });

    server.on("/evseHost", HTTP_GET, [](AsyncWebServerRequest * request) {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonBuffer jsonBuffer19;
    JsonObject& root = jsonBuffer19.createObject();
    root["type"] = "evseHost";
    JsonArray& list = root.createNestedArray("list");
    DynamicJsonBuffer jsonBuffer20;
    JsonObject& item = jsonBuffer20.createObject();

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

    list.add(item);
    root.printTo(*response);
    request->send(response);
  });
  
}

void ICACHE_FLASH_ATTR fallbacktoAPMode() {
  WiFi.disconnect(true);
  if(debug) Serial.println(F("[ INFO ] SimpleEVSE Wifi is running in Fallback AP Mode"));
  uint8_t macAddr[6];
  WiFi.softAPmacAddress(macAddr);
  char ssid[16];
  sprintf(ssid, "EVSE-WiFi-%02x%02x%02x", macAddr[3], macAddr[4], macAddr[5]);
  if(adminpass == NULL)adminpass = "admin";
  isWifiConnected = startAP(ssid);
  void setWebEvents();
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
      if(debug) Serial.printf("[ UPDT ] Firmware update started: %s\n", filename.c_str());
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
        if(debug) Serial.printf("[ UPDT ] Firmware update finished: %uB\n", index + len);
      } else {
        Update.printError(Serial);
      }
    }
  });

  setWebEvents();

  // HTTP basic authentication
  server.on("/login", HTTP_GET, [](AsyncWebServerRequest * request) {
      if (!request->authenticate("admin", adminpass)) {
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
  if(debug) Serial.println();
  if(debug) Serial.println("[ INFO ] SimpleEVSE WiFi");
  delay(1000);
  pinMode(D0, INPUT_PULLDOWN_16);
  
  SPIFFS.begin();
  sSerial.begin(9600);
  evseNode.begin(1, sSerial);
  meterNode.begin(2, Serial);
  
  if (!loadConfiguration()) {
    fallbacktoAPMode();
  }
  if(useButton){
    pinMode(buttonPin, INPUT_PULLUP);
  }
  if (useSMeter){
    pinMode(meterPin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(meterPin), handleMeterInt, FALLING);
    if(debug) Serial.println("[ Meter ] Use GPIO 0-15 with Pull-Up");
  }
  now();
  startWebserver();
  if(debug) Serial.println("End of setup routine");
}

//////////////////////////////////////////////////////////////////////////////////////////
///////       Loop
//////////////////////////////////////////////////////////////////////////////////////////
void ICACHE_RAM_ATTR loop() {
  unsigned long currentMillis = millis();
  unsigned long deltaTime = currentMillis - previousLoopMillis;
  unsigned long uptime = NTP.getUptimeSec();
  previousLoopMillis = currentMillis;
  

  if (uptime > 604800000) {   // auto restart after 7 days
    if(vehicleCharging == false){
      if(debug) Serial.println(F("[ UPDT ] Auto restarting..."));
      delay(1000);
      toReboot = true;
    }
  }
  if (inFallbackMode && uptime > 600){
    toReboot = true;
  }
  if (toReboot) {
    if(debug) Serial.println(F("[ UPDT ] Rebooting..."));
    delay(100);
    ESP.restart();
  }
  if (currentMillis >= cooldown && useRFID == true) {
    rfidloop();
  }
  if ((currentMillis > ( lastModbusAnswer + 3000)) && //Update Modbus data every 3000ms and send data to WebUI
        toQueryEVSE == true &&
        evseSessionTimeOut == false) {
    queryEVSE();
    sendEVSEdata();
  }
  else if (currentMillis > evseQueryTimeOut &&    //Setting timeout for Evse poll / push to ws
          evseSessionTimeOut == false){
    evseSessionTimeOut = true;
    pushSessionTimeOut();
  }
  if(currentMillis > lastModbusAnswer + ( queryTimer * 1000 ) && evseActive == true && evseSessionTimeOut == true){ //Query modbus every x seconds, when webinterface is not shown
    queryEVSE();
  }
  if (useMMeter && evseActive && millisUpdateMMeter < millis()){
    updateMMeterData();
  }
  if (meterInterrupt != 0){
    updateS0MeterData();
  }
  if (useSMeter && previousMeterMillis < millis() - (meterTimeout * 1000)) {  //Timeout when there is less than ~300 watt power consuption -> 6 sec of no interrupt from meter
    if(previousMeterMillis != 0){
      currentKW = 0.0;
    }
  }
  if (toSetEVSEcurrent){
    setEVSEcurrent();
  }
  if (toActivateEVSE){
    activateEVSE();
  }
  if (toDeactivateEVSE){
    deactivateEVSE(true);
  }
  if (useButton && digitalRead(buttonPin) != buttonState){
    buttonState = digitalRead(buttonPin);
    if (buttonState == LOW){
      if (evseActive == false){
        toActivateEVSE = true;
      }
      else if (evseActive == true){
        toDeactivateEVSE = true;
      }
    }
  }
  if (toSendStatus == true){
    sendStatus();
    toSendStatus = false;
  }
}
