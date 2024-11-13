unsigned long previousMillisWifi = 0;

// --- Get mac-adress ---
void getMac() {
//  WiFi.mode(WIFI_STA);
//  WiFi.begin();  
  String macString = WiFi.macAddress();
  macString.replace(":", "");
  strcpy(_cfg.deviceMac, macString.c_str());
}

// --- Get ip-adress ---


// --- Setup Wifi ---
void setupWifi() {
  dbf("Configuring WiFi\n");
  WiFi.begin(_cfg.wifiSsid, _cfg.wifiPassword);
  uint32_t connectStart = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - connectStart < TIMEOUT) {
    if (millis() - previousMillisWifi >= WIFI_INTERVAL) {
      previousMillisWifi = millis();
      dbf(".");
    }
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    dbf("\nWifi Connected\n");
    _connected = true;
  } else {
    dbf("\nWifi Connection Failed\n");
    _connected = false;
  }
  setupMqtt();
}
