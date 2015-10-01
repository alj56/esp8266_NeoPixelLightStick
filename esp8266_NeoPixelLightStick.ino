/*

commands tcp:
LS<0><1>L<bytes> load bitmap file
LS<0><1>S<delay LSB><delay MSB>
                 show loaded bitmap file with <delay> ms between lines
LS<0><1>U        switch mode to udp
LS<0><1>D<byte>  set debug mode

commands udp:
LS<0><1>T       switch mode to tcp
LS<0><1>L<length LSB><length MSB><bytes>
                show data for a line

file data, starting at each 1024 byte block (BUFFER_SIZE):
Address RawFlashDriver._startAddress + RawFlashDriver._flashSecSize:
  LS<0><1>B<width LSB><width MSB><height LSB><height MSB>
Startaddress + linenumber * BUFFER_SIZE
  <bytes> for a line
*/

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#define NEOPIXELBUS 0
#define ADAFRUIT 1
#define NEOPIXELLIBRARY ADAFRUIT
#if NEOPIXELLIBRARY == NEOPIXELBUS
  #include <NeoPixelBus.h>
#elif NEOPIXELLIBRARY == NEOPIXELBUS
  #include <Adafruit_NeoPixel.h>
#endif

#include "RawFlashDriver.h"
#include "NeoPixelLightStick.h"
#include "WiFiParameters.h"

byte debugLevel = 1;


#define BUFFER_SIZE 1024
uint8_t buffer[BUFFER_SIZE];


#define UDP_PORT 7777
#define TCP_PORT 7778
WiFiUDP udp;
WiFiServer tcp(TCP_PORT);


#define CMD_INVALID 0
#define CMD_LOAD_BITMAP 1
#define CMD_SHOW_BITMAP 2
#define CMD_TO_UDP 3
#define CMD_TO_TCP 4
#define CMD_SET_DEBUG_LEVEL 5

#define MODE_TCP 0
#define MODE_UDP 1
int mode = MODE_TCP;

#define STAT_SIZE 512
#define COLLECT_STATISTICS true
long statistics[STAT_SIZE][4];
int statCounter = 0;
#define STAT_STARTTIME 0
#define STAT_STARTSHOW 1
#define STAT_ENDED 2
#define STAT_DELAY 3


int height;
int width;
long delayMs;


void setup() {
  Serial.begin(115200);
  Serial.println(F("initializing"));

  WiFi.mode(WIFI_AP_STA);
  Serial.println(F("open AP LS"));
  WiFi.softAP("LS");
  Serial.print(F("IP-Address of AP: ")); Serial.println(WiFi.softAPIP());

  Serial.print(F("Connecting to "));
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  NeoPixelLightStick.black(NeoPixelLightStick._maxPixelCount);
  int cnt = 0;
  while (cnt < 25 && WiFi.status() != WL_CONNECTED) {
    cnt++;
    showProgress(0xff, 0xff, 0xff, cnt, 25);
    delay(500);
    Serial.print(F("."));
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print(F("WiFi connected, IP address: "));
    Serial.println(WiFi.localIP());    
    NeoPixelLightStick.color(NeoPixelLightStick._maxPixelCount, 0x00, 0xff, 0x00);
  } else {
    Serial.println(F("Connection to AP failed"));
    NeoPixelLightStick.color(NeoPixelLightStick._maxPixelCount, 0xff, 0x00, 0x00);
  }
  delay(500);
  
  Serial.println(F("Starting UDP"));
  udp.begin(UDP_PORT);
  Serial.print(F("Local port: "));
  Serial.println(udp.localPort());

  Serial.println(F("Starting TCP"));
  tcp.begin();

  NeoPixelLightStick.black(NeoPixelLightStick._maxPixelCount);
  
  Serial.println(F("initialized"));
}


void loop() {
  switch (mode) {
    case MODE_UDP:
      handleUdp();
      break;
    case MODE_TCP:
      handleTcp();
      break;
    default:
      Serial.print(F("unknown mode, switching to mode tcp"));
      mode = MODE_TCP;
      break;
  }
}


void showProgress(byte r, byte g, byte b, int value, int maxValue) {
  NeoPixelLightStick.setColor(value % maxValue, r, g, b);
  NeoPixelLightStick.setColor((value + maxValue - 1) % maxValue, 0x00, 0x00, 0x00);
  NeoPixelLightStick.show();
}


