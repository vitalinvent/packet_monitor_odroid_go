/*
  ===========================================
       Copyright (c) 2017 Stefan Kremser
              github.com/spacehuhn
  ===========================================
*/

#include <odroid_go.h>
/* include all necessary libraries */
#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_wifi_internal.h"
#include "lwip/err.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

#include <Arduino.h>
#include <TimeLib.h>
#include "FS.h"
//#include "SD.h"
#include "SPI.h"
#include <PCAP.h>

//===== SETTINGS =====//
#define CHANNEL 1
#define FILENAME "pak"
#define SAVE_INTERVAL 60 //save new file every 30s
#define CHANNEL_HOPPING true //if true it will scan on all channels
#define MAX_CHANNEL 11 //(only necessary if channelHopping is true)
#define HOP_INTERVAL 214 //in ms (only necessary if channelHopping is true)

#define SSIDLENPOS 25
#define SOURCEMACPOS 10

#define MAX_X 320
#define MAX_Y 231

//===== Run-Time variables =====//
unsigned long lastTime = 0;
unsigned long lastChannelChange = 0;
int counter = 0;
int ch = CHANNEL;
bool fileOpen = false;
byte lcdLineCount = 0;
bool snifferRunning = true;
uint32_t tmpPacketCounter;
uint32_t pkts[MAX_X];       // here the packets per second will be saved

PCAP pcap = PCAP();

//===== FUNCTIONS =====//

/* will be executed on every packet the ESP32 gets while beeing in promiscuous mode */
void sniffer(void *buf, wifi_promiscuous_pkt_type_t type) {
  tmpPacketCounter++;
//  if (fileOpen) {
//    wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*)buf;
//    wifi_pkt_rx_ctrl_t ctrl = (wifi_pkt_rx_ctrl_t)pkt->rx_ctrl;
//
//    uint32_t timestamp = now(); //current timestamp
//    uint32_t microseconds = (unsigned int)(micros() - millis() * 1000); //micro seconds offset (0 - 999)
//    pcap.newPacketSD(timestamp, microseconds, ctrl.sig_len, pkt->payload); //write packet to file
//    byte pktType = pkt->payload[0];
//
//    if (pktType == 0x40) {
//      if (lcdLineCount > 30) {
//        GO.lcd.clearDisplay();
//        GO.lcd.setCursor(0, 0);
//        lcdLineCount = 0;
//      }
//
//      byte ssidLen = pkt->payload[SSIDLENPOS];
//      if (ssidLen > 0) {
//        byte sourceMac[6];
//        Serial.print("Source MAC: ");
//        for (byte i = 0; i < 6; i++) {
//          sourceMac[i] = pkt->payload[SOURCEMACPOS + i];
//          Serial.print(sourceMac[i], HEX);
//          GO.lcd.print(sourceMac[i], HEX);
//          if (i < 5) {
//            Serial.print(":");
//            GO.lcd.print(":");
//          }
//        }
//        GO.lcd.print(" ");
//        Serial.print(" ");
//
//        char ssidName[64];
//        for (byte i = 0; i < ssidLen; i++) {
//          ssidName[i] = pkt->payload[SSIDLENPOS + 1 + i];
//        }
//        ssidName[ssidLen] = '\0';
//        Serial.print(ssidName);
//        GO.lcd.print(ssidName);
//
//        GO.lcd.println("");
//        Serial.println("");
//        lcdLineCount++;
//      }
//    }
//  }
}

esp_err_t event_handler(void *ctx, system_event_t *event) {
  return ESP_OK;
}

/* opens a new file */
void openFile() {

  //searches for the next non-existent file name
  int c = 0;
  String filename = "/" + (String)FILENAME + ".pcap";
  while (SD.open(filename)) {
    filename = "/" + (String)FILENAME + "_" + (String)c + ".pcap";
    c++;
  }

  //set filename and open the file
  pcap.filename = filename;
  fileOpen = pcap.openFile(SD);

  Serial.println("opened: " + filename);
  GO.lcd.println("opened: " + filename);
  //reset counter (counter for saving every X seconds)
  counter = 0;
}

