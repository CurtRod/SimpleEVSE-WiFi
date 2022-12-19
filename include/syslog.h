#ifndef SYSLOG_H_
#define SYSLOG_H_

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "ntp.h"
#include <deque>
#include <string.h>

#ifdef ESP8266
#include <FS.h>
#else
#include <SPIFFS.h>
#endif

class Syslog {
    public:
    bool ICACHE_FLASH_ATTR begin(AsyncWebSocket*, bool, std::deque<String>*);
    bool ICACHE_FLASH_ATTR logln(String);
    bool ICACHE_FLASH_ATTR logln(int, bool = false);
    bool ICACHE_FLASH_ATTR log(String);
    bool ICACHE_FLASH_ATTR log(int, bool = false);
    
    private: 
    bool ICACHE_FLASH_ATTR print(String, bool, bool);
    AsyncWebSocket* ws;
    bool debug;
    bool ICACHE_FLASH_ATTR send(String);
    std::deque<String>* syslogDeque;
};
#endif //SYSLOG_H