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
#include <ArduinoJson.h>              // JSON Library for Encoding and Parsing Json object to send browser. We do that because Javascript has built-in JSON parsing.
#include <FS.h>                       // SPIFFS Library for storing web files to serve to web browsers
#include <ESPAsyncTCP.h>              // Async TCP Library is mandatory for Async Web Server
#include <ESPAsyncWebServer.h>        // Async Web Server with built-in WebSocket Plug-in
#include <SPIFFSEditor.h>             // This creates a web page on server which can be used to edit text based files.
#include <TimeLib.h>                  // Library for converting epochtime to a date
#include <WiFiUdp.h>                  // Library for manipulating UDP packets which is used by NTP Client to get Timestamps
#include <SoftwareSerial.h>           // Using GPIOs for Serial Modbus communication
#include <ModbusMaster.h>
#include "ntp.h"

NtpClient NTP;

// Variables for whole scope
String sVersion = "0.1.0";
unsigned long previousMillis = 0;
unsigned long previousLoopMillis = 0;
unsigned long cooldown = 0;
const char * defaultWifiPassword = "evse";

//Metering
uint16_t meterTimeout = 6; //seconds
unsigned long numberOfMeterImps = 0;
unsigned long meterImpMillis = 0;
unsigned long previousMeterMillis = 0;
int interruptPin;
volatile byte meterInterrupt = 0;
float meteredKWh = 0;
float currentKW = 0;
unsigned long millisStartCharging = 0;
unsigned long millisStopCharging = 0;
bool vehicleCharging = false;
int16_t iPrice = 0;
uint8_t maxinstall = 0;

//SoftwareSerial and ModBus
SoftwareSerial mySerial(D1, D2); // Create a SoftwareSerial object - D1-RX D2-TX
ModbusMaster node;

uint8_t queryTimer = 5; // seconds
unsigned long lastModbusAnswer = 0;
unsigned long lastEVSEStateMillis = 0;
unsigned long evseQueryTimeOut = 0;
unsigned long iTime = 0;
bool evseSessionTimeOut = false;
bool evseActive = false;

uint8_t currentToSet = 6;
bool toSetEVSEcurrent = false;
bool toActivateEVSE = false;
bool toDeactivateEVSE = false;

bool toQueryModbus = false;
int8_t evseStatus = 0;

//EVSE Modbus Registers
int16_t MBAmpsConfig;      //Register 1000
int16_t MBAmpsOutput;      //Register 1001
int16_t MBVehicleStatus;   //Register 1002
int16_t MBAmpsPP;          //Register 1003
int16_t MBTurnOffCharging; //Register 1004
int16_t MBFirmware;        //Register 1005
int16_t MBEVSEStatus;      //Register 1006

//Others
bool shouldReboot = false;
bool useRFID = false;
bool useMeter = false;
bool evseButtonActive = false;
bool inAPMode = false;
bool isWifiConnected = false;
int autoRestartIntervalSeconds = 0;
String lastUsername = "";
String lastUID = "";
char * deviceHostname = NULL;
int buttonPin;

int buttonState = 0;
bool toSendStatus = false;

uint16_t kwhimp;
int timeZone;

boolean syncEventTriggered = false; // True if a time even has been triggered

// Create MFRC522 RFID instance
MFRC522 mfrc522 = MFRC522();
// Create AsyncWebServer instance on port "80"
AsyncWebServer server(80);
// Create WebSocket instance on URL "/ws"
AsyncWebSocket ws("/ws");

void ICACHE_RAM_ATTR handleMeterInt() {  //interrupt routine for metering
  if(meterImpMillis < millis()){   //Meter impulse is 30ms 
    meterInterrupt ++;
    meterImpMillis = millis();
  }
}

// Set things up
void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.print("[ INFO ] SimpleEVSE WiFi ");
  Serial.println(sVersion);


  delay(2000);
  
  // Start SPIFFS filesystem
  SPIFFS.begin();

  //ModBus Master
  node.begin(1, mySerial);
  mySerial.begin(9600);
  
  if (!loadConfiguration()) {
    fallbacktoAPMode();
  }
  if(evseButtonActive){
    pinMode(buttonPin, INPUT_PULLUP);
  }
  //Metering
  if (useMeter){
    pinMode(interruptPin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(interruptPin), handleMeterInt, FALLING);
  }
  now();
  startWebserver();
}

