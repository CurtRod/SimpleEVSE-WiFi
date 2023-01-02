struct s_addEvseData {
    uint16_t evseAmpsAfterboot;  //Register 2000
    uint16_t evseModbusEnabled;  //Register 2001
    uint16_t evseAmpsMin;        //Register 2002
    uint16_t evseAnIn;           //Register 2003
    uint16_t evseAmpsPowerOn;    //Register 2004
    uint16_t evseReg2005;        //Register 2005
    uint16_t evseShareMode;      //Register 2006
    uint16_t evsePpDetection;    //Register 2007
    uint16_t evseBootFirmware;   //Register 2009
};

void ICACHE_FLASH_ATTR doChangeLedTimes();
void ICACHE_FLASH_ATTR changeLedTimes(uint16_t, uint16_t);
String ICACHE_FLASH_ATTR printIP(IPAddress);
void ICACHE_FLASH_ATTR parseBytes(const char*, char, byte*, int, int);
void ICACHE_RAM_ATTR handleMeterInt();
void ICACHE_FLASH_ATTR updateS0MeterData();
void ICACHE_FLASH_ATTR updateMMeterData();
void ICACHE_FLASH_ATTR updateSDMMeterCurrent();
unsigned long ICACHE_FLASH_ATTR getChargingTime();
void ICACHE_FLASH_ATTR rfidloop();
bool ICACHE_FLASH_ATTR getAdditionalEVSEData();
void ICACHE_FLASH_ATTR sendStatus();
void ICACHE_FLASH_ATTR printScanResult(int);
void ICACHE_FLASH_ATTR logLatest(String, String);
void ICACHE_FLASH_ATTR updateLog(bool);
float ICACHE_FLASH_ATTR getS0MeterReading();
bool ICACHE_FLASH_ATTR initLogFile();
float ICACHE_FLASH_ATTR readMeter(uint16_t);
bool ICACHE_FLASH_ATTR queryEVSE(bool);
bool ICACHE_FLASH_ATTR activateEVSE();
bool ICACHE_FLASH_ATTR deactivateEVSE(bool);
bool ICACHE_FLASH_ATTR setEVSEcurrent();
bool ICACHE_FLASH_ATTR setEVSERegister(uint16_t, uint16_t);
bool ICACHE_FLASH_ATTR setSDMID();
void ICACHE_FLASH_ATTR pushSessionTimeOut();
void ICACHE_FLASH_ATTR sendEVSEdata();
void ICACHE_FLASH_ATTR sendTime();
void ICACHE_FLASH_ATTR sendUserList(int , AsyncWebSocketClient*);
void ICACHE_FLASH_ATTR onWsEvent(AsyncWebSocket*, AsyncWebSocketClient*, AwsEventType, void*, uint8_t*, size_t);
void ICACHE_FLASH_ATTR processWsEvent(JsonDocument&, AsyncWebSocketClient*);
void ICACHE_FLASH_ATTR restoreDefaultConfig();
void ICACHE_FLASH_ATTR setWebEvents();
void ICACHE_FLASH_ATTR fallbacktoAPMode();
void ICACHE_FLASH_ATTR startWebserver();
bool ICACHE_FLASH_ATTR resetUserData();
void ICACHE_FLASH_ATTR timerActivateMatch(uint16_t);
void ICACHE_FLASH_ATTR timerDeactivateMatch();
void ICACHE_FLASH_ATTR sendSyslogToWs();
