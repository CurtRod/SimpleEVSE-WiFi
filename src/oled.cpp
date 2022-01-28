#ifndef ESP8266
#include "oled.h"
#include <string>

//void EvseWiFiOled::begin(U8G2_SSD1327_WS_128X128_F_4W_HW_SPI* u8, uint8_t rotation) {
void EvseWiFiOled::begin(u8g2_t* u8, uint8_t rotation) {
  //this->u8g2->begin();

//U8G2_SSD1327_WS_128X128_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 12, /* dc=*/ 13, /* reset=*/ 33);

  u8g2_esp32_hal_t u8g2_esp32_hal = U8G2_ESP32_HAL_DEFAULT;
  u8g2_esp32_hal.clk   = GPIO_NUM_18;
	u8g2_esp32_hal.mosi  = GPIO_NUM_23;
	u8g2_esp32_hal.cs    = GPIO_NUM_12;
	u8g2_esp32_hal.dc    = GPIO_NUM_13;
	u8g2_esp32_hal.reset = GPIO_NUM_33;
  //u8g2_esp32_hal.scl = GPIO_NUM_MAX;
  //u8g2_esp32_hal.sda = GPIO_NUM_MAX;
	u8g2_esp32_hal_init(u8g2_esp32_hal);

  this->u8g2 = u8;
  u8g2_Setup_ssd1327_ws_128x128_f(
    this->u8g2,
    U8G2_R0,
    u8g2_esp32_spi_byte_cb,
    u8g2_esp32_gpio_and_delay_cb);

  u8g2_InitDisplay(this->u8g2);

/*
  switch (rotation)
  {
  case 0: 
  this->u8g2->setDisplayRotation(U8G2_R0);
    break;
  case 1: //90 degree clockwise rotation
  this->u8g2->setDisplayRotation(U8G2_R1);
    break;
  case 2: //180 degree clockwise rotation
  this->u8g2->setDisplayRotation(U8G2_R2);
    break;
  case 3: //270 degree clockwise rotation
  this->u8g2->setDisplayRotation(U8G2_R3);
    break;
  default: //No rotation
  this->u8g2->setDisplayRotation(U8G2_R0);
    break; 
  } */
}

void EvseWiFiOled::clearBuffer() {
    //this->u8g2->clearBuffer();
    u8g2_ClearBuffer(this->u8g2);
}

void EvseWiFiOled::sendBuffer() {
    //this->u8g2->sendBuffer();
    u8g2_SendBuffer(this->u8g2);
}

void EvseWiFiOled::oledLoop() {
  if (this->millisRandOffset < millis()) {
    srand (millis());
    this->offsetX = (rand()%3);
    srand (millis()-123);
    this->offsetY = (rand()%3);
    this->millisRandOffset = millis() + 60000;  // new offset in 60s
  }
}

void EvseWiFiOled::turnOff() {
  //this->u8g2->setPowerSave(1);
  u8g2_SetPowerSave(this->u8g2, 1);
  this->displayOn = false;
}

void EvseWiFiOled::turnOn() {
  //this->u8g2->setPowerSave(0);
  u8g2_SetPowerSave(this->u8g2, 0);
  this->displayOn = true;
}