void handleUdp() {
  long startTime;
  if (COLLECT_STATISTICS) startTime = millis();
  int cb = udp.parsePacket();
  if (cb) {
    if (debugLevel >= 1) {
      Serial.print(F("packet received, length="));
      Serial.println(cb);  
    }
    udp.read(buffer, BUFFER_SIZE);
    if (cb >= 5 && buffer[0] == 'L' && buffer[1] == 'S' && buffer[2] == 0 && buffer[3] == 1) {
      if (buffer[4] == 'T') {
        if (debugLevel >= 1) Serial.println(F("udp command toTCP"));
        mode = MODE_TCP;
        if (COLLECT_STATISTICS) {
          showStatistics();
        }
      } else if (cb >= 7 && buffer[4] == 'L') {      
        if (debugLevel >= 1) Serial.println(F("udp command showLine"));
        int len = buffer[5] + buffer[6] * 256;      
        if (cb != len + 7) {
          Serial.print(F("invalid packet size, expected: "));
          Serial.print(len + 7);
          Serial.print(F(", received: ")); 
          Serial.println(cb);
        } else {
          if (debugLevel >= 6) {
            Serial.println(F("data: "));
            showBufferData(7, len);
          }
          if (COLLECT_STATISTICS) {
            statistics[statCounter][STAT_STARTTIME] = startTime;
            statistics[statCounter][STAT_STARTSHOW] = millis();
          }
          NeoPixelLightStick.showBuffer(buffer, 7, len);
          if (COLLECT_STATISTICS) {
            statistics[statCounter][STAT_ENDED] = millis();
            statistics[statCounter][STAT_DELAY] = 0;
            if (statCounter < STAT_SIZE - 1) statCounter++;
          }
        }
      } else {
        Serial.print(F("invalid packet data, length: "));
        Serial.print(cb);
        Serial.print(F(", staring with: "));
        showBufferData(7, cb<7?cb:7);
      }
    } else {
      Serial.print(F("invalid command: "));
      if (cb >= 1) Serial.print((char)buffer[0]);
      if (cb >= 2) Serial.print((char)buffer[1]);
      Serial.print(F(" "));
      if (cb >= 3) Serial.print(buffer[2]);
      Serial.print(F(" "));
      if (cb >= 4) Serial.print(buffer[3]);
      Serial.print(F(" "));
      if (cb >= 5) Serial.print((char)buffer[4]);
      Serial.println();
      Serial.print(F(", staring with: "));
      showBufferData(7, cb<20?cb:20);
    }
  }    
}


bool readByteTcp(WiFiClient client, byte &b) {
  delay(1);
  while (client.connected() && !client.available()) delay(1);
  if (client.available()) b = client.read();
  return client.connected();
}


int readCommandTcp(WiFiClient client) {
  if (debugLevel == 1) {
    Serial.print(F("tcp data arrived, bytes available: "));
    delay(1);
    Serial.println(client.available());
  }
  int result = CMD_INVALID;
  byte c1, c2, c5 = '.';
  byte c3, c4 = 255;
  if (!readByteTcp(client, c1)) c1 = '?';
  if (!readByteTcp(client, c2)) c2 = '?';
  if (!readByteTcp(client, c3)) c3 = 254;
  if (!readByteTcp(client, c4)) c4 = 254;
  if (!readByteTcp(client, c5)) c5 = '?';
  if (c1 == 'L' && c2 == 'S' && c3 == 0 && c4 == 1) {
    if (c5 == 'L') {
      if (debugLevel == 1) Serial.println(F("command loadFile"));
      result = CMD_LOAD_BITMAP;
    } else if (c5 == 'S') {
      if (debugLevel == 1) Serial.println(F("command showFile"));
      result = CMD_SHOW_BITMAP;
    } else if (c5 == 'U') {
      if (debugLevel == 1) Serial.println(F("command toUDP"));
      result = CMD_TO_UDP;
    } else if (c5 == 'D') {
      if (debugLevel == 1) Serial.println(F("command setDebugLevel"));
      result == CMD_SET_DEBUG_LEVEL;
    }
  }
  if (result == CMD_INVALID) {
    Serial.print(F("unknown command: ")); 
    Serial.print((char)c1); 
    Serial.print((char)c2); 
    Serial.print(F(" "));
    Serial.print(c3); 
    Serial.print(F(" "));
    Serial.print(c4); 
    Serial.print(F(" "));
    Serial.println((char)c5);
    while (client.available()) client.read();
    client.stop();
  }
  return result;
}


