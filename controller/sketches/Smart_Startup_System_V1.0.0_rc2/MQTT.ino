unsigned long previousMillisMqtt = 0;


void setupMqtt() {
  mqttClient.onMessage(onMqttMessage);
  
  strcpy(_mainTopic, _cfg.mqttTopic);
  sprintf(_deviceTopic, "%s/%s", _mainTopic, _cfg.deviceMac);
  
  uint32_t connectStart = millis();
  dbfo("Connecting to MQTT broker \n");
  while (!mqttClient.connect(_cfg.mqttServer, _cfg.mqttPort) && millis() - connectStart < TIMEOUT) {
    if (millis() - previousMillisMqtt >= WIFI_INTERVAL) {
      previousMillisMqtt = millis();
      dbfo(".");
    }
  }
  
  mqttClient.subscribe(_mainTopic);
  mqttClient.subscribe(_deviceTopic);
  
  dbfo("\nConnected to MQTT broker\n");
  _mqtt = true;
  pubInfo();
}

void onMqttMessage(int messageSize) {
  char topic[50];
  mqttClient.messageTopic().toCharArray(topic, 50);
  byte* payload = new byte[messageSize];
  mqttClient.read(payload, messageSize);
  
  jsonDecoder(topic, payload, messageSize);
  
  delete[] payload;
}

void pubPing() {
  _jsonPubBuffer.clear();
  JsonObject& root = _jsonPubBuffer.createObject();
  root["sender"] = _cfg.deviceMac;
  root["meth"] = "inf";
  root["ping"] = "pong";
  root.printTo(_pubBuf, MESSAGE_LENGTH);
  pubMqttMessage(_pubBuf, _deviceTopic);
  dbfo("Sent simple ping\n");
  dbfo(_pubBuf);
}

void pubInfo() {
  _jsonPubBuffer.clear();
  JsonObject& root = _jsonPubBuffer.createObject();
  root["sender"] = _cfg.deviceMac;
  root["meth"] = "inf";

  JsonObject& info = root.createNestedObject("info");
  info["dId"] = _cfg.deviceId;
  info["dName"] = _cfg.deviceName;
  info["dIp"] = _cfg.deviceIp;
  info["dNumSockets"] = _cfg.deviceNumSockets;

  root.printTo(_pubBuf, MESSAGE_LENGTH);
  pubMqttMessage(_pubBuf, _deviceTopic);
  dbfo("Sent device info\n");
  dbfo("%s \n", _pubBuf);
}

void pubControl() {
  _jsonPubBuffer.clear();
  JsonObject& root = _jsonPubBuffer.createObject();
  root["sender"] = _cfg.deviceMac;
  root["meth"] = "put";
  root["ping"] = "pong";
  
  JsonArray& control = root.createNestedArray("control");
  for (int i = 0; i < NUM_SOCKETS; i++) {
    JsonObject& socketState = control.createNestedObject();
    socketState["sId"] = i + 1; 
    socketState["sState"] = relayState[i];
  }
  
  root.printTo(_pubBuf, MESSAGE_LENGTH);
  pubMqttMessage(_pubBuf, _deviceTopic);
  dbf("%s",_pubBuf);
  dbf("Sent control states\n");
}

void pubMqttMessage(const char* message, const char* topic) {
  mqttClient.beginMessage(topic);
  mqttClient.print(message);
  mqttClient.endMessage();
}

void jsonDecoder(char* topic, byte* payload, unsigned int length) {
  char* payloadStr = new char[length + 1];
  memcpy(payloadStr, payload, length);
  payloadStr[length] = '\0';

  StaticJsonBuffer<MESSAGE_LENGTH> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(payloadStr);
  
  delete[] payloadStr;

  if (!root.success()) {
    dbfo("JSON parsing failed\n");
    return;
  }
  if (strcmp(root["sender"], _cfg.deviceMac) != 0) {
    if (root.containsKey("ping")) {
      if (_connected == false) {
        pubInfo();
      } else {
        pubPing();
      }
    } else if (root.containsKey("config")) {
      setConfig(root);
    } else if (root.containsKey("control")) {
      setControl(root);
    }
  }
}

void setConfig(JsonObject& message) {
  JsonObject& config = message["config"];
  if (config.containsKey("dId")) _cfg.deviceId = config["dId"];
  if (config.containsKey("dName")) strlcpy(_cfg.deviceName, config["dName"], sizeof(_cfg.deviceName));
  if (config.containsKey("dIp")) strlcpy(_cfg.deviceIp, config["dIp"], sizeof(_cfg.deviceIp));
  if (config.containsKey("dNumSockets")) _cfg.deviceNumSockets = config["dNumSockets"];
  writeEEPROM();
  pubInfo();
}

void setControl(JsonObject& message) {
  if (message.containsKey("control")) {
    JsonArray& controlArray = message["control"].as<JsonArray>();
    int controlSize = controlArray.size();
    for (int i = 0; i < controlSize; i++) {
      JsonObject& controlItem = controlArray[i];
      if (controlItem.containsKey("sId") && controlItem.containsKey("sState")) {
        int currentSocketId = controlItem["sId"].as<int>() - 1;
        bool currentSocketState = controlItem["sState"];
        controlFromServer(currentSocketId, currentSocketState);
      }
    }
  }
}



