#ifndef ESP8266
#include <Arduino.h>
#include <U8g2lib.h>
#include "oled_templates.h"
#include "ntp.h"
#include "config.h"

class EvseWiFiOled {

public:
    bool displayOn;
//U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ 16, /* data=*/ 17);
    void begin(U8G2_SSD1306_128X64_NONAME_F_HW_I2C*, uint8_t rotation);
    void setLanguage(EvseWiFiConfig*);
    void clearBuffer();
    void sendBuffer();
    void oledLoop();
    void turnOff();
    void turnOn();
    void showDemo(uint8_t evseStatus, unsigned long chargingTime, uint16_t current, uint8_t maxcurrent, uint8_t phases, float power, float energy, time_t time, String* version, bool active, bool timerActive);
    void showCheck(bool, uint8_t);
    void showSplash(String text = "", String head = "Loading...");

private:
    U8G2_SSD1306_128X64_NONAME_F_HW_I2C* u8g2;
    void drawCheck(uint8_t);
    void drawUncheck();
    uint8_t offsetX = 0;
    uint8_t offsetY = 0;
    unsigned long millisRandOffset = 0;
    uint8_t language;
    String getTranslation(uint8_t, uint8_t);
};
#endif