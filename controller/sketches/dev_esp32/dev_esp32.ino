#include "config.h"
#include <ArduinoMqttClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <EEPROM.h> 

#define TOPIC_LENGTH 32
#define CREDENTIAL_LENGTH 32
#define MESSAGE_LENGTH 2048

#define EEPROM_CFG_DEFINED 8
#define EEPROM_CONFIG 10  
#define EEPROM_SIZE 1024
#define IS_DEFINED 6

#define sp Serial.print
#define spl Serial.println
#define spf Serial.printf

#define MAIN_TOPIC "S3"

#define SERVER_NAME "server"

struct Socket {
  char oName[CREDENTIAL_LENGTH];
  bool oState;
  char oGroup[CREDENTIAL_LENGTH];
};

struct Config {
  char mac[13];
  int id;
  char name[CREDENTIAL_LENGTH];
  char ipAdr[16];
  char project[CREDENTIAL_LENGTH];
  int dNumSockets;
  Socket sockets[MAX_SOCKETS];
};

Config _cfg;
char DEVICE_TOPIC[TOPIC_LENGTH];

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

char _incomingMessage[MESSAGE_LENGTH];
char _pubBuf[MESSAGE_LENGTH];
StaticJsonBuffer<MESSAGE_LENGTH> _jsonPubBuffer;  

bool isConnected = false;

void setup() {
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);
  
  WiFi.mode(WIFI_STA);
  WiFi.STA.begin();
  String macString = WiFi.macAddress();
  char macStr[13];
  macString.replace(":", "");
  macString.toCharArray(macStr, 13);
  spf("Mac: %s\n", macStr);
  
  loadConfig(macStr);
  setupWiFi();
  setupMQTT();
  
  snprintf(DEVICE_TOPIC, TOPIC_LENGTH, "%s/%s", MAIN_TOPIC, macStr);
  
  mqttClient.onMessage(onMqttMessage);
  mqttClient.subscribe(MAIN_TOPIC);
  mqttClient.subscribe(DEVICE_TOPIC);
}

void onMqttMessage(int messageSize) {
  char topic[50];
  mqttClient.messageTopic().toCharArray(topic, 50);
  

  byte* payload = new byte[messageSize];
  mqttClient.read(payload, messageSize);
  
  jsonDecoder(topic, payload, messageSize);
  
  delete[] payload;
}

void jsonDecoder(char* topic, byte* payload, unsigned int length) {
  char* payloadStr = new char[length + 1];
  memcpy(payloadStr, payload, length);
  payloadStr[length] = '\0';

  StaticJsonBuffer<MESSAGE_LENGTH> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(payloadStr);
  
  delete[] payloadStr;

  if (!root.success()) {
    spl("JSON parsing failed");
    return;
  }
  
  if (strcmp(root["sender"], _cfg.mac) != 0) {
    if (root.containsKey("ping")) {
      pingHandle(root);
    } else if (root.containsKey("info")) {
      infoHandle(root);
    } else if (root.containsKey("config")) {
      configHandle(root);
    } else if (root.containsKey("control")) {
      controlHandle(root);
    }
  }
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    setupWiFi();
  }
  
  if (!mqttClient.connected()) {
    setupMQTT();
  }
  
  mqttClient.poll();
}

void setupWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    spl("Connecting to WiFi...");
  }
  spl("Connected to WiFi");
  strcpy(_cfg.ipAdr, WiFi.localIP().toString().c_str());
}

void setupMQTT() {
  while (!mqttClient.connect(MQTT_SERVER, 1883)) {
    spl("Connecting to MQTT broker...");
    delay(1000);
  }
  spl("Connected to MQTT broker");
}

void loadConfig(const char* macAddress) {
  if (EEPROM.read(EEPROM_CFG_DEFINED) == IS_DEFINED) {
    EEPROM.get(EEPROM_CONFIG, _cfg);
    spl("Config loaded from EEPROM");
    
    if (strcmp(_cfg.mac, macAddress) != 0) {
      strncpy(_cfg.mac, macAddress, sizeof(_cfg.mac) - 1);
      _cfg.mac[sizeof(_cfg.mac) - 1] = '\0';
      
      _cfg.id = DEVICE_ID;
      strncpy(_cfg.name, DEVICE_NAME, sizeof(_cfg.name) - 1);
      _cfg.name[sizeof(_cfg.name) - 1] = '\0';
      _cfg.dNumSockets = MAX_SOCKETS;
      
      saveConfig();
      spl("MAC address and other fields updated in config");
    }
  } else {
    strncpy(_cfg.mac, macAddress, sizeof(_cfg.mac) - 1);
    _cfg.mac[sizeof(_cfg.mac) - 1] = '\0';
    _cfg.id = DEVICE_ID;
    strncpy(_cfg.name, DEVICE_NAME, sizeof(_cfg.name) - 1);
    _cfg.name[sizeof(_cfg.name) - 1] = '\0';
    strncpy(_cfg.project, MQTT_TOPIC, sizeof(_cfg.project) - 1);
    _cfg.project[sizeof(_cfg.project) - 1] = '\0';
    _cfg.dNumSockets = MAX_SOCKETS;
    for (int i = 0; i < MAX_SOCKETS; i++) {
      snprintf(_cfg.sockets[i].oName, sizeof(_cfg.sockets[i].oName), "Socket %d", i + 1);
      _cfg.sockets[i].oState = false;
      _cfg.sockets[i].oGroup[0] = '\0';
    }
    saveConfig();
    spl("Default config created and saved");
  }
  
  if (_cfg.id == 0) _cfg.id = DEVICE_ID;
  if (_cfg.name[0] == '\0') strncpy(_cfg.name, DEVICE_NAME, sizeof(_cfg.name) - 1);
  if (_cfg.dNumSockets == 0) _cfg.dNumSockets = MAX_SOCKETS;
  
  spf("Loaded config: MAC=%s, ID=%d, Name=%s, Sockets=%d\n", _cfg.mac, _cfg.id, _cfg.name, _cfg.dNumSockets);
}