void startWebserver() {

  // Start WebSocket Plug-in and handle incoming message on "onWsEvent" function
  server.addHandler(&ws);
  ws.onEvent(onWsEvent);
  
  // Serve all files in root folder
  server.serveStatic("/", SPIFFS, "/");
  
  // Handle what happens when requested web file couldn't be found
  server.onNotFound([](AsyncWebServerRequest * request) {
    AsyncWebServerResponse *response = request->beginResponse(404, "text/plain", "Not found");
    request->send(response);
  });

  // Simple Firmware Update Handler
  server.on("/auth/update", HTTP_POST, [](AsyncWebServerRequest * request) {
    shouldReboot = !Update.hasError();
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", shouldReboot ? "OK" : "FAIL");
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

  // Start Web Server
  server.begin();
}

// Main Loop
void loop() {
  unsigned long currentMillis = millis();
  unsigned long deltaTime = currentMillis - previousLoopMillis;
  unsigned long uptime = NTP.getUptimeSec();
  previousLoopMillis = currentMillis;

  if (autoRestartIntervalSeconds > 0 && uptime > autoRestartIntervalSeconds * 1000) {
    if(vehicleCharging == false){
      Serial.println(F("[ UPDT ] Auto restarting..."));
      delay(1000);
      shouldReboot = true;
    }
  }

  // check for a new update and restart
  if (shouldReboot) {
    Serial.println(F("[ UPDT ] Rebooting..."));
    delay(100);
    ESP.restart();
  }

  // Another loop for RFID Events, since we are using polling method instead of Interrupt we need to check RFID hardware for events
  if (currentMillis >= cooldown && useRFID == true) {
    rfidloop();
  }
  if ((currentMillis > ( lastModbusAnswer + 1000)) && //Update Modbus data every 1000ms and send data to WebUI
        toQueryModbus == true &&
        evseSessionTimeOut == false) {
    queryModbus();
    sendEVSEdata();
  }
  else if (currentMillis > evseQueryTimeOut &&    //Setting timeout for Evse poll / push to ws
          evseSessionTimeOut == false){
    evseSessionTimeOut = true;
    pushSessionTimeOut();
  }
  if(currentMillis > lastModbusAnswer + ( queryTimer * 1000 ) && evseActive == true && evseSessionTimeOut == true){ //Query modbus every x seconds, when webinterface is not shown
    queryModbus();
  }
  if (meterInterrupt){   //new Meter impulse
    updateMeterData();
  }
  if (previousMeterMillis < millis() - (meterTimeout * 1000)) {  //Timeout when there is less than ~300 watt power consuption -> 6 sec of no interrupt from meter
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
  if (evseButtonActive){
    buttonState = digitalRead(buttonPin);
    if (buttonState == LOW && evseActive == false) {
      toActivateEVSE = true;
    }
  }
  if (toSendStatus == true){
    sendStatus();
    toSendStatus = false;
  }
}

/* ------------------ RFID Functions ------------------- */
// RFID Specific Loop
void rfidloop() {
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

  // There are Mifare PICCs which have 4 byte or 7 byte UID
  // Get PICC's UID and store on a variable
  Serial.print(F("[ INFO ] PICC's UID: "));
  String uid = "";
  for (int i = 0; i < mfrc522.uid.size; ++i) {
    uid += String(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.print(uid);
  // Get PICC type
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  String type = mfrc522.PICC_GetTypeName(piccType);

  // We are going to use filesystem to store known UIDs.
  // If we know the PICC we need to know if its User have an Access
  int AccType = 0;  // First assume User do not have access
  // Prepend /P/ on filename so we distinguish UIDs from the other files
  String filename = "/P/";
  filename += uid;

  File f = SPIFFS.open(filename, "r");
  // Check if we could find it above function returns true if the file is exist
  if (f) {
    // Now we need to read contents of the file to parse JSON object contains Username and Access Status
    size_t size = f.size();
    // Allocate a buffer to store contents of the file.
    std::unique_ptr<char[]> buf(new char[size]);
    // We don't use String here because ArduinoJson library requires the input
    // buffer to be mutable. If you don't use ArduinoJson, you may as well
    // use configFile.readString instead.
    f.readBytes(buf.get(), size);
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.parseObject(buf.get());
    // Check if we succesfully parse JSON object
    if (json.success()) {
      // Get username Access Status
      String username = json["user"];
      AccType = json["acctype"];
      Serial.println(" = known PICC");
      Serial.print("[ INFO ] User Name: ");

      if (username == "undefined")
        Serial.print(uid);
      else
        Serial.print(username);

      // Check if user have an access
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
      
      // Also inform Administrator Portal
      // Encode a JSON Object and send it to All WebSocket Clients
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
      AsyncWebSocketMessageBuffer * buffer = ws.makeBuffer(len); //  creates a buffer (len + 1) for you.
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
    // If we don't know the UID, inform Administrator Portal so admin can give access or add it to database
    lastUsername = "Unknown";
    lastUID = uid;
    
    Serial.println(" = unknown PICC");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["command"] = "piccscan";
    // UID of Scanned RFID Tag
    root["uid"] = uid;
    // Type of PICC
    root["type"] = type;
    root["known"] = 0;
    size_t len = root.measureLength();
    AsyncWebSocketMessageBuffer * buffer = ws.makeBuffer(len); //  creates a buffer (len + 1) for you.
    if (buffer) {
      root.printTo((char *)buffer->get(), len + 1);
      ws.textAll(buffer);
    }
  }
  // So far got we got UID of Scanned RFID Tag, checked it if it's on the database and access status, informed Administrator Portal
}


// Handles WebSocket Events
void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) {
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

      // We should always get a JSON object (stringfied) from browser, so parse it
      DynamicJsonBuffer jsonBuffer8;
      JsonObject& root = jsonBuffer8.parseObject(msg);
      if (!root.success()) {
        Serial.println(F("[ WARN ] Couldn't parse WebSocket message"));
        return;
      }

      // Web Browser sends some commands, check which command is given
      const char * command = root["command"];

      // Check whatever the command is and act accordingly
      if (strcmp(command, "remove")  == 0) {
        const char* uid = root["uid"];
        String filename = "/P/";
        filename += uid;
        SPIFFS.remove(filename);
      }
      else if (strcmp(command, "configfile")  == 0) {
        File f = SPIFFS.open("/auth/config.json", "w+");
        if (f) {
          root.prettyPrintTo(f);
          //f.print(msg);
          f.close();
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
        }
        f.close();
        ws.textAll("{\"command\":\"result\",\"resultof\":\"userfile\",\"result\": true}");
      }
      else if (strcmp(command, "latestlog")  == 0) {
        File logFile = SPIFFS.open("/auth/latestlog.json", "r");
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
        File configFile = SPIFFS.open("/auth/config.json", "r");
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
        toQueryModbus = true;
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
    }
  }
}

void LogLatest(String uid, String username) {
  File logFile = SPIFFS.open("/auth/latestlog.json", "r");
  if (!logFile) {
    // Can not open file create it.
    File logFile = SPIFFS.open("/auth/latestlog.json", "w");
    DynamicJsonBuffer jsonBuffer3;
    JsonObject& root = jsonBuffer3.createObject();
    root["type"] = "latestlog";
    JsonArray& list = root.createNestedArray("list");
    root.printTo(logFile);
    logFile.close();
    
    logFile = SPIFFS.open("/auth/latestlog.json", "r");
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
      File logFile = SPIFFS.open("/auth/latestlog.json", "w");
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

void sendEVSEdata(){
  // Send EVSE-Data to UI
  // Encode a JSON Object and send it to All WebSocket Clients
  if (evseSessionTimeOut == false){
    DynamicJsonBuffer jsonBuffer9;
    JsonObject& root = jsonBuffer9.createObject();
    root["command"] = "getevsedata";
    root["evse_vehicle_state"] = evseStatus;
    root["evse_active"] = evseActive;
    root["evse_current_limit"] = MBAmpsConfig;
    root["evse_current"] = String(currentKW, 1);
    root["evse_charging_time"] = getChargingTime();
    root["evse_charged_kwh"] = String(meteredKWh, 2);
    root["evse_maximum_current"] = maxinstall;
    
    size_t len = root.measureLength();
    AsyncWebSocketMessageBuffer * buffer = ws.makeBuffer(len); //  creates a buffer (len + 1) for you.
    if (buffer) {
      root.printTo((char *)buffer->get(), len + 1);
     ws.textAll(buffer);
    }
  }
}

void sendTime() {
  DynamicJsonBuffer jsonBuffer10;
  JsonObject& root = jsonBuffer10.createObject();
  root["command"] = "gettime";
  root["epoch"] = now();
  root["timezone"] = timeZone;
  size_t len = root.measureLength();
  AsyncWebSocketMessageBuffer * buffer = ws.makeBuffer(len); //  creates a buffer (len + 1) for you.
  if (buffer) {
    root.printTo((char *)buffer->get(), len + 1);
    ws.textAll(buffer);
  }
}

void sendUserList(int page, AsyncWebSocketClient * client) {
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
      // We don't use String here because ArduinoJson library requires the input
      // buffer to be mutable. If you don't use ArduinoJson, you may as well
      // use configFile.readString instead.
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

#ifdef ESP8266
extern "C" {
#include "user_interface.h"  // Used to get Wifi status information
}
#endif

void sendStatus() {
  // Getting additional Modbus data
  uint8_t result;
  uint16_t evse_amps_afterboot;     //Register 2000
  uint16_t evse_modbus_enabled;     //Register 2001
  uint16_t evse_amps_min;           //Register 2002
  uint16_t evse_pp_detection;       //Register 2007
  uint16_t evse_firmware;           //Register 2009
  
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
  queryModbus();
  node.clearTransmitBuffer();
  node.clearResponseBuffer();
  result = node.readHoldingRegisters(0x07D0, 10);  // read 10 registers starting at 0x07D0 (2000)
  
  if (result != 0){
    // error occured
    MBVehicleStatus = 0;
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
        evse_amps_afterboot  = node.getResponseBuffer(i);    //Register 2000
        break;
      case 1:
        evse_modbus_enabled = node.getResponseBuffer(i);     //Register 2001
        break;
      case 2:
        evse_amps_min = node.getResponseBuffer(i);           //Register 2002
        break;
      case 7:
        evse_pp_detection = node.getResponseBuffer(i);       //Register 2007
        break;
      case 9:
        evse_firmware = node.getResponseBuffer(i);           //Register 2009
        break;    
      }
    }
  }

  root["evse_amps_conf"] = MBAmpsConfig;          //Reg 1000
  root["evse_amps_out"] = MBAmpsOutput;           //Reg 1001
  root["evse_vehicle_state"] = MBVehicleStatus;   //Reg 1002
  root["evse_pp_limit"] = MBAmpsPP;               //Reg 1003
  root["evse_firmware"] = MBFirmware;             //Reg 1005
  root["evse_state"] = MBEVSEStatus;              //Reg 1006
    
  root["evse_amps_afterboot"] = evse_amps_afterboot;    //Reg 2000
  root["evse_modbus_enabled"] = evse_modbus_enabled;    //Reg 2001
  root["evse_amps_min"] = evse_amps_min;                //Reg 2002
  root["evse_pp_detection"] = evse_pp_detection;        //Reg 2007
  
  size_t len = root.measureLength();
  AsyncWebSocketMessageBuffer * buffer = ws.makeBuffer(len); //  creates a buffer (len + 1) for you.
  if (buffer) {
    root.printTo((char *)buffer->get(), len + 1);
    ws.textAll(buffer);
  }
}

String printIP(IPAddress adress) {
  return (String)adress[0] + "." + (String)adress[1] + "." + (String)adress[2] + "." + (String)adress[3];
}

// Send Scanned SSIDs to websocket clients as JSON object
void printScanResult(int networksFound) {
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

bool startAP(const char * ssid, const char * password = NULL) {
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

// Fallback to AP Mode, so we can connect to ESP if there is no Internet connection
void fallbacktoAPMode() {
  Serial.println(F("[ INFO ] SimpleEVSE Wifi is running in Fallback AP Mode"));
  uint8_t macAddr[6];
  WiFi.softAPmacAddress(macAddr);
  char ssid[15];
  sprintf(ssid, "evse-wifi-%02x%02x%02x", macAddr[3], macAddr[4], macAddr[5]);
  isWifiConnected = startAP(ssid, defaultWifiPassword);
  server.serveStatic("/auth/", SPIFFS, "/auth/").setDefaultFile("evse.htm").setAuthentication("admin", "admin");
}

void parseBytes(const char* str, char sep, byte* bytes, int maxBytes, int base) {
  for (int i = 0; i < maxBytes; i++) {
    bytes[i] = strtoul(str, NULL, base);  // Convert byte
    str = strchr(str, sep);               // Find next separator
    if (str == NULL || *str == '\0') {
      break;                            // No more separators, exit
    }
    str++;                                // Point to next character after separator
  }
}

bool loadConfiguration() {
  File configFile = SPIFFS.open("/auth/config.json", "r");
  if (!configFile) {
    Serial.println(F("[ WARN ] Failed to open config file"));
    return false;
  }
  size_t size = configFile.size();
  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);
  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable. If you don't use ArduinoJson, you may as well
  // use configFile.readString instead.
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

  //Handle wheather RFID hardware is present or not
  if (json["rfid"] == true){
    useRFID = true;
    int rfidss = 15;
    if (json.containsKey("sspin")) {
      rfidss = json["sspin"];
    }
    int rfidgain = json["rfidgain"];
    Serial.println(F("[ INFO ] Trying to setup RFID hardware"));
    setupRFID(rfidss, rfidgain);
  }
  else{
    Serial.println(F("[ INFO ] No RFID hardware is configured"));
    useRFID = false;
  }

  //Handle wheather meter hardware is prensent or not
  if(json["meter"] == true){
    useMeter = true;
    interruptPin = json["intpin"];
  }
  else{
    useMeter = false;
    Serial.println(F("[ INFO ] No meter hardware is configured"));
  }

  evseButtonActive = json["buttonactive"];
  if (evseButtonActive == true){
    buttonPin = json["buttonpin"];
    Serial.println("[ INFO ] EVSE set to \"Button active\"");
  }
  const char * l_hostname = json["hostnm"];
  autoRestartIntervalSeconds = json["auto_restart_interval_seconds"].as<int>();
  
  free(deviceHostname);
  deviceHostname = strdup(l_hostname);
  const char * bssidmac = json["bssid"];
  byte bssid[6];
  parseBytes(bssidmac, ':', bssid, 6, 16);

  // Set Hostname.
  WiFi.hostname(deviceHostname);

  // Start mDNS service (if Bonjour installed on Windows or Avahi on Linux)
  if (!MDNS.begin(deviceHostname)) {
    Serial.println("Error setting up MDNS responder!");
  }
  // Add Web Server service to mDNS
  MDNS.addService("http", "tcp", 80);
  
  timeZone = json["timezone"];
  kwhimp = json["kwhimp"];
  iPrice = json["price"];
  maxinstall = json["maxinstall"];

  const char * ssid = json["ssid"];
  const char * password = json["pswd"];
  int wmode = json["wmode"];
  const char * adminpass = json["adminpwd"];

  // Serve confidential files in /auth/ folder with a Basic HTTP authentication
  server.serveStatic("/auth/", SPIFFS, "/auth/").setDefaultFile("evse.htm").setAuthentication("admin", adminpass);
  //server.serveStatic("/", SPIFFS, "/");
  ws.setAuthentication("admin", adminpass);
  // Add Text Editor (http://hostname/edit) to Web Server. This feature likely will be dropped on final release.
  server.addHandler(new SPIFFSEditor("admin", adminpass));

  if (wmode == 1) {
    Serial.println(F("[ INFO ] SimpleEVSE Wifi is running in AP Mode "));
    return startAP(ssid, password);
  }
  else if (!connectSTA(ssid, password, bssid)) {
    return false;
  }

  deactivateEVSE(false);  //Initial deactivation
  
  const char * ntpserver = "pool.ntp.org";
  IPAddress timeserverip;
  WiFi.hostByName(ntpserver, timeserverip);
  String ip = printIP(timeserverip);
  NTP.Ntp(ntpserver, timeZone, 3600);   //use pool.ntp.org, timeZone, update every x seconds
  
  return true;
}

// Configure RFID Hardware
void setupRFID(int rfidss, int rfidgain) {
  SPI.begin();           // MFRC522 Hardware uses SPI protocol
    mfrc522.PCD_Init(rfidss, UINT8_MAX);    // Initialize MFRC522 Hardware
    Serial.println("PCD_Init successfully done");
    // Set RFID Hardware Antenna Gain
    // This may not work with some boards
    mfrc522.PCD_SetAntennaGain(rfidgain);

    Serial.printf("[ INFO ] RFID SS_PIN: %u and Gain Factor: %u", rfidss, rfidgain);
    Serial.println("");
    ShowReaderDetails(); // Show details of PCD - MFRC522 Card Reader details
}

// Try to connect Wi-Fi
bool connectSTA(const char* ssid, const char* password, byte bssid[6]) {

  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  // First connect to a wi-fi network
  WiFi.begin(ssid, password, 0, bssid);
  // Inform user we are trying to connect
  Serial.print(F("[ INFO ] Trying to connect WiFi: "));
  Serial.print(ssid);

  // We try it for 20 seconds and give up on if we can't connect
  unsigned long now = millis();
  uint8_t timeout = 20; // define when to time out in seconds
  // Wait until we connect or 20 seconds pass
  do {
    if (WiFi.status() == WL_CONNECTED) {
      break;
    }
    delay(500);
    Serial.print(F("."));
  }
  while (millis() - now < timeout * 1000);
  // We now out of the while loop, either time is out or we connected. check what happened
  if (WiFi.status() == WL_CONNECTED) { // Assume time is out first and check
    Serial.println();
    Serial.print(F("[ INFO ] Client IP address: ")); // Great, we connected, inform
    Serial.println(WiFi.localIP());
    isWifiConnected = true;
    return true;
  }
  else { // We couln't connect, time is out, inform
    Serial.println();
    Serial.println(F("[ WARN ] Couldn't connect in time"));
    return false;
  }
}

void ShowReaderDetails() {
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

void updateMeterData() {
  if (vehicleCharging){
    currentKW = 3600.0 / float(meterImpMillis - previousMeterMillis) / float(kwhimp / 1000) ;  //Calculating kW
    previousMeterMillis = meterImpMillis;
    meterImpMillis = meterImpMillis + 35;
    meterInterrupt = 0;
    numberOfMeterImps ++;
    meteredKWh = float(numberOfMeterImps) / float(kwhimp / 1000) / 1000.0;
  }
}

void pushWebsocketEVSEData(){
    sendEVSEdata();
}

int getChargingTime(){
  if(vehicleCharging == true){
    iTime = millis() - millisStartCharging;
  }
  else {
    iTime = millisStopCharging - millisStartCharging;
  }
  return iTime;
}

void pushSessionTimeOut(){
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

void queryModbus(){
  //New ModBus Master Library
  uint8_t result;
  
  node.clearTransmitBuffer();
  node.clearResponseBuffer();
  result = node.readHoldingRegisters(0x03E8, 7);  // read 7 registers starting at 0x03E8 (1000)
  
  if (result != 0){
    // error occured
    MBVehicleStatus = 0;
    Serial.print("[ ModBus ] Error ");
    Serial.print(result, HEX);
    Serial.println(" occured while getting EVSE data - trying again...");
    node.clearTransmitBuffer();
    node.clearResponseBuffer();
    //delay(10);
    return;
  }
  else{
    // register successufully read
    Serial.println("[ ModBus ] got EVSE data successfully ");
    lastModbusAnswer = millis();

    //process answer
    for(int i = 0; i < 7; i++){
      switch(i){
      case 0:
        MBAmpsConfig = node.getResponseBuffer(i);     //Register 1000
        break;
      case 1:
        MBAmpsOutput = node.getResponseBuffer(i);     //Register 1001
        break;
      case 2:
        MBVehicleStatus = node.getResponseBuffer(i);   //Register 1002
        break;
      case 3:
        MBAmpsPP = node.getResponseBuffer(i);          //Register 1003
        break;
      case 4:
        MBTurnOffCharging = node.getResponseBuffer(i); //Register 1004
        break;
      case 5:
        MBFirmware = node.getResponseBuffer(i);        //Register 1005
        break;
      case 6:
        MBEVSEStatus = node.getResponseBuffer(i);      //Register 1006
        break;
      }
    }
    lastEVSEStateMillis = millis();
    if (MBVehicleStatus == 0){
      evseStatus = 0; //modbus communication failed
    }
    if (MBEVSEStatus == 3){     //EVSE not Ready
        if (MBVehicleStatus == 2 ||
            MBVehicleStatus == 3 ){
          evseStatus = 2; //vehicle detected
        }
        else{
          evseStatus = 1; // EVSE deactivated
        }
        if (vehicleCharging == true){   //vehicle interrupted charging
          vehicleCharging = false;
          millisStopCharging = millis();
          Serial.println("Vehicle interrupted charging");
          updateLog();
        }
        evseActive = false;
        return;
    }
    if (MBVehicleStatus == 1){
      evseStatus = 1;  // ready
    }
    else if (MBVehicleStatus == 2){
      evseStatus = 2; //vehicle detected
    }
    else if (MBVehicleStatus == 3){
      evseStatus = 3; //charging
      if (vehicleCharging == false) {
        vehicleCharging = true;
      }
    }
  }
}

void activateEVSE() {
  static uint16_t iTransmit;
  Serial.println("[ ModBus ] Query Modbus before activating EVSE");
  queryModbus();
  
  if (MBEVSEStatus == 3 &&
      MBVehicleStatus != 0){    //no modbus error occured
      iTransmit = 8192;  // disable EVSE after charge
      
    uint8_t result;
    node.clearTransmitBuffer();
    node.setTransmitBuffer(0, iTransmit); // set word 0 of TX buffer (bits 15..0)
    result = node.writeMultipleRegisters(0x07D5, 1);  // write register 0x07D5 (2005)
  
    if (result != 0){
      // error occured
      Serial.print("[ ModBus ] Error ");
      Serial.print(result, HEX);
      Serial.println(" occured while activating EVSE - trying again...");
    }
    else{
      // register successufully written
      Serial.println("[ ModBus ] EVSE successfully activated");
      toActivateEVSE = false;
      evseActive = true;
      LogLatest(lastUID, lastUsername);
      vehicleCharging = true;
      meteredKWh = 0.0;
      numberOfMeterImps = 0;
      millisStartCharging = millis();
    }
    return;
  }
}

void deactivateEVSE(bool logUpdate) {
  //New ModBus Master Library
  static uint16_t iTransmit = 16384;  // deactivate evse
  uint8_t result;
  
  node.clearTransmitBuffer();
  node.setTransmitBuffer(0, iTransmit); // set word 0 of TX buffer (bits 15..0)
  result = node.writeMultipleRegisters(0x07D5, 1);  // write register 0x07D5 (2005)
  
  if (result != 0){
    // error occured
    Serial.print("[ ModBus ] Error ");
    Serial.print(result, HEX);
    Serial.println(" occured while deactivating EVSE - trying again...");
  }
  else{
    // register successufully written
    Serial.println("[ ModBus ] EVSE successfully deactivated");
    toDeactivateEVSE = false;
    evseActive = false;
    if(logUpdate){
      updateLog();
    } 
  }
}

void setEVSEcurrent(){  // telegram 1: write EVSE current
  //New ModBus Master Library
  uint8_t result;
  
  node.clearTransmitBuffer();
  node.setTransmitBuffer(0, currentToSet); // set word 0 of TX buffer (bits 15..0)
  result = node.writeMultipleRegisters(0x03E8, 1);  // write register 0x03E8 (1000 - Actual configured amps value)
  
  if (result != 0){
    // error occured
    Serial.print("[ ModBus ] Error ");
    Serial.print(result, HEX);
    Serial.println(" occured while setting current in EVSE - trying again...");
  }
  else{   
    // register successufully written
    Serial.println("[ ModBus ] Current successfully set");
    MBAmpsConfig = currentToSet;  //foce update in WebUI
    sendEVSEdata();               //foce update in WebUI
    toSetEVSEcurrent = false;
  } 
}

void updateLog() {
File logFile = SPIFFS.open("/auth/latestlog.json", "r");
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
    File logFile = SPIFFS.open("/auth/latestlog.json", "w");
    DynamicJsonBuffer jsonBuffer7;
    JsonObject& item = jsonBuffer7.createObject();
    item["uid"] = uid;
    item["username"] = username;
    item["timestamp"] = timestamp;
    item["duration"] = getChargingTime();
    item["energy"] = String(meteredKWh, 2);
    item["price"] = iPrice;
    list.add(item);
    root.printTo(logFile);
  }
  logFile.close();
}

