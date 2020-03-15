#include <Arduino.h>
#include <MFRC522.h>
#include <FS.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include "Ntp.h"

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
	void ICACHE_FLASH_ATTR begin(int rfidss, int rfidgain, NtpClient* ntp, bool debug);
    scanResult ICACHE_FLASH_ATTR readPicc();
    DynamicJsonDocument ICACHE_FLASH_ATTR getUserList(int page);
    unsigned long cooldown;

private:
    void ICACHE_FLASH_ATTR printReaderDetails();
    NtpClient* ntpClient;
    bool debug;

};