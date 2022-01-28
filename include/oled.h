#ifndef ESP8266
#include <Arduino.h>
//#include <U8g2lib.h>
#include "oled_templates.h"
#include "ntp.h"
#include "config.h"
#include <clib/u8g2.h>
#include "u8g2_esp32_hal.h"
#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>

class EvseWiFiOled {

public:
    bool displayOn;
    //void begin(U8G2_SSD1327_WS_128X128_F_4W_HW_SPI*, uint8_t rotation);
    void begin(u8g2_t * u8g2, uint8_t rotation);
    void setLanguage(EvseWiFiConfig*);
    void clearBuffer();
    void sendBuffer();
    void oledLoop();
    void turnOff();
    void turnOn();
    void showDemo(uint8_t evseStatus, unsigned long chargingTime, uint16_t current, uint8_t maxcurrent, float power, float energy, time_t time, String* version, bool active, bool timerActive, int8_t rssi);
    void showCheck(bool, uint8_t);
    void showSplash(String text = "Loading...");

private:
    //U8G2_SSD1327_WS_128X128_F_4W_HW_SPI* u8g2;
    u8g2_t * u8g2;
    void drawCheck(uint8_t);
    void drawUncheck();
    uint8_t offsetX = 0;
    uint8_t offsetY = 0;
    unsigned long millisRandOffset = 0;
    uint8_t language;
    String getTranslation(uint8_t, uint8_t);
    bool blinkWifi0 = false;
};
#endif