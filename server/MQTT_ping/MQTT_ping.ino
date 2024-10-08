#include "config.h"
#include <ArduinoMqttClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include "EEPROM.h"

#define TOPIC_LENGTH 32
#define CREDENTIAL_LENGTH 32
#define MESSAGE_LENGTH 1024

#define EEPROM_CFG_DEFINED 8
#define EEPROM_CONFIG 10  
#define EEPROM_SIZE 1024
#define IS_DEFINED 6

#define sp Serial.print
#define spl Serial.println
#define spf Serial.printf

char _mainTopic[TOPIC_LENGTH];
char _configTopic[TOPIC_LENGTH];
char _infoTopic[TOPIC_LENGTH];
char _pingTopic[TOPIC_LENGTH];
char _controlTopic[TOPIC_LENGTH];

char _subMessage[MESSAGE_LENGTH];
char _currentTopic[TOPIC_LENGTH];

unsigned long previousMillis = 0;

struct Outlet {
  int oId;  // New field for outlet ID
  char oName[CREDENTIAL_LENGTH];
  bool oState;
  char oGroup[CREDENTIAL_LENGTH];
};

struct Config {
  char project[CREDENTIAL_LENGTH];
  int port = 1883;
  int id = DEVICE_ID;
  char macAdr[CREDENTIAL_LENGTH];
  char ipAdr[CREDENTIAL_LENGTH];
  char name[CREDENTIAL_LENGTH];
  char mqtt_server[CREDENTIAL_LENGTH];
  char dGroup[CREDENTIAL_LENGTH];
  int dSockets = MAX_SOCKETS;
  Outlet outlets[MAX_SOCKETS];
};

Config _cfg;

NetworkClient espClient;
MqttClient mqttClient(espClient);

StaticJsonBuffer<MESSAGE_LENGTH> jsonBuffer;

