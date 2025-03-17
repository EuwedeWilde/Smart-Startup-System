unsigned long previousMillisWifi = 0;

// --- Get mac-adress ---
void getMac() {
  char mac[13];  // 12 characters for MAC + null terminator
  uint64_t chipid = ESP.getEfuseMac();
  sprintf(mac, "%02X%02X%02X%02X%02X%02X", 
    ((uint8_t)(chipid)),
    ((uint8_t)(chipid >> 8)),
    ((uint8_t)(chipid >> 16)), 
    ((uint8_t)(chipid >> 24)),
    ((uint8_t)(chipid >> 32)),
    ((uint8_t)(chipid >> 40)));
  strcpy(_cfg.deviceMac, mac);
  writeEEPROM();
  dbfo("Our Mac is: %s\n", mac);
}

// --- Setup Wifi ---
void setupWifi() {
  dbfo("Configuring WiFi\n");
  WiFi.begin(_cfg.wifiSsid, _cfg.wifiPassword);
  uint32_t connectStart = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - connectStart < TIMEOUT) {
    if (millis() - previousMillisWifi >= WIFI_INTERVAL) {
      previousMillisWifi = millis();
      dbfo(".");
    }
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    dbfo("\nWifi Connected\n");
    _connected = true;
    setupMqtt();
  } else {
    dbfo("\nWifi Connection Failed\n");
    _connected = false;
  }
}