void EvseWiFiOled::showDemo(uint8_t evseStatus, unsigned long chargingTime, uint16_t current, uint8_t maxcurrent, float power, float energy, time_t time, String* version, bool active, bool timerActive, int8_t rssi) {
  //this->u8g2->firstPage();
  u8g2_FirstPage(this->u8g2);

  String _time;
  if (hour(time) < 10) {
    _time += ("0" + (String)hour(time));
  }
  else {
    _time += (String)hour(time);
  }
  _time += ":";
  if (minute(time) < 10) {
    _time += ("0" + (String)minute(time));
  }
  else {
    _time += (String)minute(time);
  }

  String _date = ((String)year(time) + "-");
  if (month(time) < 10) {_date += ("0" + (String)month(time));}
  else {_date += (String)month(time);}
  _date += "-";
  if (day(time) < 10) {_date += ("0" + (String)day(time));}
  else {_date += (String)day(time);}

  String _weekday;

  // Day of the week
  switch (weekday(time))
  {
  case 1:
    _weekday = "Sunday";
    break;
  case 2:
    _weekday = "Monday";
    break;
  case 3:
    _weekday = "Tuesday";
    break;
  case 4:
    _weekday = "Wednesday";
    break;
  case 5:
    _weekday = "Thursday";
    break;
  case 6:
    _weekday = "Friday";
    break;
  case 7:
    _weekday = "Saturday";
    break;
  default:
    _weekday = "";
    break;
  } 

  String strDuration = "";
  chargingTime = chargingTime / 1000;
  uint8_t chargingTimeH = chargingTime / 3600;
  uint8_t chargingTimeM = chargingTime / 60 % 60;
  if (chargingTimeH == 0) {
    strDuration = "00";
  }
  else if (chargingTimeH < 10) {
    strDuration = "0" + (String)chargingTimeH;
  }
  else {
    strDuration = (String)chargingTimeH;
  }
  if (chargingTimeM == 0) {
    strDuration += ":00";
  }
  else if (chargingTimeM < 10) {
    strDuration += ":0" + (String)chargingTimeM;
  }
  else {
    strDuration += ":" + (String)chargingTimeM;
  }

  String strCurrent = "";
  if (current > 64) {
    strCurrent = (String)(current / 100.0);
    strCurrent = strCurrent.substring(0, strCurrent.length()-1);
  }
  else {
    strCurrent = (String)current; 
  }
  strCurrent += + " / " + (String)maxcurrent + " A";
  
  String strPower = (String)power + " kW";
  String strEnergy = (String)energy + " kWh";
  String strSwVersion = "v" + *version;

  uint8_t val_x = 0 + this->offsetX;
  uint8_t val_y = 20 + this->offsetY;
  uint8_t val_x_date = val_x + 48;
  uint8_t val_y_date1 = val_y - 8;
  uint8_t val_y_date2 = val_y + 6;

  do {
    //Date+Time
    //this->u8g2->setFont(u8g2_font_helvR14_tf);
    //this->u8g2->drawUTF8(val_x,val_y, _time.c_str());

    //this->u8g2->setFont(u8g2_font_helvR10_tf);
    //this->u8g2->drawUTF8(val_x_date, val_y_date1, _weekday.c_str());
    //this->u8g2->setFont(u8g2_font_helvR08_tf);
    //this->u8g2->drawUTF8(val_x_date, val_y_date2, _date.c_str());

    u8g2_SetFont(this->u8g2, u8g2_font_helvR14_tf);
    u8g2_DrawUTF8(this->u8g2, val_x, val_y, _time.c_str());

    u8g2_SetFont(this->u8g2, u8g2_font_helvR10_tf);
    u8g2_DrawUTF8(this->u8g2, val_x_date, val_y_date1, _weekday.c_str());
    u8g2_SetFont(this->u8g2, u8g2_font_helvR08_tf);
    u8g2_DrawUTF8(this->u8g2, val_x_date, val_y_date2, _date.c_str());

    //WiFi Signal
    if (rssi == 0) {
      if (this->blinkWifi0) {
        //this->u8g2->drawXBMP(val_x_date + 62, val_y -3, 11, 10, xbm_wifi0);
        u8g2_DrawXBMP(this->u8g2, val_x_date + 62, val_y -3, 11, 10, xbm_wifi0);
        this->blinkWifi0 = false;
      }
      else {
        this->blinkWifi0 = true;
      }
    }
    else if (rssi < -75) {
      //this->u8g2->drawXBMP(val_x_date + 62, val_y -3, 11, 9, xbm_wifi1);
      u8g2_DrawXBMP(this->u8g2, val_x_date + 62, val_y -3, 11, 9, xbm_wifi1);
    }
    else if (rssi < -60) {
      //this->u8g2->drawXBMP(val_x_date + 62, val_y -3, 11, 9, xbm_wifi2);
      u8g2_DrawXBMP(this->u8g2, val_x_date + 62, val_y -3, 11, 9, xbm_wifi2);
    }
    else if (rssi >= -60 && rssi < 0) {
      //this->u8g2->drawXBMP(val_x_date + 62, val_y -3, 11, 9, xbm_wifi3);
      u8g2_DrawXBMP(this->u8g2, val_x_date + 62, val_y -3, 11, 9, xbm_wifi3);
    }
    val_y += 9;

    //this->u8g2->setFont(u8g2_font_helvR12_tf);
    u8g2_SetFont(this->u8g2, u8g2_font_helvR12_tf);

    //this->u8g2->drawHLine(val_x,val_y, 122);
    u8g2_DrawHLine(this->u8g2, val_x,val_y, 122);
    val_y += 17;

    //Vehicle Status
    String strStatus = "";
    switch (evseStatus)
    {
    case 0:
      strStatus += "MB Error";
      break;
    case 1:
      strStatus += "Ready";
      break;
    case 2:
      strStatus += "Detected";
      break;
    case 3:
      strStatus += "Charging";
      break;
    default:
      break;
    }

    //Values
    if (active) {
      //this->u8g2->drawXBMP(val_x, val_y-12, 12, 12, xbm_unlocked);
      u8g2_DrawXBMP(this->u8g2, val_x, val_y-12, 12, 12, xbm_unlocked);
    }
    else {
      //this->u8g2->drawXBMP(val_x, val_y-12, 12, 12, xbm_locked);
      u8g2_DrawXBMP(this->u8g2, val_x, val_y-12, 12, 12, xbm_locked);
    }
    //this->u8g2->drawUTF8(val_x + 17,val_y, strStatus.c_str());
    u8g2_DrawUTF8(this->u8g2, val_x + 17,val_y, strStatus.c_str());
    val_y += 16;

    if (timerActive) {
      //this->u8g2->drawXBMP(val_x + 110, val_y-28, 12, 12, xbm_time);
      u8g2_DrawXBMP(this->u8g2, val_x + 110, val_y-28, 12, 12, xbm_time);
    }
    
    //this->u8g2->drawXBMP(val_x, val_y-12, 12, 12, xbm_flash);
    //this->u8g2->drawUTF8(val_x + 17,val_y, strCurrent.c_str());
    u8g2_DrawXBMP(this->u8g2, val_x, val_y-12, 12, 12, xbm_flash);
    u8g2_DrawUTF8(this->u8g2, val_x + 17,val_y, strCurrent.c_str());

    val_y += 16;

    //this->u8g2->drawXBMP(val_x, val_y-12, 12, 12, xbm_power);
    //this->u8g2->drawUTF8(val_x + 17,val_y, strPower.c_str());
    u8g2_DrawXBMP(this->u8g2, val_x, val_y-12, 12, 12, xbm_power);
    u8g2_DrawUTF8(this->u8g2, val_x + 17,val_y, strPower.c_str());
    val_y += 16;

    //this->u8g2->drawXBMP(val_x, val_y-12, 12, 12, xbm_time);
    //this->u8g2->drawUTF8(val_x + 17,val_y, strDuration.c_str());
    u8g2_DrawXBMP(this->u8g2, val_x, val_y-12, 12, 12, xbm_time);
    u8g2_DrawUTF8(this->u8g2, val_x + 17,val_y, strDuration.c_str());
    val_y += 16;
    
    //this->u8g2->drawXBMP(val_x, val_y-12, 12, 12, xbm_energy);
    //this->u8g2->drawUTF8(val_x + 17,val_y, strEnergy.c_str());
    u8g2_DrawXBMP(this->u8g2, val_x, val_y-12, 12, 12, xbm_energy);
    u8g2_DrawUTF8(this->u8g2, val_x + 17,val_y, strEnergy.c_str());
  //} while ( this->u8g2->nextPage() );
  } while ( u8g2_NextPage(this->u8g2) );
  delay(100);
}