void handleTcp() {
  WiFiClient client = tcp.available();
  if (client && client.connected()) {
    int command = readCommandTcp(client);
    switch(command) {
      case CMD_LOAD_BITMAP: {
        NeoPixelLightStick.black(NeoPixelLightStick._maxPixelCount);
        int progressCounter = 0;
        int maxProgessPixel = 12;
        if (debugLevel == 1) {
          RawFlashDriver.showFlashInfo();
          Serial.println(F("erasing flash"));
        }
        bool result = true;
        for (uint32_t i = RawFlashDriver._startAddress + RawFlashDriver._flashSecSize; 
             i < RawFlashDriver._startAddress + RawFlashDriver._flashSize && result; 
             i = i + RawFlashDriver._flashSecSize) {
          if (debugLevel >= 1) {
            Serial.print(F("erasing address: 0x")); Serial.println(i, HEX);
          }
          showProgress(0x00, 0x00, 0xff, progressCounter, maxProgessPixel);
          progressCounter++;
          result = RawFlashDriver.eraseSector(i);
          delay(1);
        }
        if (!result) {
          Serial.println(F("erasing flash failed"));
        }
        NeoPixelLightStick.black(NeoPixelLightStick._maxPixelCount);
        Serial.println(F("writing header"));
        height = -1; width = -1;
        for (int i = 0; i < BUFFER_SIZE; i++) buffer[i] = 0xff;
        for (int i = 0; i < 9; i++) if (!readByteTcp(client, buffer[i])) buffer[i] = 0xfe;
        if (buffer[0] == 'L' && buffer[1] == 'S' && buffer[2] == 0 && buffer[3] == 1 && buffer[4] == 'B' && 
            buffer[6] < 0xfe && buffer[8] < 0xfe) {
          height = buffer[5] + buffer[6] * 256;
          width = buffer[7] + buffer[8] * 256;
          if (debugLevel >= 1) {
            Serial.print(F("width: ")); Serial.print(width); Serial.print(F(", height: ")); Serial.println(height);
          }
          RawFlashDriver.write(RawFlashDriver._startAddress + RawFlashDriver._flashSecSize, buffer, 9);
          int index = 0;
          int addr = RawFlashDriver._startAddress + RawFlashDriver._flashSecSize + BUFFER_SIZE;
          byte b;
          progressCounter = 0;
          while (readByteTcp(client, buffer[index]) && addr < RawFlashDriver._startAddress + RawFlashDriver._flashSize && progressCounter < width) {
            index++;
            if (index == height*3) {
              if (debugLevel >= 1) { 
                Serial.print(F("line: ")); Serial.print(progressCounter);
                Serial.print(F(", writing buffer to: 0x")); Serial.print(addr, HEX);
                Serial.print(F(", size: ")); Serial.println(height*3);
              }
              RawFlashDriver.write(addr, buffer, height*3);
              index = 0;
              addr = addr + BUFFER_SIZE;
              showProgress(0x00, 0xff, 0x00, progressCounter, maxProgessPixel);
              progressCounter++;
              delay(1);
           }
          }
          Serial.println(F("writing bitmap finished"));
          NeoPixelLightStick.color(NeoPixelLightStick._maxPixelCount, 0x00, 0xff, 0x00);
          client.write("OK");
        } else {
          Serial.println(F("invalid header"));
          showBufferData(0, 9);
          NeoPixelLightStick.color(NeoPixelLightStick._maxPixelCount, 0xff, 0x00, 0x00);
          client.write("NOK, invalid header");
        }
        delay(500);
        NeoPixelLightStick.black(NeoPixelLightStick._maxPixelCount);
        break;
      }
      case CMD_SHOW_BITMAP: {
        width = -1;
        height = -1;
        delayMs = -1;
        byte b1, b2;
        if (readByteTcp(client, b1) && readByteTcp(client, b2)) {
          delayMs = b1 + b2 * 256;
          if (debugLevel >= 1) {
            Serial.print(F("delay: "));
            Serial.println(delayMs);
          }
          if (RawFlashDriver.read(RawFlashDriver._startAddress + RawFlashDriver._flashSecSize, buffer, BUFFER_SIZE) &&
              buffer[0] == 'L' && buffer[1] == 'S' && buffer[2] == 0x00 && buffer[3] == 0x01 && buffer[4] == 'B') {
            height = buffer[5] + buffer[6] * 256;
            width = buffer[7] + buffer[8] * 256;
            Serial.print(F("found bitmap, width: ")); Serial.print(width); Serial.print(F(", height: ")); Serial.println(height);
            Serial.println(F("start showing bitmap"));
            uint32_t address = RawFlashDriver._startAddress + RawFlashDriver._flashSecSize + BUFFER_SIZE;
            int counter = 0;
            statCounter = 0;
            for (int i = 0; i < width; i++) {
              long startTime = millis();
              if (COLLECT_STATISTICS) statistics[statCounter][STAT_STARTTIME] = startTime;
              if (debugLevel >= 5) {
                Serial.print(F("handleShowFile, counter: "));
                Serial.println(counter);
              }
              if (RawFlashDriver.read(address, buffer, height*3)) {
                if (debugLevel >= 4) showBufferData(0, height*3);
                address = address + BUFFER_SIZE;
                if (COLLECT_STATISTICS) statistics[statCounter][STAT_STARTSHOW] = millis();
                NeoPixelLightStick.showBuffer(buffer, 0, height*3);
                long endTime = millis();
                if (COLLECT_STATISTICS) statistics[statCounter][STAT_ENDED] = endTime;
                long aDelay = startTime + delayMs - endTime;
                if (COLLECT_STATISTICS) statistics[statCounter][STAT_DELAY] = aDelay;
                if (COLLECT_STATISTICS) if (statCounter < STAT_SIZE) statCounter++;
                delay(aDelay>0?aDelay:0);
              } else {
                Serial.println(F("RawFlashDriver.read failed"));
               }
            }
            NeoPixelLightStick.black(NeoPixelLightStick._maxPixelCount);
            if (debugLevel >= 1) Serial.println(F("handleShowFile, bitmap finished"));
            if (COLLECT_STATISTICS) showStatistics();  
            client.write("OK");      
          } else {
            Serial.println(F("invalid data in flash"));
            showBufferData(0, 9);
            client.write("NOK: invalid data in flash");
          }
        } else  {
          Serial.println(F("invalid delay"));
          client.write("NOK: invalid delay");
        }
        break;
      }
      case CMD_TO_UDP:
        mode = MODE_UDP;
        statCounter = 0;
        while (client.available()) client.read();
        client.write("OK");
        break;
      case CMD_SET_DEBUG_LEVEL:
        if (client.available()) debugLevel = client.read();
        while (client.available()) client.read();
        client.write("OK");
        break;
      default:
        client.write("NOK, invalid command");
        break;
    }
    client.flush();
    client.stop();
  }
}