void setup() {
  Serial.begin(115200);
  spl("Hello World!");
  strcpy(_cfg.project, MQTT_TOPIC);
  strcpy(_cfg.mqtt_server, MQTT_SERVER);
  strcpy(_cfg.name, DEVICE_NAME);
  initEEPROM();
  initWifi();
  mqttClient.subscribe(_infoTopic);
  mqttClient.subscribe(_configTopic);
  mqttClient.subscribe(_pingTopic);
  mqttClient.subscribe(_controlTopic);

  char macBuffer[32]; 
  char ipBuffer[CREDENTIAL_LENGTH]; 
  uint8_t mac[CREDENTIAL_LENGTH]; 

  WiFi.macAddress(mac); 
  snprintf(macBuffer, sizeof(macBuffer), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  IPAddress ip = WiFi.localIP(); 
  snprintf(ipBuffer, sizeof(ipBuffer), "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);

  strcpy(_cfg.macAdr, macBuffer);
  strcpy(_cfg.ipAdr, ipBuffer);

  // Initialize outlet IDs and names if they haven't been set
  for (int i = 0; i < _cfg.dSockets; i++) {
    _cfg.outlets[i].oId = i;  // Set the ID to the index
    if (strlen(_cfg.outlets[i].oName) == 0) {
      snprintf(_cfg.outlets[i].oName, CREDENTIAL_LENGTH, "Outlet_%d", i + 1);
    }
    if (strlen(_cfg.outlets[i].oGroup) == 0) {
      snprintf(_cfg.outlets[i].oGroup, CREDENTIAL_LENGTH, "Group_%d", (i % 2) + 1);
    }
  }

  for (int i = 0; i < _cfg.dSockets; i++) {
    pinMode(SOCKET_PINS[i], OUTPUT);
    digitalWrite(SOCKET_PINS[i], _cfg.outlets[i].oState ? HIGH : LOW);
  }

  char pingMessage[MESSAGE_LENGTH];
  sprintf(pingMessage, "{\"did\":%i, \"name\":\"%s\"}", _cfg.id, _cfg.name);
  pubMqttMessage(pingMessage, _pingTopic);
}

void loop() {
  mqttClient.poll();
  toggleOutputs();
}

void initEEPROM() {
  EEPROM.begin(EEPROM_SIZE);
  if (!readEEPROM()) {
    spl("No Config Defined");
    writeEEPROM();
  } 
}

void writeEEPROM() {
  spl("Writing to EEPROM...");
  EEPROM.put(EEPROM_CONFIG, _cfg);
  EEPROM.write(EEPROM_CFG_DEFINED, IS_DEFINED);
  if (EEPROM.commit()) {
    spl("EEPROM successfully committed");
  } else {
    spl("ERROR! EEPROM commit failed");
  }
}

bool readEEPROM(){
  if (EEPROM.read(EEPROM_CFG_DEFINED) == IS_DEFINED ){
    spl("Loaded CONFIG from EEPROM");
    EEPROM.get(EEPROM_CONFIG, _cfg);
    return true;
  } else {
    Serial.println(("Config EEPROM Empty"));
    return false;
  }
}

void clearEEPROM(){
  EEPROM.write(EEPROM_CFG_DEFINED, 255);
  EEPROM.commit();
  Serial.println(("EEPROM Cleared!"));
}

void initWifi() {
  sp("Configuring WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  uint32_t connectStart = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - connectStart < 20000) {
    if (millis() - previousMillis >= WIFI_INTERVAL) {
      previousMillis = millis();
      sp(".");
    }
  }
  spl("");
  spl("WiFi connected");
  if (!initMQTT()) {
    spl("MQTT setup failed...");
  }
}

bool initMQTT() {
  if (!mqttClient.connect(_cfg.mqtt_server, _cfg.port)) {
    spf("MQTT connection failed! Error code = %i\n", mqttClient.connectError());
    while (1);
  }
  sprintf(_mainTopic, "%s/#", _cfg.project);
  sprintf(_configTopic, "%s/config", _cfg.project);
  sprintf(_infoTopic, "%s/info", _cfg.project);
  sprintf(_pingTopic, "%s/ping", _cfg.project);
  sprintf(_controlTopic, "%s/control", _cfg.project);
  
  spl("Connection established with MQTT broker!");
  mqttClient.onMessage(onMqttMessage);
  return true;
}

void onMqttMessage(int messageSize) {
  (mqttClient.messageTopic()).toCharArray(_currentTopic,TOPIC_LENGTH);;
  int charMsgPos = 0;
  while (mqttClient.available()) {
    char readChar = (char)mqttClient.read();
    _subMessage[charMsgPos] = readChar;
    charMsgPos++;
  }
  _subMessage[charMsgPos] = '\0';
  spl(_subMessage);
  jsonDecoder();
  memset(_subMessage, 0, MESSAGE_LENGTH);
  memset(_currentTopic, 0, TOPIC_LENGTH);
}

void pubMqttMessage(char pubMessage[], char subTopic[]) {
  mqttClient.beginMessage(subTopic);
  mqttClient.printf(pubMessage);
  mqttClient.endMessage();
}

void jsonDecoder() {
  StaticJsonBuffer<MESSAGE_LENGTH> jsonBuffer;
  JsonObject& message = jsonBuffer.parseObject(_subMessage);

  if (!message.success()) {
    spl("Contains no JSON or is invalid!");
    return;
  }
  if (message["did"] != DEVICE_ID){
    if (strcmp(_currentTopic, _pingTopic) == 0) {
      pingHandle(message);
    } else if (strcmp(_currentTopic, _infoTopic) == 0) {
      infoHandle(message);
    } else if (strcmp(_currentTopic, _configTopic) == 0) {
      configHandle(message);
    } else if (strcmp(_currentTopic, _controlTopic) == 0) {
      controlHandle(message);
    }
  }
}

void pingHandle(JsonObject& message) {
  if (message["did"] == SERVER_ID && message["ping"] == "ALL") {
    char pingMessage[MESSAGE_LENGTH];
    sprintf(pingMessage, "{\"did\":%i, \"name\":\"%s\"}", _cfg.id, _cfg.name);
    pubMqttMessage(pingMessage, _pingTopic);
  }
}

void infoHandle(JsonObject& message) {
  if (message["did"] == SERVER_ID && message["requestInfo"] == DEVICE_ID) {
    StaticJsonBuffer<MESSAGE_LENGTH> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    
    root["did"] = _cfg.id;
    root["mac"] = _cfg.macAdr;
    root["dname"] = _cfg.name;
    root["ip"] = _cfg.ipAdr;
    root["numSockets"] = _cfg.dSockets;
    
    JsonArray& sockets = root.createNestedArray("sockets");
    for (int i = 0; i < _cfg.dSockets; i++) {
      JsonObject& socket = sockets.createNestedObject();
      socket["id"] = _cfg.outlets[i].oId;  // Include the ID in the response
      socket["name"] = _cfg.outlets[i].oName;
      socket["state"] = _cfg.outlets[i].oState;
      socket["group"] = _cfg.outlets[i].oGroup;
    }
    
    char infoMessage[MESSAGE_LENGTH];
    root.printTo(infoMessage, MESSAGE_LENGTH);
    pubMqttMessage(infoMessage, _infoTopic);
  }
}

void configHandle(JsonObject& message) {
  if (message["sId"] == SERVER_ID && message.containsKey("dConfig")) {
    JsonObject& configMsg = message["dConfig"];
    if (configMsg["dId"] == _cfg.id && strcmp(configMsg["dMac"], _cfg.macAdr) == 0) {
      bool configChanged = false;

      if (strcmp(configMsg["dName"], _cfg.name) != 0) {
        strcpy(_cfg.name, configMsg["dName"]);
        configChanged = true;
      }
      if (strcmp(configMsg["dIp"], _cfg.ipAdr) != 0) {
        strcpy(_cfg.ipAdr, configMsg["dIp"]);
        configChanged = true;
      }

      if (configMsg.containsKey("sockets")) {
        JsonArray& socketArray = configMsg["sockets"];
        for (int i = 0; i < _cfg.dSockets && i < socketArray.size(); i++) {
          JsonObject& socketConfig = socketArray[i];
          if (socketConfig.containsKey("name")) {
            if (strcmp(_cfg.outlets[i].oName, socketConfig["name"]) != 0) {
              strncpy(_cfg.outlets[i].oName, socketConfig["name"], CREDENTIAL_LENGTH);
              configChanged = true;
            }
          }
          if (socketConfig.containsKey("group")) {
            if (strcmp(_cfg.outlets[i].oGroup, socketConfig["group"]) != 0) {
              strncpy(_cfg.outlets[i].oGroup, socketConfig["group"], CREDENTIAL_LENGTH);
              configChanged = true;
            }
          }
          if (socketConfig.containsKey("state")) {
            if (_cfg.outlets[i].oState != socketConfig["state"]) {
              _cfg.outlets[i].oState = socketConfig["state"];
              configChanged = true;
            }
          }
        }
      }

      if (configChanged) {
        writeEEPROM();
        spl("Config updated and saved to EEPROM");
        
        // Send a confirmation message back to the server
        StaticJsonBuffer<MESSAGE_LENGTH> jsonBuffer;
        JsonObject& root = jsonBuffer.createObject();
        root["did"] = _cfg.id;
        root["status"] = "updated";
        root["name"] = _cfg.name;
        root["ip"] = _cfg.ipAdr;
        
        JsonArray& outletArray = root.createNestedArray("outlets");
        for (int i = 0; i < _cfg.dSockets; i++) {
          JsonObject& outlet = outletArray.createNestedObject();
          outlet["name"] = _cfg.outlets[i].oName;
          outlet["group"] = _cfg.outlets[i].oGroup;
          outlet["state"] = _cfg.outlets[i].oState;
        }
        
        char confirmMessage[MESSAGE_LENGTH];
        root.printTo(confirmMessage, MESSAGE_LENGTH);
        pubMqttMessage(confirmMessage, _configTopic);
      } else {
        spl("No changes in config");
      }
    }
  }
}

void controlHandle(JsonObject& message) {
  if (message["sId"] == SERVER_ID) {
    JsonObject& controlMsg = message["dControl"];
    if (controlMsg["did"] == _cfg.id && strcmp(controlMsg["mac"], _cfg.macAdr) == 0) {
      int socketId = controlMsg["socketId"];  // Use socketId instead of socketName
      bool newState = controlMsg["state"];
      
      if (socketId >= 0 && socketId < _cfg.dSockets) {
        _cfg.outlets[socketId].oState = newState;
        digitalWrite(SOCKET_PINS[socketId], newState ? HIGH : LOW);
        
        // Send confirmation message
        StaticJsonBuffer<MESSAGE_LENGTH> jsonBuffer;
        JsonObject& root = jsonBuffer.createObject();
        root["did"] = _cfg.id;
        root["socketId"] = socketId;
        root["socketName"] = _cfg.outlets[socketId].oName;
        root["newState"] = newState;
        root["status"] = "updated";
        
        char confirmMessage[MESSAGE_LENGTH];
        root.printTo(confirmMessage, MESSAGE_LENGTH);
        pubMqttMessage(confirmMessage, _controlTopic);
        
        spl("Socket state updated");
        writeEEPROM();  // Save the new state to EEPROM
      } else {
        spl("Invalid socket ID");
      }
    }
  }
}

void toggleOutputs() {
  for (int i = 0; i < _cfg.dSockets; i++) {
    if (_cfg.outlets[i].oState) {
      digitalWrite(SOCKET_PINS[i], HIGH);
    } else {
      digitalWrite(SOCKET_PINS[i], LOW);
    }
  }
}