//===== SETUP =====//
void setup() {
  GO.begin();
//  GO.lcd.setTextFont(2);
  GO.lcd.setTextSize(3);
  GO.lcd.setTextColor(LIGHTGREY);
  GO.lcd.println("   Packet monitor");
  GO.lcd.setTextColor(DARKGREY);
  GO.lcd.setTextSize(1);
  GO.lcd.println(" Odroid GO vitalinvent (c) 2022");
  GO.lcd.println(" ________  __    __    _       _");
  GO.lcd.setTextColor(LIGHTGREY);
  GO.lcd.setTextSize(2);
  GO.lcd.println("");
  GO.lcd.println("A - next channel B - previous channel");
  GO.lcd.println("");
  GO.lcd.println("");
  GO.lcd.println("");
  GO.lcd.println("");
  GO.lcd.println("");
  GO.lcd.println("Press any key to continue");

  boolean pressed=false;
  int cnt=0;
  while(pressed){
    if(GO.JOY_Y.isPressed() || GO.JOY_X.isPressed() || GO.BtnA.isPressed() || 
        GO.BtnB.isPressed() || GO.BtnMenu.isPressed() || GO.BtnVolume.isPressed() || 
        GO.BtnSelect.isPressed() || GO.BtnStart.isPressed()) {pressed=true;}
    dekay(1000);
    if (cnt>10) {pressed=true;}
    cnt++;
  }
  Serial.println();

//  /* initialize SD card */
//  if (!SD.begin()) {
//    Serial.println("Card Mount Failed");
//    GO.lcd.println("Card Mount Failed");
//    return;
//  }
//
//  uint8_t cardType = SD.cardType();
//
//  if (cardType == CARD_NONE) {
//    Serial.println("No SD card attached");
//    GO.lcd.println("No SD card attached");
//    return;
//  }

//  Serial.print("SD Card Type: ");
//  if (cardType == CARD_MMC) {
//    Serial.println("MMC");
//  } else if (cardType == CARD_SD) {
//    Serial.println("SDSC");
//  } else if (cardType == CARD_SDHC) {
//    Serial.println("SDHC");
//  } else {
//    Serial.println("UNKNOWN");
//  }
//
//  int64_t cardSize = SD.cardSize() / (1024 * 1024);
//  Serial.printf("SD Card Size: %lluMB\n", cardSize);
//
//  openFile();

  /* setup wifi */
  nvs_flash_init();
  tcpip_adapter_init();
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
  ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
  ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
  ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_AP) );
  ESP_ERROR_CHECK( esp_wifi_start() );
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(sniffer);
  wifi_second_chan_t secondCh = (wifi_second_chan_t)NULL;
  esp_wifi_set_channel(ch, secondCh);

  Serial.println("Sniffer started!");
  GO.lcd.println("Sniffer started!");
}

double getMultiplicator() {
  uint32_t maxVal = 1;
  for (int i = 0; i < MAX_X; i++) {
    if (pkts[i] > maxVal) maxVal = pkts[i];
  }
  if (maxVal > MAX_Y) return (double)MAX_Y / (double)maxVal;
  else return 5;
}

void loop() {
  unsigned long currentTime = millis();
  GO.update();
  /* for every xxx miliseconds */
  if (currentTime - lastTime > 500) {
    lastTime = currentTime; //update time
    tmpPacketCounter = 0;
    GO.lcd.clearDisplay();

    GO.lcd.drawString("channel: "+(String)ch,0,231); 
    pkts[MAX_X - 1] = tmpPacketCounter;
    int len;
    GO.lcd.drawLine(0, 230 - MAX_Y, MAX_X, 230 - MAX_Y,ILI9341_DARKGREY);
    for (int i = 0; i < MAX_X; i++) {
      len = pkts[i] * getMultiplicator();
      GO.lcd.setTextSize(1);
      GO.lcd.drawLine(i, 230, i, 230 - (len > MAX_Y ? MAX_Y : len),ILI9341_DARKGREY);
      if (i < MAX_X - 1) pkts[i] = pkts[i + 1];
    }
  }
  
  if (GO.BtnA.isPressed()) {
    ch++; //increase channel
    if (ch > MAX_CHANNEL) ch = 1;
    tmpPacketCounter = 0;
    GO.lcd.clearDisplay();
    wifi_second_chan_t secondCh = (wifi_second_chan_t)NULL;
    esp_wifi_set_channel(ch, secondCh);
  }

  if (GO.BtnB.isPressed()) {
    ch--; //derease channel
    if (ch < 0) ch = MAX_CHANNEL;
    tmpPacketCounter = 0;
    GO.lcd.clearDisplay();
    wifi_second_chan_t secondCh = (wifi_second_chan_t)NULL;
    esp_wifi_set_channel(ch, secondCh);
  }

  
//
//  /* for every second */
//  if (fileOpen && currentTime - lastTime > 1000 && snifferRunning) {
//    pcap.flushFile(); //save file
//    lastTime = currentTime; //update time
//    counter++; //add 1 to counter
//  }
//  /* when counter > 30s interval */
//  if (fileOpen && counter > SAVE_INTERVAL && snifferRunning) {
//    pcap.closeFile(); //save & close the file
//    fileOpen = false; //update flag
//    Serial.println("==================");
//    Serial.println(pcap.filename + " saved!");
//    Serial.println("==================");
//    GO.lcd.println(pcap.filename + " saved!");
//    openFile(); //open new file
//  }

//  if (GO.BtnB.isPressed() && snifferRunning) {
//    GO.lcd.clearDisplay();
//    GO.lcd.setCursor(0, 0);
//    lcdLineCount = 0;
//    GO.lcd.println("Sniffer stopped!");
//    Serial.println("Sniffer stopped!");
//
//    pcap.closeFile(); //save & close the file
//    fileOpen = false; //update flag
//    Serial.println("==================");
//    Serial.println(pcap.filename + " saved!");
//    Serial.println("==================");
//    GO.lcd.println(pcap.filename + " saved!");
//
//    snifferRunning = false;
//  }
//  if (GO.BtnA.isPressed() && !snifferRunning) {
//    GO.lcd.clearDisplay();
//    GO.lcd.setCursor(0, 0);
//    lcdLineCount = 0;
//    GO.lcd.println("Sniffer started!");
//    Serial.println("Sniffer started!");
//
//    counter = 0;
//    fileOpen = false; //update flag
//    openFile(); //open new file
//
//    snifferRunning = true;
//  }
}