void showBufferData(int startIndex, int length) {
  int maxLen = NeoPixelLightStick._maxPixelCount*3;
  for (int i = 0; i < (length<maxLen?length:maxLen); i++) {
    if (i % 16 == 0) {
      Serial.print(F("R "));
      Serial.print(i);
      Serial.print(F(": "));
    }
    Serial.print(F("0x"));
    Serial.print(buffer[i+startIndex], HEX);
    if (i % 16 == 15) {
      Serial.println();
    } else {
      Serial.print(F(" - "));
    }
  }  
  Serial.println();
}


void showStatistics() {
  long last = 0;
  for (int i = 0; i < statCounter; i++) {
    Serial.print(i); Serial.print(F(": time = ")); Serial.print(statistics[i][STAT_ENDED] - statistics[i][STAT_STARTTIME]);
    Serial.print(F(", delay: ")); Serial.print(statistics[i][STAT_DELAY]);
    Serial.print(F(", reading: ")); Serial.print(statistics[i][STAT_STARTSHOW] - statistics[i][STAT_STARTTIME]);
    Serial.print(F(", showing: ")); Serial.print(statistics[i][STAT_ENDED] - statistics[i][STAT_STARTSHOW]);
    Serial.print(F(", between frames: ")); Serial.println(statistics[i][STAT_STARTTIME] - last);
    last = statistics[i][STAT_STARTTIME];
  }
}

