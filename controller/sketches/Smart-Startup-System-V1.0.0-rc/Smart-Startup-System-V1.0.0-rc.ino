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
  char deviceMac[CREDENTIAL_LENGTH];
  int deviceId = DEVICE_ID;
  char deviceName[CREDENTIAL_LENGTH];
  char deviceIp[CREDENTIAL_LENGTH];
  int deviceNumSockets = NUM_SOCKETS;
  char wifiSsid[CREDENTIAL_LENGTH];
  char wifiPassword[CREDENTIAL_LENGTH];
  char mqttServer[CREDENTIAL_LENGTH];
  int mqttPort = MQTT_PORT;
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

bool relayState[NUM_SOCKETS];
bool buttonState[NUM_SOCKETS];

// --- Setup ---
void setup() {
  Serial.begin(115200);
  setupEEPROM();
  setupRelays();
  if (ETH_OR_WIFI) {
    setupWifi();
  } else {
    setupEth();
  }
}

// --- Main Loop ---
void loop() {
  if (WiFi.status() != WL_CONNECTED && ETH_OR_WIFI == 1) {
    ledDisconnectedConnection();
    setupWifi();
  }
  if (!ETH.linkUp() && ETH_OR_WIFI == 0) {
    ledDisconnectedConnection();
    setupEth();
  }

  if (!mqttClient.connected()) {
    ledDisconnectedMqtt();
    setupMqtt();
  }

  mqttClient.poll();
  controlFromButtons();
}
