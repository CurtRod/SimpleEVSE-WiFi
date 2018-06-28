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

#include <ESP8266WiFi.h>              // Whole thing is about using Wi-Fi networks
#include <SPI.h>                      // RFID MFRC522 Module uses SPI protocol
#include <ESP8266mDNS.h>              // Zero-config Library (Bonjour, Avahi)
#include <MFRC522.h>                  // Library for Mifare RC522 Devices
#include <ArduinoJson.h>              // JSON Library for Encoding and Parsing Json object to send browser
#include <FS.h>                       // SPIFFS Library for storing web files to serve to web browsers
#include <ESPAsyncTCP.h>              // Async TCP Library is mandatory for Async Web Server
#include <ESPAsyncWebServer.h>        // Async Web Server with built-in WebSocket Plug-in
#include <SPIFFSEditor.h>             // This creates a web page on server which can be used to edit text based files
#include <TimeLib.h>                  // Library for converting epochtime to a date
#include <WiFiUdp.h>                  // Library for manipulating UDP packets which is used by NTP Client to get Timestamps
#include <SoftwareSerial.h>           // Using GPIOs for Serial Modbus communication
#include <ModbusMaster.h>
#include <ESP8266Ping.h>

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

//Metering
float meteredKWh = 0;
float currentKW = 0;
uint8_t intLength = 0;

//Metering S0
uint8_t meterPin;
uint16_t meterTimeout = 6; //sec
uint16_t kwhimp;
volatile unsigned long numberOfMeterImps = 0;
unsigned long meterImpMillis = 0;
unsigned long previousMeterMillis = 0;
volatile uint8_t meterInterrupt = 0;

//Metering Modbus
unsigned long millisUpdateMMeter = 0;
float startTotal;

//SoftwareSerial and Modbus
SoftwareSerial sSerial(D1, D2); //SoftwareSerial object (RX, TX)
ModbusMaster evseNode;
ModbusMaster meterNode;

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
bool inAPMode = false;
bool inFallbackMode = false;
bool isWifiConnected = false;
String lastUsername = "";
String lastUID = "";
char * deviceHostname = NULL;
uint8_t buttonPin;
char * adminpass = NULL;
int timeZone;

MFRC522 mfrc522 = MFRC522();  // Create MFRC522 RFID instance
AsyncWebServer server(80);    // Create AsyncWebServer instance on port "80"
AsyncWebSocket ws("/ws");     // Create WebSocket instance on URL "/ws"
NtpClient NTP;

//////////////////////////////////////////////////////////////////////////////////////////
///////       Auxiliary Functions
//////////////////////////////////////////////////////////////////////////////////////////

String ICACHE_FLASH_ATTR printIP(IPAddress adress) {
  return (String)adress[0] + "." + (String)adress[1] + "." + (String)adress[2] + "." + (String)adress[3];
}

