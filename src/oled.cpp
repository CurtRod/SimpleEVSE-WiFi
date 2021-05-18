#ifndef ESP8266
#include "oled.h"
#include <string>

void EvseWiFiOled::begin(U8G2_SSD1327_WS_128X128_F_4W_HW_SPI* u8, uint8_t rotation) {
  this->u8g2 = u8;
  this->u8g2->begin();
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
  }
}

void EvseWiFiOled::clearBuffer() {
    this->u8g2->clearBuffer();
}

void EvseWiFiOled::sendBuffer() {
    this->u8g2->sendBuffer();
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
  this->u8g2->setPowerSave(1);
  this->displayOn = false;
}

void EvseWiFiOled::turnOn() {
  this->u8g2->setPowerSave(0);
  this->displayOn = true;
}

void EvseWiFiOled::showDemo(uint8_t evseStatus, unsigned long chargingTime, uint16_t current, uint8_t maxcurrent, float power, float energy, time_t time, String* version, bool active, bool timerActive) {
  this->u8g2->firstPage();

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
  uint8_t val_x_date = val_x + 52;
  uint8_t val_y_date1 = val_y - 8;
  uint8_t val_y_date2 = val_y + 5;

  do {
    this->u8g2->setFont(u8g2_font_helvR14_tr);
    this->u8g2->drawStr(val_x,val_y, _time.c_str());
    val_y += 9;

    this->u8g2->setFont(u8g2_font_helvR10_tr);
    this->u8g2->drawStr(val_x_date,val_y_date1, _weekday.c_str());
    this->u8g2->setFont(u8g2_font_helvR08_tr);
    this->u8g2->drawStr(val_x_date,val_y_date2, _date.c_str());
    
    this->u8g2->setFont(u8g2_font_helvR12_tr);
    this->u8g2->drawHLine(val_x,val_y, 122);
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

    if (active) {
      this->u8g2->drawXBMP(val_x, val_y-12, 12, 12, xbm_unlocked);
    }
    else {
      this->u8g2->drawXBMP(val_x, val_y-12, 12, 12, xbm_locked);
    }
    this->u8g2->drawStr(val_x + 17,val_y, strStatus.c_str());
    val_y += 16;

    if (timerActive) {
      this->u8g2->drawXBMP(val_x + 110, val_y-28, 12, 12, xbm_time);
    }
    
    this->u8g2->drawXBMP(val_x, val_y-12, 12, 12, xbm_flash);
    this->u8g2->drawStr(val_x + 17,val_y, strCurrent.c_str());
    val_y += 16;

    this->u8g2->drawXBMP(val_x, val_y-12, 12, 12, xbm_power);
    this->u8g2->drawStr(val_x + 17,val_y, strPower.c_str());
    val_y += 16;

    this->u8g2->drawXBMP(val_x, val_y-12, 12, 12, xbm_time);
    this->u8g2->drawStr(val_x + 17,val_y, strDuration.c_str());
    val_y += 16;
    
    this->u8g2->drawXBMP(val_x, val_y-12, 12, 12, xbm_energy);
    this->u8g2->drawStr(val_x + 17,val_y, strEnergy.c_str());
  } while ( this->u8g2->nextPage() );
  delay(100);
}

void EvseWiFiOled::showSplash(String text) {
  this->u8g2->firstPage();
  do {
    this->u8g2->drawXBMP(32, 15, 64, 64, xbm_splash);
    this->u8g2->setFont(u8g2_font_helvR10_tr);
    this->u8g2->drawStr(35 + this->offsetX, 100 + this->offsetY, "Loading...");
    this->u8g2->drawStr(this->offsetX, 120 + this->offsetY, text.c_str());
  } while(this->u8g2->nextPage());
}

void EvseWiFiOled::drawCheck(uint8_t textId) {
  /*
    textID 0 => "RFID tag valid"
    textID 1 => "Button input"
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
    text = "Button input";
    val_x_text += 20;
    val_y_text += 110;
  default:
    break;
  }
  this->u8g2->firstPage();
  do {
    this->u8g2->drawXBMP( 27 + this->offsetX, 25 + this->offsetY, xbm_checked_width, xbm_checked_height, xbm_checked);
    this->u8g2->setFont(u8g2_font_helvR12_tr);
    this->u8g2->drawStr(val_x_text, val_y_text, text.c_str());
  } while(this->u8g2->nextPage());
}

void EvseWiFiOled::drawUncheck() {
  String text = "RFID tag invalid!";
  this->u8g2->firstPage();
  do {
    this->u8g2->drawXBMP( 32 + this->offsetX, 25 + this->offsetY, xbm_unchecked_width, xbm_unchecked_height, xbm_unchecked);
    this->u8g2->setFont(u8g2_font_helvR12_tr);
    this->u8g2->drawStr(10 + this->offsetX, 110 + this->offsetY, text.c_str());
  } while(this->u8g2->nextPage());
}

void EvseWiFiOled::showCheck(bool checked, uint8_t textID) {
  if (checked) {
    drawCheck(textID);
  }
  else {
    drawUncheck();
  }
}
#endif