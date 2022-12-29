#include <rfid.h>

#ifdef RFID
MFRC522 mfrc522 = MFRC522();


bool ICACHE_FLASH_ATTR EvseWiFiRfid::begin(int rfidss, bool usePN532, int rfidgain, NtpClient* ntp, bool debug, Syslog* s) {
  this->debug = debug;
  this->ntpClient = ntp;
  this->slog = s;
  mfrc522.PCD_SetAntennaGain(rfidgain);
  delay(50);
  mfrc522.PCD_Init(rfidss, 0);
  delay(50);
  if (this->debug) this->slog->log(this->ntpClient->iso8601DateTime() + "[ INFO ] RFID SS_PIN:");
  if (this->debug) this->slog->log(rfidss);
  if (this->debug) this->slog->log("and Gain Factor: ");
  if (this->debug) this->slog->logln(rfidgain);
  delay(50);
  printReaderDetails();

  return true;
}
#endif

void ICACHE_FLASH_ATTR EvseWiFiRfid::printReaderDetails() {
#ifdef RFID
  // Get the MFRC522 software version 
  byte v = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  if (this->debug) this->slog->log(this->ntpClient->iso8601DateTime() + "[ INFO ] MFRC522 Version: 0x");
  if (this->debug) this->slog->log(v, true);
  if (v == 0x91) {
    if (this->debug) this->slog->log(" = v1.0");
  }
  else if (v == 0x92) {
    if (this->debug) this->slog->log(" = v2.0");
  }
  else if (v == 0x88) {
    if (this->debug) this->slog->log(" = clone");
  }
  else {
    if (this->debug) this->slog->log(" (unknown)");
  }
  if (this->debug) this->slog->logln("");
  // When 0x00 or 0xFF is returned, communication probably failed
  if ((v == 0x00) || (v == 0xFF)) {
    if (this->debug) this->slog->logln("[ WARN ] Communication failure, check if MFRC522 properly connected");
  }
  #endif
}

scanResult ICACHE_FLASH_ATTR EvseWiFiRfid::readPicc() {
  scanResult res;
  #ifdef RFID
  //RC522
    if (! mfrc522.PICC_IsNewCardPresent()) {
      delay(50);
      res.read = false;
      this->cooldown = millis() + 50;
      return res;
    }
    if (! mfrc522.PICC_ReadCardSerial()) {
      delay(50);
      res.read = false;
      this->cooldown = millis() + 50;
      return res;
    }
    res.read = true;
    this->slog->logln(this->ntpClient->iso8601DateTime() + "[ RFID ] Card detected to read!");
    mfrc522.PICC_HaltA();
    this->cooldown = millis() + 3000;
    if (this->debug) this->slog->log(this->ntpClient->iso8601DateTime() +"[ RFID ] PICC's UID: ");
    String uid = "";
    for (int i = 0; i < mfrc522.uid.size; ++i) {
      uid += String(mfrc522.uid.uidByte[i], HEX);
    }
    
    res.uid = uid;
    if (this->debug) this->slog->log(uid);
    MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
    String type = mfrc522.PICC_GetTypeName(piccType);
    res.type = type;
  
    //SPIFFS.begin();
    int AccType = 0;
    String filename = "/P/";
    filename += res.uid;
    File rfidFile = SPIFFS.open(filename, "r");
    #ifdef ESP8266
    if (rfidFile)  // Known PICC
    #else
    if (SPIFFS.exists(filename)) 
    #endif
    {
      res.known = true;
      String jsonString = "";
      while (rfidFile.available()) {
        jsonString += char(rfidFile.read());
      }
      StaticJsonDocument<200> jsonDoc;
      DeserializationError error = deserializeJson(jsonDoc, jsonString);
      if (!error) {
        res.user = strdup(jsonDoc["user"]);
        AccType = jsonDoc["acctype"];
        if (this->debug) this->slog->logln(" = known PICC");
        if (this->debug) this->slog->log(this->ntpClient->iso8601DateTime() + "[ INFO ] User Name: ");
        if (res.user == "undefined") {
          if (this->debug) this->slog->log(res.uid);
        }
        else {
          if (this->debug) this->slog->log(res.user);
        }
        if ((AccType == 1 || AccType == 99) &&
            ntpClient->getUtcTimeNow() < jsonDoc["validuntil"]) {
          if (this->debug) this->slog->logln(" has permission");
          res.valid = true;
        }
        else {
          if (this->debug) this->slog->logln(" does not have permission");
        }
      }
      else {
        if (this->debug) this->slog->logln("");
        if (this->debug) this->slog->logln("[ WARN ] Failed to parse User Data");
      }
    }
    else { // Unknown PICC
      if (this->debug) this->slog->logln(" = unknown PICC");
    }
    rfidFile.close();
    //SPIFFS.end();
#endif
 return res;
}

DynamicJsonDocument ICACHE_FLASH_ATTR EvseWiFiRfid::getUserList(int page) {
  this->slog->log("getUserlist - Page: ");
  this->slog->logln(page);
  DynamicJsonDocument jsonDoc(3000);
  jsonDoc["command"] = "userlist";
  jsonDoc["page"] = page;
  JsonArray users = jsonDoc.createNestedArray("list");
  //SPIFFS.begin();
  #ifdef ESP8266
  Dir dir = SPIFFS.openDir("/P/");
  #else
  File dir = SPIFFS.open("/P");
  #endif
  int first = (page - 1) * 15;
  int last = page * 15;
  int i = 0;
  #ifdef ESP8266
  while (dir.next()) {
  #else
  File file = dir.openNextFile();
  while (file) {
  #endif
    if (i >= first && i < last) {
      JsonObject item = users.createNestedObject();
      #ifdef ESP8266
      String uid = dir.fileName();
      File f = SPIFFS.open(dir.fileName(), "r");
      #else
      String uid = file.name();
      File f = SPIFFS.open(file.name(), "r");
      #endif
      uid.remove(0, 3);
      item["uid"] = uid;
      size_t size = f.size();
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
    #ifdef ESP32
    file = dir.openNextFile();
    #endif
  }
  #ifndef ESP8266
  file.close();
  #endif
  //SPIFFS.end();
  serializeJson(jsonDoc, Serial);
  float pages = i / 15.0;
  jsonDoc["haspages"] = ceil(pages);
  return jsonDoc;
}

bool ICACHE_FLASH_ATTR EvseWiFiRfid::performSelfTest() {
  #ifdef RFID
  return mfrc522.PCD_PerformSelfTest();
  #else
  return false;
  #endif
}

bool ICACHE_FLASH_ATTR EvseWiFiRfid::reset() {
  #ifdef RFID
  mfrc522.PCD_Init();
  #endif
  return true;
}
