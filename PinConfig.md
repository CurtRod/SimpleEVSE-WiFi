# pins EVSE-WIFI


|Module|Module PIN|EVSE-Wifi 1.0<br/>Wemos D1 Mini|EVSE-Wifi 2.0<br/>Wemos D1 Mini32|GND|3.3v|Remark
|:----|:-:|:-----:|:-----:|:-:|:-:|:------------------------|
|LED EVSE-WIFI|+|D0|26|x||optional
|EVSE (*1)|TX|D1||||using SoftwareSerial
|EVSE (*1)|RX|D2||||using SoftwareSerial
|RS485|TX|TX (*1)|22|x|x|Modbus interface
|RS485|RX|RX (*1)|21|||Modbus Interface
|EVSE (*1)|A|-|-|||using RS485 bus A, bus-id=1
|EVSE (*1)|B|-|-|||using RS485 bus B, bus-id=1
|SDM120<br/>SDM630|A|||||optional, using RS485 bus A, bus-id=2
|SDM120<br/>SDM630|B|||||optional, using RS485 bus B, bus-id=2
|S0 Meter|S0+|D3|17|||optional
|S0 Meter|S0-|||x||optional
|Button (Charge)||D4|16|x||optional
|Button (Reset)||Rst|Rst|x||optional
|RFID|Base|||x|x|
|RFID|SCK|D5|18|||(*2)
|RFID|MISO|D6|19|||(*2)
|RFID|MOSI|D7|23|||(*2)
|RFID|SS (SDA)|D8|5|||(*2)
|CPInt|S||4|x|x|(*5)
|RSE|||2|||(*3)
|Oled SSD1327|base|||x|x|optional (*4)
|Oled SSD1327|CS||12||||
|Oled SSD1327|DC||13||||
|Oled SSD1327|DIN (MOSI)||18||||
|Oled SSD1327|CLK (SCK)||23||||
|Oled SSD1327|reset||33||||


(*1) EVSE connection: 
* in 8266 (EVSE-WIFI 1.0) the EVSE is connected using SoftwareSerial on D1/D2, and RS485 is always enabled on TXD/RXD. The Modbus Meter (SDM120/630) is always connected using the RS485 adapter on TXD/RXD.
* EVSE-WIFI 2.0 and SmartWB the RS485 is connected over 22/21 using HardwareSerial. EVSE and ModbusMeters are connected over RS485 only / always.  
EVSE has different connectors for Modbus and Serial input, so this is not directly pin compatible.  

(*2) RFID uses always first SPI (VSPI) pins from board. Cannot be changed.  
(*3) "Rundsteuerempfänger" (receiver for external control), SmartWB only. Extra hardware required.  
(*4) the default display is a WaveShare SSD1327 connected using 4wire SPI interface and uses the VSPI hardware interface (shared with RFID SPI bus). The graphics library (u8g2) does not fit into ESP8266 memory in this setup, therefore supported on ESP32 only. Using different SPI (HSPI) requires using 13/14 for DIN and CLK, causing other pin conflicts. See here: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/spi_master.html  
(*5) CPInt (CP Interrupt) simulates disconnecting / connecting charger from car and is required for some car models which will fall asleep after some time of inactivity on charge port. Using CPInt requires connecting a relais, which will be connected in between EVSE CP pin and CP wire to car connector