void saveConfig() {
  EEPROM.put(EEPROM_CONFIG, _cfg);
  EEPROM.write(EEPROM_CFG_DEFINED, IS_DEFINED);
  EEPROM.commit();
  spl("Config saved to EEPROM");
}

void pubMqttMessage(const char* message, const char* topic) {
  mqttClient.beginMessage(topic);
  mqttClient.print(message);
  mqttClient.endMessage();
}

void pingHandle(JsonObject& message) {
  if (strcmp(message["sender"], SERVER_NAME) == 0) {
    if (message.containsKey("ping")) {
      if (isConnected) {
        sendSimplePing();
      } else {
        sendPingWithInfo();
        isConnected = true;
      }
    }
  }
}

void infoHandle(JsonObject& message) {
  if (strcmp(message["sender"], SERVER_NAME) == 0 && message.containsKey("info")) {
    sendPingWithInfo();
    isConnected = true;
  }
}

void sendPingWithInfo() {
  _jsonPubBuffer.clear();
  StaticJsonBuffer<MESSAGE_LENGTH> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  root["sender"] = _cfg.mac;
  root["meth"] = "put";
  
  JsonObject& info = root.createNestedObject("info");
  info["dMac"] = _cfg.mac;
  info["dId"] = _cfg.id;
  info["dName"] = _cfg.name;
  info["dIp"] = _cfg.ipAdr;
  info["dNumSockets"] = _cfg.dNumSockets;
  
  JsonArray& sockets = info.createNestedArray("dSockets");
  for (int i = 0; i < _cfg.dNumSockets; i++) {
    JsonObject& socket = sockets.createNestedObject();
    socket["sId"] = i + 1;
    socket["sName"] = _cfg.sockets[i].oName;
    socket["sState"] = _cfg.sockets[i].oState;
    socket["sGroup"] = _cfg.sockets[i].oGroup;
  }
  
  root.printTo(_pubBuf, sizeof(_pubBuf));
  pubMqttMessage(_pubBuf, DEVICE_TOPIC);
  spl("Sent device info");
  
  spf("Sent info message: %s\n", _pubBuf);
}

void sendSimplePing() {
  StaticJsonBuffer<MESSAGE_LENGTH> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["sender"] = _cfg.mac;
  root["meth"] = "inf";
  root["ping"] = "pong";

  char pingMessage[MESSAGE_LENGTH];
  root.printTo(pingMessage, MESSAGE_LENGTH);
  pubMqttMessage(pingMessage, DEVICE_TOPIC);
  spl("Sent simple ping");
}

void configHandle(JsonObject& message) {
  if (strcmp(message["sender"], SERVER_NAME) == 0 && message.containsKey("config")) {
    JsonObject& config = message["config"];
    if (strcmp(config["dMac"], _cfg.mac) == 0) {
      if (config.containsKey("dName")) strcpy(_cfg.name, config["dName"]);
      if (config.containsKey("dNumSockets")) _cfg.dNumSockets = config["dNumSockets"];
      
      if (config.containsKey("dSockets")) {
        JsonArray& sockets = config["dSockets"];
        for (JsonArray::iterator it = sockets.begin(); it != sockets.end(); ++it) {
          JsonObject& socket = *it;
          int sId = socket["sId"];
          if (sId > 0 && sId <= MAX_SOCKETS) {
            int index = sId - 1;
            if (socket.containsKey("sName")) strcpy(_cfg.sockets[index].oName, socket["sName"]);
            if (socket.containsKey("sGroup")) strcpy(_cfg.sockets[index].oGroup, socket["sGroup"]);
          }
        }
      }
      
      saveConfig();
      sendPingWithInfo();
    }
  }
}

void controlHandle(JsonObject& message) {
  if (strcmp(message["sender"], SERVER_NAME) == 0 && message.containsKey("control")) {
    JsonObject& control = message["control"];
    if (strcmp(control["dMac"], _cfg.mac) == 0) {
      if (control.containsKey("dSockets")) {
        JsonArray& sockets = control["dSockets"];
        for (JsonArray::iterator it = sockets.begin(); it != sockets.end(); ++it) {
          JsonObject& socket = *it;
          int sId = socket["sId"];
          if (sId > 0 && sId <= MAX_SOCKETS) {
            int index = sId - 1;
            if (socket.containsKey("sState")) {
              _cfg.sockets[index].oState = socket["sState"];
              digitalWrite(SOCKET_PINS[index], _cfg.sockets[index].oState ? HIGH : LOW);
            }
          }
        }
      }
      sendPingWithInfo();
    }
  }
}