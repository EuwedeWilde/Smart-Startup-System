// --- Initialize EEPROM ---
void setupEEPROM() {
  EEPROM.begin(EEPROM_SIZE);
  if (!readEEPROM()) {
    dbf("No Config Defined\n");
    defineConfig();
    writeEEPROM();
  } 
}

// --- Define Config ---
void defineConfig() {
  getMac();
  strcpy(_cfg.deviceName, "Test123");
  strcpy(_cfg.deviceIp, "192.0.0.200"); // could later be adjusted for server-side changed IP
  strcpy(_cfg.wifiSsid, WIFI_SSID);
  strcpy(_cfg.wifiPassword, WIFI_PASSWORD);
  strcpy(_cfg.mqttServer, MQTT_SERVER);
  strcpy(_cfg.mqttTopic, MQTT_TOPIC);
}

// --- Write to EEPROM ---
void writeEEPROM() {
  dbf("Writing to EEPROM...\n");
  EEPROM.put(EEPROM_CONFIG, _cfg);
  EEPROM.write(EEPROM_CFG_DEFINED, IS_DEFINED);
  EEPROM.commit();
  dbf("Written to EEPROM!\n");
}

// --- Read from EEPROM ---
bool readEEPROM() {
  if (EEPROM.read(EEPROM_CFG_DEFINED) == IS_DEFINED) {
    dbf("Loaded CONFIG from EEPROM\n");
    EEPROM.get(EEPROM_CONFIG, _cfg);
    return true;
  } else {
    dbf("Config EEPROM Empty\n");
    getMac(); 
    return false;
  }
}

// --- Clear EEPROM ---
void clearEEPROM(){
  EEPROM.write(EEPROM_CFG_DEFINED, 255);
  EEPROM.commit();
  dbf(("EEPROM Cleared!\n"));
}
