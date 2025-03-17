// --- Libraries ---
#include "config.h"
#include <EEPROM.h>
#include <ETH.h>
#include <WiFi.h>
#include <ArduinoMqttClient.h>
#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

// --- Config to save in EEPROM ---
struct Config {
  int magicNum;
  char deviceMac[CREDENTIAL_LENGTH];
  int deviceId = 0;
  char deviceName[CREDENTIAL_LENGTH];
  char deviceIp[CREDENTIAL_LENGTH];
  int deviceNumSockets = 2;
  char deviceConType[CREDENTIAL_LENGTH];
  char wifiSsid[CREDENTIAL_LENGTH];
  char wifiPassword[CREDENTIAL_LENGTH];
  char mqttServer[CREDENTIAL_LENGTH];
  int mqttPort = 1883;
  char mqttTopic[CREDENTIAL_LENGTH];
};

Config _cfg;

// --- Connectivity Setup---
char _mainTopic[TOPIC_LENGTH];
char _deviceTopic[TOPIC_LENGTH];

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

char _incomingMessage[MESSAGE_LENGTH];
char _pubBuf[MESSAGE_LENGTH];
StaticJsonBuffer<MESSAGE_LENGTH> _jsonPubBuffer;  

bool _connected = false;
bool _mqtt = false;

bool relayState[NUM_SOCKETS];
bool buttonState[NUM_SOCKETS];

bool debugState = false;
bool configState = false;

// --- Setup ---
void setup() {
  setupSerial();
  setupEEPROM();
  deviceInfo();
  setupRelays();
}

// --- Main Loop ---
void loop() {
  watchSerial();
  if (_connected == false) {
    if (strcmp(_cfg.deviceConType, "WIFI") == 0 && strlen(_cfg.wifiPassword) > 0 && strlen(_cfg.wifiSsid) > 0) {
      setupWifi();
    }
    if (strcmp(_cfg.deviceConType, "ETH") == 0) {
    {
      setupEth();
    }
  }

  if (_mqtt == false && _connected == true){
    setupMqtt();
  } 

  // if (WiFi.status() != WL_CONNECTED && ETH_OR_WIFI == 1) {
  //   ledDisconnectedConnection();
  //   setupWifi();
  // }
  // if (!ETH.linkUp() && ETH_OR_WIFI == 0) {
  //   ledDisconnectedConnection();
  //   setupEth();
  // }

  // if (!mqttClient.connected()) {
  //   ledDisconnectedMqtt();
  //   setupMqtt();
  // }

  if (_connected && _mqtt) {
    mqttClient.poll();
    controlFromButtons();
  }
}
