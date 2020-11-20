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

void EvseWiFiOled::showDemo(uint8_t evseStatus, unsigned long chargingTime, uint8_t current, uint8_t maxcurrent, float power, float energy, time_t time, String* version) {
  this->u8g2->firstPage();

  String head;
  if (hour(time) < 10) {
    head += ("0" + (String)hour(time));
  }
  else {
    head += (String)hour(time);
  }
  head += ":";
  if (minute(time) < 10) {
    head += ("0" + (String)minute(time));
  }
  else {
    head += (String)minute(time);
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

  chargingTime = chargingTime / 1000;
  String strCurrent = (String)current + " / " + maxcurrent + " A";
  String strPower = (String)power + " kW";
  String strEnergy = (String)energy + " kWh";
  String strSwVersion = "v" + *version;

uint8_t val_x = 0;
uint8_t val_y = 20;

  do {
    this->u8g2->setFont(u8g2_font_helvR14_tr);
    this->u8g2->drawStr(val_x,val_y, head.c_str());
    val_y += 9;

    this->u8g2->setFont(u8g2_font_helvR10_tr);
    this->u8g2->drawStr(52,12, _weekday.c_str());
    this->u8g2->setFont(u8g2_font_helvR08_tr);
    this->u8g2->drawStr(52,25, _date.c_str());
    
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

    this->u8g2->drawStr(val_x,val_y, strStatus.c_str());
    val_y += 16;
    this->u8g2->drawStr(val_x,val_y, strCurrent.c_str());
    val_y += 16;
    this->u8g2->drawStr(val_x,val_y, strPower.c_str());
    val_y += 16;
    this->u8g2->drawStr(val_x,val_y, strEnergy.c_str());
    this->u8g2->setFont(u8g2_font_helvR08_tr);
    this->u8g2->drawStr(95,128, strSwVersion.c_str());
  } while ( this->u8g2->nextPage() );
  delay(100);
}

void EvseWiFiOled::drawLock() {
  this->u8g2->firstPage();
  do {
    this->u8g2->drawXBMP( 0, 0, 128, 128, xbm_locked_bits);
  } while(this->u8g2->nextPage());
}

void EvseWiFiOled::drawUnlock() {
  this->u8g2->firstPage();
  do {
    this->u8g2->drawXBMP( 0, 0, 128, 128, xbm_unlocked_bits);
  } while(this->u8g2->nextPage());
}

void EvseWiFiOled::showLock(bool locked) {
  if (locked) {
    drawLock();
  }
  else {
    drawUnlock();
  }
}
#endif