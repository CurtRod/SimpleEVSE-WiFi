#include <Arduino.h>
#include <ArduinoJson.h>

#ifdef ESP8266
#include <FS.h>
#else
#include <SPIFFS.h>
#endif

#define ACTUAL_CONFIG_VERSION 1

struct s_wifiConfig {
    const char* bssid;
    const char* ssid;
    bool wmode;
    const char* pswd;
    bool staticip;
    const char* ip;
    const char* subnet;
    const char* gateway;
    const char* dns;
};

struct s_meterConfig {
    bool usemeter;
    const char* metertype;
    float price;
    uint8_t intpin;
    uint16_t kwhimp;
    uint16_t implen;
    uint8_t meterphase;
    uint8_t factor;
};

struct s_rfidConfig {
    bool userfid;
    bool usePN532;
    uint8_t sspin;
    int8_t rfidgain;
};

struct s_ntpConfig {
    int8_t timezone;
    const char* ntpip;
    bool dst;
};

struct s_buttonConfig {
    bool usebutton;
    uint8_t buttonpin;
};

struct s_systemConfig {
    const char* hostnm;
    const char* adminpwd;
    bool wsauth;
    bool debug;
    bool disfbreboot;
    uint8_t maxinstall;
    uint8_t configversion;
    uint8_t evsecount;
    bool logging;
    bool api;
};

struct s_evseConfig {
    uint8_t mbid;
    bool alwaysactive;
    bool remote;
    uint8_t ledconfig;
    uint8_t drotation;
    bool resetcurrentaftercharge;
    uint8_t maxcurrent;
    float avgconsumption;
    bool rseActive;
    uint8_t rseValue;
};

class EvseWiFiConfig {
public:
    bool ICACHE_FLASH_ATTR loadConfig(String = "");
    bool ICACHE_FLASH_ATTR loadConfiguration();
    bool ICACHE_FLASH_ATTR printConfigFile();
    bool ICACHE_FLASH_ATTR printConfig();
    bool ICACHE_FLASH_ATTR renewConfigFile();
    bool ICACHE_FLASH_ATTR updateConfig(String);
    String ICACHE_FLASH_ATTR getConfigJson();

// wifiConfig
    const char * ICACHE_FLASH_ATTR getWifiBssid();
    const char * ICACHE_FLASH_ATTR getWifiSsid();
    bool ICACHE_FLASH_ATTR getWifiWmode();
    const char * ICACHE_FLASH_ATTR getWifiPass();
    bool ICACHE_FLASH_ATTR getWifiStaticIp();
    const char * ICACHE_FLASH_ATTR getWifiIp();
    const char * ICACHE_FLASH_ATTR getWiFiSubnet();
    const char * ICACHE_FLASH_ATTR getWiFiGateway();
    const char * ICACHE_FLASH_ATTR getWiFiDns();

// meterConfig
    bool ICACHE_FLASH_ATTR getMeterActive(uint8_t meterId);
    const char * ICACHE_FLASH_ATTR getMeterType(uint8_t meterId);
    float ICACHE_FLASH_ATTR getMeterEnergyPrice(uint8_t meterId);
    uint8_t ICACHE_FLASH_ATTR getMeterPin(uint8_t meterId);
    uint16_t ICACHE_FLASH_ATTR getMeterImpKwh(uint8_t meterId);
    uint16_t ICACHE_FLASH_ATTR getMeterImpLen(uint8_t meterId);
    uint8_t ICACHE_FLASH_ATTR getMeterPhaseCount(uint8_t meterId);
    uint8_t ICACHE_FLASH_ATTR getMeterFactor(uint8_t meterId);
    bool useSMeter;
    bool useMMeter;
    bool mMeterTypeSDM120;
    bool mMeterTypeSDM630;

// rfidConfig
    bool ICACHE_FLASH_ATTR getRfidActive();
    uint8_t ICACHE_FLASH_ATTR getRfidPin();
    uint8_t ICACHE_FLASH_ATTR getRfidUsePN532();
    int8_t ICACHE_FLASH_ATTR getRfidGain();

// ntpConfig
    int8_t ICACHE_FLASH_ATTR getNtpTimezone();
    const char * ICACHE_FLASH_ATTR getNtpIp();
    bool ICACHE_FLASH_ATTR getNtpDst();

// buttonConfig
    bool ICACHE_FLASH_ATTR getButtonActive(uint8_t buttonId);
    uint8_t ICACHE_FLASH_ATTR getButtonPin(uint8_t buttonId);

// systemConfig
    const char * ICACHE_FLASH_ATTR getSystemHostname();
    const char * ICACHE_FLASH_ATTR getSystemPass();
    bool ICACHE_FLASH_ATTR getSystemWsauth();
    bool ICACHE_FLASH_ATTR getSystemDebug();
    bool ICACHE_FLASH_ATTR getSystemDisableFallbackReboot();
    uint8_t ICACHE_FLASH_ATTR getSystemMaxInstall();
    uint8_t ICACHE_FLASH_ATTR getSystemConfigVersion();
    uint8_t ICACHE_FLASH_ATTR getSystemEvseCount();
    bool ICACHE_FLASH_ATTR getSystemLogging();
    bool ICACHE_FLASH_ATTR getSystemApi();

// evseConfig
    uint8_t ICACHE_FLASH_ATTR getEvseMbid(uint8_t evseId);
    bool ICACHE_FLASH_ATTR getEvseAlwaysActive(uint8_t evseId);
    bool ICACHE_FLASH_ATTR getEvseRemote(uint8_t evseId);
    uint8_t ICACHE_FLASH_ATTR getEvseLedConfig(uint8_t evseId);
    uint8_t ICACHE_FLASH_ATTR getEvseDisplayRotation(uint8_t evseId);
    uint8_t ICACHE_FLASH_ATTR getEvseLedPin(uint8_t evseId);
    bool ICACHE_FLASH_ATTR getEvseResetCurrentAfterCharge(uint8_t evseId);
    uint8_t ICACHE_FLASH_ATTR getEvseMaxCurrent(uint8_t evseId);
    float ICACHE_FLASH_ATTR getEvseAvgConsumption(uint8_t evseId);
    uint8_t ICACHE_FLASH_ATTR getEvseCpIntPin(uint8_t evseId);
    bool ICACHE_FLASH_ATTR getEvseRseActive(uint8_t evseId);
    uint8_t ICACHE_FLASH_ATTR getEvseRsePin(uint8_t evseId);
    uint8_t ICACHE_FLASH_ATTR getEvseRseValue(uint8_t evseId);

private:
    s_wifiConfig wifiConfig;
    s_meterConfig meterConfig[1];
    s_rfidConfig rfidConfig;
    s_ntpConfig ntpConfig;
    s_buttonConfig buttonConfig[1];
    s_systemConfig systemConfig;
    s_evseConfig evseConfig[1];

    bool configLoaded;
    bool pre_0_4_Config;
    bool ICACHE_FLASH_ATTR saveConfigFile(String jsonConfig);

protected:

};