void EvseWiFiOled::showSplash(String text, String head) {
  //this->u8g2->firstPage();
  u8g2_FirstPage(this->u8g2);
  do {
    //this->u8g2->drawXBMP(32, 15, 64, 64, xbm_splash);
    //this->u8g2->setFont(u8g2_font_helvR10_tf);
    //this->u8g2->drawUTF8(35 + this->offsetX, 100 + this->offsetY, head.c_str());
    //this->u8g2->drawUTF8(this->offsetX, 120 + this->offsetY, text.c_str());

    u8g2_DrawXBMP(this->u8g2, 32, 15, 64, 64, xbm_splash);
    u8g2_SetFont(this->u8g2, u8g2_font_helvR10_tf);
    u8g2_DrawUTF8(this->u8g2, 35 + this->offsetX, 100 + this->offsetY, head.c_str());
    u8g2_DrawUTF8(this->u8g2, this->offsetX, 120 + this->offsetY, text.c_str());

  //} while(this->u8g2->nextPage());
  } while(u8g2_NextPage(this->u8g2));
}

void EvseWiFiOled::drawCheck(uint8_t textId) {
  /*
    textID 0 => "RFID tag valid"
    textID 1 => "Button input"
    textID 3 => "RFID tag read"
  */
  String text = "";
  uint8_t val_x_text = this->offsetX;
  uint8_t val_y_text = this->offsetY;
  switch (textId)
  {
  case 0:
    text = "RFID tag valid";
    val_x_text += 10;
    val_y_text += 110;
    break;
  case 1:
    text = this->getTranslation(22, this->language); //Button input
    val_x_text += 10;
    val_y_text += 110;
    break;
  case 3:
    text = this->getTranslation(23, this->language); //RFID tag read
    val_x_text += 10;
    val_y_text += 110;
    break;
  default:
    break;
  }
  //this->u8g2->firstPage();
  u8g2_FirstPage(this->u8g2);
  do {
    //this->u8g2->drawXBMP( 27 + this->offsetX, 25 + this->offsetY, xbm_checked_width, xbm_checked_height, xbm_checked);
    //this->u8g2->setFont(u8g2_font_helvR12_tf);
    //this->u8g2->drawUTF8(val_x_text, val_y_text, text.c_str());

    u8g2_DrawXBMP(this->u8g2, 27 + this->offsetX, 25 + this->offsetY, xbm_checked_width, xbm_checked_height, xbm_checked);
    u8g2_SetFont(this->u8g2, u8g2_font_helvR12_tf);
    u8g2_DrawUTF8(this->u8g2, val_x_text, val_y_text, text.c_str());

  //} while(this->u8g2->nextPage());
  } while(u8g2_NextPage(this->u8g2));
}

