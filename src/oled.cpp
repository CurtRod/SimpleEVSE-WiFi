#ifdef OLED
#include "oled.h"
#include <string>

//U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
void EvseWiFiOled::begin(U8G2_SSD1306_128X64_NONAME_F_HW_I2C* u8, uint8_t rotation) {
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

void EvseWiFiOled::setLanguage(EvseWiFiConfig* config) {
  if (strcmp(config->getSystemLanguage(), "en") == 0) {
    this->language = 0; //EN
  }
  else {
    this->language = 1; //DE
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

void EvseWiFiOled::showDemo(uint8_t evseStatus, unsigned long chargingTime, uint16_t current, uint8_t maxcurrent, uint8_t phases,float power, float energy, time_t time, String* version, bool active, bool timerActive) {
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



  // Day of the week
  String _weekday = this->getTranslation(weekday(time), this->language);



























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
  strCurrent += + " / " + (String)maxcurrent + " A " + "(" + phases + "P)";
  
  String strPower = (String)power + " kW";
  String strEnergy = (String)energy + " kWh";
  String strSwVersion = "v" + *version;

  uint8_t val_x = 0 + this->offsetX;
  uint8_t val_y = 20 + this->offsetY;
  uint8_t val_x_date = val_x + 52;
  uint8_t val_y_date1 = val_y - 8;
  uint8_t val_y_date2 = val_y + 5;

  do {
    this->u8g2->setFont(u8g2_font_helvR14_tf);
    this->u8g2->drawUTF8(val_x,val_y, _time.c_str());
    val_y += 9;

    this->u8g2->setFont(u8g2_font_helvR10_tf);
    this->u8g2->drawUTF8(val_x_date,val_y_date1, _weekday.c_str());
    this->u8g2->setFont(u8g2_font_helvR08_tf);
    this->u8g2->drawUTF8(val_x_date,val_y_date2, _date.c_str());
    
    this->u8g2->setFont(u8g2_font_helvR12_tf);
    this->u8g2->drawHLine(val_x,val_y, 122);
    val_y += 17;

    //Vehicle Status
    String strStatus = "";
    switch (evseStatus)
    {
    case 0:
      strStatus += this->getTranslation(10, this->language); //MB Error
      break;
    case 1:
      strStatus += this->getTranslation(11, this->language); //Ready
      break;
    case 2:
      strStatus += this->getTranslation(12, this->language); //Detected
      break;
    case 3:
      strStatus += this->getTranslation(13, this->language); //Charging
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
    this->u8g2->drawUTF8(val_x + 17,val_y, strStatus.c_str());
    val_y += 16;

    if (timerActive) {
      this->u8g2->drawXBMP(val_x + 110, val_y-28, 12, 12, xbm_time);
    }
    
    this->u8g2->drawXBMP(val_x, val_y-12, 12, 12, xbm_flash);
    this->u8g2->drawUTF8(val_x + 17,val_y, strCurrent.c_str());
    val_y += 16;

    this->u8g2->drawXBMP(val_x, val_y-12, 12, 12, xbm_power);
    this->u8g2->drawUTF8(val_x + 17,val_y, strPower.c_str());
    val_y += 16;

    this->u8g2->drawXBMP(val_x, val_y-12, 12, 12, xbm_time);
    this->u8g2->drawUTF8(val_x + 17,val_y, strDuration.c_str());
    val_y += 16;
    
    this->u8g2->drawXBMP(val_x, val_y-12, 12, 12, xbm_energy);
    this->u8g2->drawUTF8(val_x + 17,val_y, strEnergy.c_str());
  } while ( this->u8g2->nextPage() );
  delay(100);
}

void EvseWiFiOled::showSplash(String text, String head) {
  this->u8g2->firstPage();
  do {
    this->u8g2->drawXBMP(32, 15, 64, 64, xbm_splash);
    this->u8g2->setFont(u8g2_font_helvR10_tf);
    this->u8g2->drawUTF8(35 + this->offsetX, 100 + this->offsetY, head.c_str());
    this->u8g2->drawUTF8(this->offsetX, 120 + this->offsetY, text.c_str());
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
    text = this->getTranslation(20, this->language); //RFID tag valid
    val_x_text += 10;
    val_y_text += 110;
    break;
  case 1:
    text = this->getTranslation(22, this->language); //Button input
    val_x_text += 15;
    val_y_text += 110;
  case 3:
    text = this->getTranslation(23, this->language); //RFID tag read
    val_x_text += 10;
    val_y_text += 110;
  default:
    break;
  }
  this->u8g2->firstPage();
  do {
    this->u8g2->drawXBMP( 27 + this->offsetX, 25 + this->offsetY, xbm_checked_width, xbm_checked_height, xbm_checked);
    this->u8g2->setFont(u8g2_font_helvR12_tf);
    this->u8g2->drawUTF8(val_x_text, val_y_text, text.c_str());
  } while(this->u8g2->nextPage());
}

void EvseWiFiOled::drawUncheck() {
  String text = this->getTranslation(21, this->language); //RFID tag invalid!
  this->u8g2->firstPage();
  do {
    this->u8g2->drawXBMP( 32 + this->offsetX, 25 + this->offsetY, xbm_unchecked_width, xbm_unchecked_height, xbm_unchecked);
    this->u8g2->setFont(u8g2_font_helvR12_tf);
    this->u8g2->drawUTF8(10 + this->offsetX, 110 + this->offsetY, text.c_str());
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
  return "";
}

#endif