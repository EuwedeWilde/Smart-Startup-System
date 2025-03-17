// --- Initialize EEPROM ---
void setupEEPROM() {
  EEPROM.begin(EEPROM_SIZE);
}

// --- Write to EEPROM ---
void writeEEPROM() {
  dbfo("Writing to EEPROM...\n");
  _cfg.magicNum = MAGIC_NUM;  
  EEPROM.put(EEPROM_CONFIG, _cfg);  
  EEPROM.commit();
  dbfo("Written to EEPROM!\n");
}

// --- Read from EEPROM ---
bool readEEPROM() {
  EEPROM.get(EEPROM_CONFIG, _cfg); 
  
  if (_cfg.magicNum == MAGIC_NUM) {
    dbfo("EEPROM data is valid.\n");
    return true;  
  } else {
    dbfo("EEPROM data is invalid.\n");
    return false;  
  }
}
// --- Clear EEPROM ---
void clearEEPROM(){
  for (int i = 0; i < EEPROM_SIZE; i++) {
    EEPROM.write(i, 0); // Write 0 to each address
  }  
  EEPROM.commit();
  dbfo(("EEPROM Cleared!\n"));
}
