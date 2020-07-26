#include <Arduino.h>
#include <MFRC522.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h> 
#include <Wire.h>
#include <SPI.h>
//#include <Adafruit_PN532.h> 
#include "ntp.h"

#ifdef ESP8266
#include <FS.h>
#else
#include <SPIFFS.h>
#endif

struct scanResult {
    String uid = "";
    String type = "";
    String user = "";
    bool read = false;
    bool known = false;
    bool valid = false;
};

class EvseWiFiRfid {
public:
	bool ICACHE_FLASH_ATTR begin(int rfidss, bool usePN532, int rfidgain, NtpClient* ntp, bool debug);
    scanResult ICACHE_FLASH_ATTR readPicc();
    bool ICACHE_FLASH_ATTR performSelfTest();
    bool ICACHE_FLASH_ATTR reset();
    DynamicJsonDocument ICACHE_FLASH_ATTR getUserList(int page);
    unsigned long cooldown;

private:
    void ICACHE_FLASH_ATTR printReaderDetails();
    NtpClient* ntpClient;
    bool debug;
    bool usePN532;
};