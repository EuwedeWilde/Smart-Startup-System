void setupSerial() {
  Serial.begin(115200);
  dbf(F(
    "----------------------------------------------------------------------------------------------------------------------\n"
    "   _____ __  ______    ____  ______   ______________    ____  ________  ______     _______  _____________________  ___\n"
    "  / ___//  |/  /   |  / __ \\/_  __/  / ___/_  __/   |  / __ \\/_  __/ / / / __ \\   / ___/\\ \\/ / ___/_  __/ ____/  |/  /\n"
    "  \\__ \\/ /|_/ / /| | / /_/ / / /     \\__ \\ / / / /| | / /_/ / / / / / / / /_/ /   \\__ \\  \\  /\\__ \\ / / / __/ / /|_/ / \n"
    " ___/ / /  / / ___ |/ _, _/ / /     ___/ // / / ___ |/ _, _/ / / / /_/ / ____/   ___/ /  / /___/ // / / /___/ /  / /  \n"
    "/____/_/  /_/_/  |_/_/ |_| /_/     /____//_/ /_/  |_/_/ |_| /_/  \\____/_/       /____/  /_//____//_/ /_____/_/  /_/   \n"
    "\n"
    "----------------------------------------------------------------------------------------------------------------------\n"));
  dbf("PROGRAM INFO\n\tBy %s\n\tVersion %s\n\tLast update on %s\n", MAKERS, VERSION, DATE);
  dbf("----------------------------------------------------------------------------------------------------------------------\n");
}

void deviceInfo() {
  dbf("DEVICE INFO\n\tMAC: %s\n\tID: %d\n\tName: %s\n\tIP: %s\n\tNumber of Sockets: %d\n", _cfg.deviceMac, _cfg.deviceId, _cfg.deviceName, _cfg.deviceIp, _cfg.deviceNumSockets);
  serialHelp();
}

void serialHelp() {
  dbf("----------------------------------------------------------------------------------------------------------------------\n");
  dbf("HELP\n\tType: \"-c\" or \"--config\" to configure the device\n\tType: \"-d\" or \"--debug\" to get debug messages\n\tType: \"-h\" or \"--help\" to get this help message\n");
  dbf("----------------------------------------------------------------------------------------------------------------------\n");
}

void configMenu() {
  dbf("----------------------------------------------------------------------------------------------------------------------\n");
  dbf("CONFIG MENU\n");
  dbf("\t\"--set-mac\"                           // Automatically sets MAC, sometimes not read correctly\n");                     
  dbf("\t\"--set-id <DEVICE ID>\"\n");
  dbf("\t\"--set-name <DEVICE NAME>\"            // Max 16 characters\n");
  dbf("\t\"--set-ip <DEVICE IP>\"                // Use format xxx.xxx.xxx.xxx\n");
  dbf("\t\"--set-numsok <NUMBER OF SOCKETS>\"\n");
  dbf("\t\"--set-contype <CONNECTION TYPE>\"     // WIFI, ETH or AUTO are valid values\n");
  dbf("\t\"--set-wifinw <NETWORK NAME>\"\n");
  dbf("\t\"--set-wifipw <NETWORK PASSWORD>\"\n");
  dbf("\t\"--set-mqttsvr <MQTT SERVER\"\n");
  dbf("\t\"--set-mqtttpc <MQTT TOPIC>\"\n");
  dbf("\t\"--set-mqttprt <MQTT PORT>\"           // Normally set to 1883\n");
  dbf("\n");
  dbf("\t\"--get-dinfo\"                         // Gets all values stored in the EEPROM\n");
  dbf("\n");
  dbf("\t\"--del-dinfo\"                         // Deletes all values stored in the EEPROM\n");
  dbf("\n");
  dbf("\t\"--cmd-menu\"                          // Opens this config command menu\n");
  dbf("----------------------------------------------------------------------------------------------------------------------\n");
}