void ICACHE_FLASH_ATTR parseBytes(const char* str, char sep, byte* bytes, int maxBytes, int base) {
  for (int i = 0; i < maxBytes; i++) {
    bytes[i] = strtoul(str, NULL, base);  // Convert byte
    str = strchr(str, sep);               // Find next separator
    if (str == NULL || *str == '\0') {
      break;                            // No more separators, exit
    }
    str++;                                // Point to next character after separator
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
  delay(10);
  currentKW = readMeter(0x000C) / 1000.0;
  delay(10);
  meteredKWh = readMeter(0x0156) - startTotal;
  delay(10);
  millisUpdateMMeter = millis() + 5000;
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
  Serial.print(F("[ INFO ] PICC's UID: "));
  String uid = "";
  for (int i = 0; i < mfrc522.uid.size; ++i) {
    uid += String(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.print(uid);
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
      Serial.println(" = known PICC");
      Serial.print("[ INFO ] User Name: ");
      if (username == "undefined"){
        Serial.print(uid);
      }
      else{
        Serial.print(username);
      }
      if (AccType == 1) {
        toActivateEVSE = true;
        previousMillis = millis();
        Serial.println(" have access");
      }
      else if (AccType == 99)
      {
        toActivateEVSE = true;
        previousMillis = millis();
        Serial.println(" have admin access, enable wifi");
      }
      else {
        Serial.println(" does not have access");
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
      Serial.println("");
      Serial.println(F("[ WARN ] Failed to parse User Data"));
    }
    f.close();
  }
  else { // Unknown PICC
    lastUsername = "Unknown";
    lastUID = uid;
    
    Serial.println(" = unknown PICC");
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

void ICACHE_FLASH_ATTR pushWebsocketEVSEData(){
    sendEVSEdata();
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
    Serial.println("[ ModBus ] got additional EVSE data successfully ");
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
    delay(30);
    root["meter_total"] = readMeter(0x0156);
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
      Serial.println("Impossible to read JSON file");
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
    Serial.println("Cannot create Logfile");
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
    Serial.println("Impossible to read JSON file");
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
      item["energy"] = float(int((meteredKWh + 0.05) * 100.0)) / 100.0;
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
  Serial.println("");

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
    Serial.println("[ ModBus ] got EVSE data successfully ");
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
          Serial.println("Vehicle interrupted charging");
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
  Serial.println("[ ModBus ] Query Modbus before activating EVSE");
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
      Serial.println("[ ModBus ] EVSE successfully activated");
      toActivateEVSE = false;
      evseActive = true;
      logLatest(lastUID, lastUsername);
      vehicleCharging = true;
      if(useMMeter){
        delay(20);
        startTotal = readMeter(0x0156);
      }
      else{
        meteredKWh = 0.0;  
      }
      numberOfMeterImps = 0;
      millisStartCharging = millis();
      sendEVSEdata();
      return true;
    }
  }
  else if (evseVehicleStatus != 0){
    Serial.println("[ Modbus ] EVSE already active!");
    toActivateEVSE = false;
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
    Serial.println("[ ModBus ] EVSE successfully deactivated");

    if(useMMeter){
      meteredKWh = readMeter(0x0156) - startTotal;
    }
    toDeactivateEVSE = false;
    evseActive = false;
    if(logUpdate){
      updateLog(false);
    }
    sendEVSEdata();
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
    Serial.println("[ ModBus ] Current successfully set");
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
    Serial.println("[ ModBus ] Register " + (String)reg + " successfully set to " + (String)val);
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
  Serial.println("TimeOut sent to browser!");
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
    Serial.printf("[ WARN ] WebSocket[%s][%u] error(%u): %s\r\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
  }
  else if (type == WS_EVT_DATA) {
    AwsFrameInfo * info = (AwsFrameInfo*)arg;
    String msg = "";
    if (info->final && info->index == 0 && info->len == len) {
      //the whole message is in a single frame and we got all of it's data
      for (size_t i = 0; i < info->len; i++) {
        msg += (char) data[i];
      }
      DynamicJsonBuffer jsonBuffer8;
      JsonObject& root = jsonBuffer8.parseObject(msg);
      if (!root.success()) {
        Serial.println(F("[ WARN ] Couldn't parse WebSocket message"));
        return;
      }
      
      const char * command = root["command"];
      if (strcmp(command, "remove")  == 0) {
        const char* uid = root["uid"];
        String filename = "/P/";
        filename += uid;
        SPIFFS.remove(filename);
      }
      else if (strcmp(command, "configfile")  == 0) {
        File f = SPIFFS.open("/config.json", "w+");
        if (f) {
          root.prettyPrintTo(f);
          //f.print(msg);
          f.close();
          if(vehicleCharging){
            deactivateEVSE(true);
          }
          ESP.reset();
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
          Serial.println("[ DEBUG ] Userfile written!");
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
        Serial.println("sendEVSEdata");
        toQueryEVSE = true;
      }
      else if (strcmp(command, "setcurrent") == 0){
        currentToSet = root["current"];
        Serial.print("Call setEVSECurrent() ");
        Serial.println(currentToSet);
        toSetEVSEcurrent = true;
      }
      else if (strcmp(command, "activateevse") == 0){
        toActivateEVSE = true;
        Serial.println("Activate EVSE via WebSocket");
        lastUID = "-";
        lastUsername = "GUI";
      }
      else if (strcmp(command, "deactivateevse") == 0){
        toDeactivateEVSE = true;
        Serial.println("Deactivate EVSE via WebSocket");
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
    }
  }
}

//////////////////////////////////////////////////////////////////////////////////////////
///////       Setup Functions
//////////////////////////////////////////////////////////////////////////////////////////
void ICACHE_FLASH_ATTR ShowReaderDetails() {
  // Get the MFRC522 software version
  byte v = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  Serial.print(F("[ INFO ] MFRC522 Version: 0x"));
  Serial.print(v, HEX);
  if (v == 0x91)
    Serial.print(F(" = v1.0"));
  else if (v == 0x92)
    Serial.print(F(" = v2.0"));
  else if (v == 0x88)
    Serial.print(F(" = clone"));
  else
    Serial.print(F(" (unknown)"));
  Serial.println("");
  // When 0x00 or 0xFF is returned, communication probably failed
  if ((v == 0x00) || (v == 0xFF)) {
    Serial.println(F("[ WARN ] Communication failure, check if MFRC522 properly connected"));
  }
}

void ICACHE_FLASH_ATTR setupRFID(int rfidss, int rfidgain) {
  SPI.begin();           // MFRC522 Hardware uses SPI protocol
    mfrc522.PCD_Init(rfidss, UINT8_MAX);    // Initialize MFRC522 Hardware
    mfrc522.PCD_SetAntennaGain(rfidgain);
    Serial.printf("[ INFO ] RFID SS_PIN: %u and Gain Factor: %u", rfidss, rfidgain);
    Serial.println("");
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
    Serial.print(F("."));
  }
  while (millis() - now < timeout * 1000);
  if (WiFi.status() == WL_CONNECTED) {
    isWifiConnected = true;
    return true;
  }
  else {
    Serial.println();
    Serial.println(F("[ WARN ] Couldn't connect in time"));
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
    Serial.println(F("[ WARN ] Failed to open config file"));
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

  useRFID = false;
  if (json.containsKey("rfid") &&
       json.containsKey("sspin") &&
       json.containsKey("rfidgain")){
    if (json["rfid"] == true){
      int rfidss = json["sspin"];
      int rfidgain = json["rfidgain"];
      useRFID = true;
      Serial.println(F("[ INFO ] Trying to setup RFID hardware"));
      setupRFID(rfidss, rfidgain);
    }
  }

  useMMeter = false;
  useSMeter = false;
  if(json.containsKey("meter") &&
      json.containsKey("metertype")){
    iPrice = json["price"];
    if(json["metertype"] == "m"){
      useMMeter = true;
      useSMeter = false;
      Serial.println(F("[ INFO ] Modbus Meter is configured"));
    }
    else if(json["metertype"] == "s"){
      if(json.containsKey("intpin") &&
          json.containsKey("kwhimp")){
        meterPin = json["intpin"];
        kwhimp = json["kwhimp"];
        useMMeter = false;
        useSMeter = true;
        if(json.containsKey("implen")){
          intLength = json["implen"];
          intLength += 3;
        }
        else{
          intLength = 33;
        }
        Serial.println(F("[ INFO ] S0 Meter is configured"));
      }
    }
  }

  useButton = false;
  if(json.containsKey("buttonactive")){
    if(json["buttonactive"] == true &&
        json.containsKey("buttonpin")){
      useButton = true;
      buttonPin = json["buttonpin"];
      Serial.println("[ INFO ] EVSE set to \"Button active\"");
    }
  }

  if(json.containsKey("avgconsumption")){
    String sConsumption = json["avgconsumption"];
    consumption = strtof((sConsumption).c_str(),0);
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

  ws.setAuthentication("admin", adminpass);
  server.addHandler(new SPIFFSEditor("admin", adminpass));

  queryEVSE();
  if(vehicleCharging){
    updateLog(true);
  }
  deactivateEVSE(false);  //initial deactivation
  vehicleCharging = false;

  if (wmode == 1) {
    Serial.println(F("[ INFO ] SimpleEVSE Wifi is running in AP Mode "));
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
  if(!Ping.ping("google.com", 5)){
    Serial.println("[ NTP ] Error pinging pool.ntp.org - no NTP support!");
  }
  else{ 
    Serial.println("[ NTP ] Echo response received from pool.ntp.org - set up NTP"); 
    const char * ntpserver = "pool.ntp.org";
    IPAddress timeserverip;
    WiFi.hostByName(ntpserver, timeserverip);
    String ip = printIP(timeserverip);
    NTP.Ntp(ntpserver, timeZone, 3600);   //use pool.ntp.org, timeZone, update every x minutes
  }
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
    list.add(item);
    root.printTo(*response);
    request->send(response);
  });
  
  //getLog
  server.on("/getLog", HTTP_GET, [](AsyncWebServerRequest * request) {
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/latestlog.json", "application/json");
    request->send(response);
  });
  
  //setCurrent
  server.on("/setCurrent", HTTP_GET, [](AsyncWebServerRequest * request) {
    bool suc = false;
    for(int i=0;i<request->params(); i++){
      AsyncWebParameter* p = request->getParam(i);
      if(p->name() == "current"){
        if(atoi(p->value().c_str()) <= maxinstall && atoi(p->value().c_str()) >= 6 ){
          currentToSet = atoi(p->value().c_str());
          if(setEVSEcurrent()){
            request->send(200, "text/plain", ("S0_set current to " + (String)currentToSet) + "A");
          }
          else{
            request->send(200, "text/plain", "E0_could not set current - internal error");
          }
        }
        else{
          request->send(200, "text/plain", ("E1_could not set current - give a value between 6 and " + (String)maxinstall));
        }
        break;
        suc = true;
      }
    }
    if (suc == false){
      request->send(200, "text/plain", "E2_could not set current - wrong parameter");
    }
  });
  
  //setStatus
  server.on("/setStatus", HTTP_GET, [](AsyncWebServerRequest * request) {
    bool suc = false;
    for(int i=0;i<request->params(); i++){
      AsyncWebParameter* p = request->getParam(i);
      if(p->name() == "active"){
        Serial.println(p->value().c_str());
        if(strcmp(p->value().c_str(), "true") == 0){
          if(activateEVSE()){
            request->send(200, "text/plain", "S0_EVSE successfully activated");
          }
          else{
            request->send(200, "text/plain", "E0_could not activate EVSE - internal error");
          }
        }
        else if(strcmp(p->value().c_str(), "false") == 0){
          if(deactivateEVSE(false)){
            request->send(200, "text/plain", "S0_EVSE successfully deactivated");
          }
          else{
            request->send(200, "text/plain", "E0_could not deactivate EVSE - internal error");
          }
        }
        else{
          request->send(200, "text/plain", "E1_could not process - give a valid value (true/false)");
        }
        break;
        suc = true;
      }
    }
    if (suc == false){
      request->send(200, "text/plain", "E2_could not process - wrong parameter");
    }
  });
  
}

void ICACHE_FLASH_ATTR fallbacktoAPMode() {
  WiFi.disconnect(true);
  Serial.println(F("[ INFO ] SimpleEVSE Wifi is running in Fallback AP Mode"));
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
      Serial.printf("[ UPDT ] Firmware update started: %s\n", filename.c_str());
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
        Serial.printf("[ UPDT ] Firmware update finished: %uB\n", index + len);
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
  Serial.println();
  Serial.print("[ INFO ] SimpleEVSE WiFi");
  delay(1000);
  
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
    if (meterPin == 16){
      pinMode(meterPin, INPUT_PULLDOWN_16);
      attachInterrupt(digitalPinToInterrupt(meterPin), handleMeterInt, RISING);
      Serial.println("[ Meter ] Use GPIO 16 with Pull-Down");
    }
    else{
      pinMode(meterPin, INPUT_PULLUP);
      attachInterrupt(digitalPinToInterrupt(meterPin), handleMeterInt, FALLING);
      Serial.println("[ Meter ] Use GPIO 0-15 with Pull-Up");
    }
  }
  now();
  startWebserver();
  Serial.println("End of setup routine");
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
      Serial.println(F("[ UPDT ] Auto restarting..."));
      delay(1000);
      toReboot = true;
    }
  }
  if (inFallbackMode && uptime > 600){
    toReboot = true;
  }
  if (toReboot) {
    Serial.println(F("[ UPDT ] Rebooting..."));
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
  if (useMMeter && millisUpdateMMeter < millis()){
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

