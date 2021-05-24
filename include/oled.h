#ifdef OLED
#include <Arduino.h>
#include <U8g2lib.h>
#include "oled_templates.h"
#include "ntp.h"

class EvseWiFiOled {

public:
    bool displayOn;

    void begin(U8G2_SSD1327_WS_128X128_F_4W_HW_SPI*, uint8_t rotation);
    void clearBuffer();
    void sendBuffer();
    void oledLoop();
    void turnOff();
    void turnOn();
    void showDemo(uint8_t evseStatus, unsigned long chargingTime, uint16_t current, uint8_t maxcurrent, float power, float energy, time_t time, String* version, bool active, bool timerActive);
    void showCheck(bool, uint8_t);
    void showSplash(String text = "Loading...");

private:
    U8G2_SSD1327_WS_128X128_F_4W_HW_SPI* u8g2;
    void drawCheck(uint8_t);
    void drawUncheck();
    uint8_t offsetX = 0;
    uint8_t offsetY = 0;
    unsigned long millisRandOffset = 0;
};
#endif