void configCommand(String command) {
  if (command.indexOf("--set-mac") != -1) {
    getMac();
    dbf("----------------------------------------------------------------------------------------------------------------------\n");
    dbf("Set MAC to \"%s\"\n", _cfg.deviceMac);
    dbf("----------------------------------------------------------------------------------------------------------------------\n");
  } else if (command.indexOf("--set-id") != -1) {
    int startPos = command.indexOf("--set-id") + 8; 
    String idStr = command.substring(startPos);
    idStr.trim();
    int newId = idStr.toInt();
    _cfg.deviceId = newId;
    dbf("----------------------------------------------------------------------------------------------------------------------\n");
    dbf("Set ID to \"%d\"\n", _cfg.deviceId);
    dbf("----------------------------------------------------------------------------------------------------------------------\n");
    writeEEPROM(); 
   } else if (command.indexOf("--set-name") != -1) {
    int startPos = command.indexOf("--set-name") + 10; 
    String nameStr = command.substring(startPos);
    nameStr.trim();
    strcpy(_cfg.deviceName, nameStr.c_str());
    dbf("----------------------------------------------------------------------------------------------------------------------\n");
    dbf("Set device name to \"%s\"\n", _cfg.deviceName);
    dbf("----------------------------------------------------------------------------------------------------------------------\n");
    writeEEPROM(); 
   } else if (command.indexOf("--set-ip") != -1) {
    int startPos = command.indexOf("--set-ip") + 8; 
    String ipStr = command.substring(startPos);
    ipStr.trim();
    strcpy(_cfg.deviceIp, ipStr.c_str());
    dbf("----------------------------------------------------------------------------------------------------------------------\n");
    dbf("Set IP to \"%s\"\n", _cfg.deviceIp);
    dbf("----------------------------------------------------------------------------------------------------------------------\n");
    writeEEPROM(); 
  } else if (command.indexOf("--set-numsok") != -1) {
    int startPos = command.indexOf("--set-numsok") + 12; 
    String numsokStr = command.substring(startPos);
    numsokStr.trim();
    int newNumsok = numsokStr.toInt();
    _cfg.deviceNumSockets = newNumsok;
    dbf("----------------------------------------------------------------------------------------------------------------------\n");
    dbf("Set number of sockets to \"%d\"\n", _cfg.deviceNumSockets);
    dbf("----------------------------------------------------------------------------------------------------------------------\n");
    writeEEPROM(); 
  } else if (command.indexOf("--set-contype") != -1) {
    int startPos = command.indexOf("--set-contype") + 13; 
    String contypeStr = command.substring(startPos);
    contypeStr.trim();
    strcpy(_cfg.deviceConType, contypeStr.c_str());      
    dbf("----------------------------------------------------------------------------------------------------------------------\n");
    dbf("Set connection type to \"%s\"\n", _cfg.deviceConType);
    dbf("----------------------------------------------------------------------------------------------------------------------\n");
    writeEEPROM(); 
    _connected = false;
  } else if (command.indexOf("--set-wifinw") != -1) {
    int startPos = command.indexOf("--set-wifinw") + 12; 
    String wifinwStr = command.substring(startPos);
    wifinwStr.trim();
    strcpy(_cfg.wifiSsid, wifinwStr.c_str());      
    dbf("----------------------------------------------------------------------------------------------------------------------\n");
    dbf("Set wifi network to \"%s\"\n", _cfg.wifiSsid);
    dbf("----------------------------------------------------------------------------------------------------------------------\n");
    writeEEPROM(); 
    _connected = false;
  }  else if (command.indexOf("--set-wifipw") != -1) {
    int startPos = command.indexOf("--set-wifipw") + 12; 
    String wifipwStr = command.substring(startPos);
    wifipwStr.trim();
    strcpy(_cfg.wifiPassword, wifipwStr.c_str());      
    dbf("----------------------------------------------------------------------------------------------------------------------\n");
    dbf("Set wifi password to \"%s\"\n", _cfg.wifiPassword);
    dbf("----------------------------------------------------------------------------------------------------------------------\n");
    writeEEPROM(); 
    _connected = false;
  } else if (command.indexOf("--get-dinfo") != -1) {
    if (readEEPROM()) { 
      dbf("----------------------------------------------------------------------------------------------------------------------\n");
      dbf("STORED DEVICE INFO:\n");
      dbf("\tMAC: %s\n", _cfg.deviceMac);
      dbf("\tID: %d\n", _cfg.deviceId);
      dbf("\tName: %s\n", _cfg.deviceName);
      dbf("\tIP: %s\n", _cfg.deviceIp);
      dbf("\tNumber of Sockets: %d\n", _cfg.deviceNumSockets);
      dbf("\tConnection type: %s\n", _cfg.deviceConType);
      dbf("\tWiFi SSID: %s\n", _cfg.wifiSsid);
      dbf("\tWiFi Password: %s\n", _cfg.wifiPassword);
      dbf("\tMQTT Server: %s\n", _cfg.mqttServer);
      dbf("\tMQTT Topic: %s\n", _cfg.mqttTopic);
      dbf("----------------------------------------------------------------------------------------------------------------------\n");
    } else {
      dbf("----------------------------------------------------------------------------------------------------------------------\n");
      dbf("Well this is akward, the EEPROM is empty.\n");
      dbf("----------------------------------------------------------------------------------------------------------------------\n");
    }
  } else if (command.indexOf("--del-dinfo") != -1) {
    clearEEPROM();
    dbf("----------------------------------------------------------------------------------------------------------------------\n");
    dbf("EEPROM has been cleared!\n");
    dbf("----------------------------------------------------------------------------------------------------------------------\n");
  } else if (command.indexOf("--cfg-menu") != -1) {
    configMenu();
  }
}

void watchSerial() {
  String serialString = "";
  if (Serial.available() > 0) {
    while (Serial.available() > 0) {
      char incomingChar = Serial.read();
      serialString += incomingChar;
    }
    serialString.trim();
    dbf("Recieved input: \"%s\"\n", serialString);
    if (serialString == "-h" || serialString == "--help") {
      serialHelp();
    } else if (serialString == "-d" || serialString == "--debug") {
      debugState = !debugState;
      if (debugState == false) {
        dbf("----------------------------------------------------------------------------------------------------------------------\n");
        dbf("Turning off debug messages...\n");
        dbf("----------------------------------------------------------------------------------------------------------------------\n");
      } else {
        dbf("----------------------------------------------------------------------------------------------------------------------\n");
        dbf("Turning on debug messages...\n");
        dbf("----------------------------------------------------------------------------------------------------------------------\n");
      }
    } else if (serialString == "-c" || serialString == "--config") {
      configState = !configState;
      if (configState == false) {
        dbf("----------------------------------------------------------------------------------------------------------------------\n");
        dbf("Leaving config mode...\n");
        dbf("----------------------------------------------------------------------------------------------------------------------\n");
      } else {
        configMenu();
      }
    } else if (configState) {
      configCommand(serialString);
    }
  }
}