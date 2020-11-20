#ifndef ESP8266
#include <Arduino.h>
#include <U8g2lib.h>
#include "oled_templates.h"
#include "ntp.h"

class EvseWiFiOled {

public:
    void begin(U8G2_SSD1327_WS_128X128_F_4W_HW_SPI*, uint8_t rotation);
    void clearBuffer();
    void sendBuffer();
    void showDemo(uint8_t evseStatus, unsigned long chargingTime, uint8_t current, uint8_t maxcurrent, float power, float energy, time_t time, String* version);
    void showLock(bool locked);

private:
    U8G2_SSD1327_WS_128X128_F_4W_HW_SPI* u8g2;
    void drawLock();
    void drawUnlock();
};
#endif