void EvseWiFiOled::drawUncheck() {
  String text = this->getTranslation(21, this->language); //RFID tag invalid!
  //this->u8g2->firstPage();
  u8g2_FirstPage(this->u8g2);
  do {
    //this->u8g2->drawXBMP( 32 + this->offsetX, 25 + this->offsetY, xbm_unchecked_width, xbm_unchecked_height, xbm_unchecked);
    //this->u8g2->setFont(u8g2_font_helvR12_tf);
    //this->u8g2->drawUTF8(10 + this->offsetX, 110 + this->offsetY, text.c_str());

    u8g2_DrawXBMP(this->u8g2, 32 + this->offsetX, 25 + this->offsetY, xbm_unchecked_width, xbm_unchecked_height, xbm_unchecked);
    u8g2_SetFont(this->u8g2, u8g2_font_helvR12_tf);
    u8g2_DrawUTF8(this->u8g2, 10 + this->offsetX, 110 + this->offsetY, text.c_str());

  //} while(this->u8g2->nextPage());
  } while(u8g2_NextPage(this->u8g2));
}

void EvseWiFiOled::showCheck(bool checked, uint8_t textID) {
  if (checked) {
    drawCheck(textID);
  }
  else {
    drawUncheck();
  }
}

String EvseWiFiOled::getTranslation(uint8_t id, uint8_t lang) {
  switch (id)
  {
  case 1:
    if (lang == 0) {
      return "Sunday";
    }
    else {
      return "Sonntag";
    }
    break;
  case 2:
    if (lang == 0) {
      return "Monday";
    }
    else {
      return "Montag";
    }
    break;
  case 3:
    if (lang == 0) {
      return "Tuesday";
    }
    else {
      return "Dienstag";
    }
    break;
  case 4:
    if (lang == 0) {
      return "Wednesday";
    }
    else {
      return "Mittwoch";
    }
    break;
  case 5:
    if (lang == 0) {
      return "Thursday";
    }
    else {
      return "Donnerstag";
    }
    break;
  case 6:
    if (lang == 0) {
      return "Friday";
    }
    else {
      return "Freitag";
    }
    break;
  case 7:
    if (lang == 0) {
      return "Saturday";
    }
    else {
      return "Samstag";
    }
    break;
  case 10:
    if (lang == 0) {
      return "MB Error";
    }
    else {
      return "MB Fehler";
    }
    break;
  case 11:
    if (lang == 0) {
      return "Ready";
    }
    else {
      return "Bereit";
    }
    break;
  case 12:
    if (lang == 0) {
      return "Detected";
    }
    else {
      return "Angeschlossen";
    }
    break;
  case 13:
    if (lang == 0) {
      return "Charging";
    }
    else {
      return "Lädt"; // ä
    }
    break;
  case 20:
    if (lang == 0) {
      return "RFID tag valid";
    }
    else {
      return "RFID gültig";
    }
    break;
  case 21:
    if (lang == 0) {
      return "RFID tag invalid!";
    }
    else {
      return "RFID ungültig!";
    }
    break;
  case 22:
    if (lang == 0) {
      return "Button input";
    }
    else {
      return "Taster erkannt";
    }
    break;
  case 23:
    if (lang == 0) {
      return "RFID Tag read";
    }
    else {
      return "RFID erkannt";
    }
    break;
  default:
    break;
  }
  return "x";
}

#endif