<section>
  <h3 class="docs__subtitle" id="device">Device</h3>
  <p class="docs__content">
    Previously I explained how to make the devices, but to make them functional, you need to add some software. The device makes use off quite some libraries, here a quick overview:  
  </p>
  <ul class="docs__content-list">
    <li><em>config.h</em>  - library for the configuration of the device</li>
    <li><em>EEPROM.h</em> - library for reading and writing to the EEPROM of the device</li>
    <li><em>ETH.h</em> - library for adding Ethernet capabilites with the WIZ W5500 Ethernet module</li>
    <li><em>WiFi.h</em> - library for adding  WiFi capabilities to the ESP32</li>
    <li><em>ArduinoMqttClient.h</em> * -  library for communicating with the MQTT protocol </li>
    <li><em>ArduinoJson.h</em> ** - library for using JSON with on a arduino</li>
    <li><em>ESPmDNS.h</em> - library for adding DNS capabilities to the ESP32</li>
    <li><em>WiFiUdp.h</em> - library for adding UDP capabilities to the ESP32</li>
    <li><em>ArduinoOTA.h</em> - library for adding OTA capabilities to the ESP32</li>
  </ul>
  <p class="docs__content-footnote">* Modified for this project, download from github <br> ** Used arduinoJson 5.x</p>
  <h4 class="docs__subsubtitle" id="eeprom">EEPROM</h4>
  <p class="docs__content">
    EEPROM control the storage of memory, even when the device is powered down. The following things are currently stored there:
  </p>
  <ul class="docs__content-list">
    <li>Device Mac Address</li>
    <li>Device ID</li>
    <li>Device IP Adress</li>
    <li>Device Name</li>
    <li>Device Number of Sockets</li>
    <li>WiFi SSID</li>
    <li>WiFi Password</li>
    <li>MQTT Server Address</li>
    <li>MQTT Server Port</li>
    <li>MQTT Topic</li>
  </ul>
  <p class="docs__content">
    You want to to store things in the EEPROM that you want to change without the need of reflashing the whole device. The WiFi network is a good example, because sometimes it could be that you want to set up your system somewhere else, with some code you can easily change the network without reflashing the ESP32. the Mac is the only exception, since that address never changes, but it is device unique. The EEPROM just seemed to be a nice place to save it.
  </p>
  <p class="docs__content">
    At the beginning of the sketch the EEPROM is setup with:
  </p>
  <code class="docs__content-code">
    {@html `void setupEEPROM() {
  EEPROM.begin(EEPROM_SIZE);
  if (!readEEPROM()) {
    dbf("No Config Defined\\n");
    defineConfig(); // Values are assigned to the config
    writeEEPROM();
  } 
}`}
  </code>
  <p class="docs__content-footnote">Setup function of EEPROM</p>
  <p class="docs__content">
    The EEPROM is initialized. When nothing is in the EEPROM, it writes some arbitrary values from the config struct stored int the flash memory to the EEPROM. Once that is initialized, you can read and write to the EEPROM with the following functions:
  </p>  
  <code class="docs__content-code">
    {@html `void writeEEPROM() {
  dbf("Writing to EEPROM...\\n");
  EEPROM.put(EEPROM_CONFIG, _cfg);
  EEPROM.write(EEPROM_CFG_DEFINED, IS_DEFINED);
  EEPROM.commit();
  dbf("Written to EEPROM!\\n");
}

bool readEEPROM() {
  if (EEPROM.read(EEPROM_CFG_DEFINED) == IS_DEFINED) {
    dbf("Loaded CONFIG from EEPROM\\n");
    EEPROM.get(EEPROM_CONFIG, _cfg);
    return true;
  } else {
    dbf("Config EEPROM Empty\\n");
    getMac(); // Get the mac address of the device
    return false;
  }
}`}
  </code>
  <p class="docs__content-footnote">Read and write functions of EEPROM</p>
  <h4 class="docs__subsubtitle" id="mqtt">MQTT</h4>
  <p class="docs__content">
    The MQTT protocol is used to communicate with the server. All devices are subbed to the "S3" topic and pub on the "S3/XXXXXXXXXXXX" with XXXXXXXXXXXX being replaced by the mac address of the device that pubs. When the device is powered on and initialized, it will always first pubs its info. This is to let the database sync up with the device info. After that it will send pong after each ping message.   
  </p>
  <p class="docs__content">
    The device it self also has a button for every socket, this is to manually turn on and off the sockets. Every time one of these is pressed the socket state switches and also a MQTT request is send to update frontend and database.   
  </p>
  <code class="docs__content-code">
    {@html `S3/E05A1B2307DC {"sender":"E05A1B2307DC","meth":"inf","info":{"dId":1,"dName":"device","dIp":"192.0.0.200","dNumSockets":2}}
S3/E05A1B2307DC {"sender":"E05A1B2307DC","meth":"inf","ping":"pong"}
S3/E05A1B2307DC {"sender":"E05A1B2307DC","meth":"put","control":[{"sId":1,"sState":true},{"sId":2,"sState":true}]}`}
  </code>
  <p class="docs__content-footnote">Example of a device info, pong and control</p>
  <p class="docs__content">
    The device sometimes also just listens to the commands of the server and updates its info accordingly when asked. This is when config or control is changed in the frontend. It does not pub a message on those two occurrences.
  </p>
  <h4 class="docs__subsubtitle" id="connectivity">Connectivity</h4>
  <p class="docs__content">
    As mentioned earlier, the device offers two methods of connecting to it, using WiFi or using Ethernet. As of version 1.0.0, this needs to be configured in the flash, however in future iterations I hope to implement a auto detect, when Ethernet is available, it will automatically switch to ethernet. This configuring of the network type is currently done by flipping a bit, 1 is for WiFi and 0 for Ethernet. This is done in the config.h file.
  </p>
  <code class="docs__content-code">
    {@html `// --- CONNECTIVITY ---
#define ETH_OR_WIFI 1 // 0 is ETH, 1 is WIFI`}
  </code>
  <p class="docs__content-footnote">Network type configuration</p>
  <p class="docs__content">
    This method of configuring on flash is also not ideal for setting up the WiFi network, because currently its credentials are also set there, but I can imagine that when you move the device to a different network, this is not ideal. Maybe a configuration via Bluetooth, Serial or direct ethernet connection (PC to device) could be a nice upgrade. 
  </p>
</section>