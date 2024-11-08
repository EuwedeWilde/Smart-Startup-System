<section class="docs">
  <h2 class="docs__title">Docs</h2>
  <p class="docs__content-footnote">
    Documentation is still under construction. Version 0.1.0. 2024-11-08. By <a href="https://github.com/EuwedeWilde" class="docs__link">Euwe de Wilde</a>
  </p>
  <h3 class="docs__subtitle">Introduction</h3>
  <p class="docs__content">
    If you are seeing this, great! That means that you managed to get the frontend up and running. This is the documentation of the Smart Startup System or S3 (pronounced <em>"S-cubed"</em>) for short. This is an opensource socket switching system, the problem with most smart switching systems on the market is that they do not have an ethernet port and are not very configurable. This system changes that and is an upgrade in all regards (I am a bit biased). This documentation hopes to shed light on the making of the devices and how they work. 
  </p>
  <h3 class="docs__subtitle">Contents</h3>
  <ul class="docs__content-list">
    <li><a href="#hardware" class="docs__link">Hardware</a></li>
    <li><a href="#software" class="docs__link">Software</a></li>
    <ul class="docs__content-list">
      <li><a href="#architecture" class="docs__link">Architecture</a></li>
      <li><a href="#server" class="docs__link">Server</a></li>
      <li><a href="#device" class="docs__link">Device</a></li>
      <ul class="docs__content-list">
        <li><a href="#eeprom" class="docs__link">EEPROM</a></li>
        <li><a href="#mqtt" class="docs__link">MQTT</a></li>
      </ul>
      <li><a href="#frontend" class="docs__link">Frontend</a></li>
      <li><a href="#database" class="docs__link">Database</a></li>
    </ul>
  </ul>
  <h2 class="docs__title" id="software">Software</h2>
  <h3 class="docs__subtitle" id="architecture">Architecture</h3>
  <p class="docs__content">
    All communication between different systems (e.g. the device, database, frontend) all go via the server. You can see the server as the manager, telling everyone what to do and making sure all communication goes seamlessly. This is done with different commutation protocols, depending on the different systems. 
  </p>

  <img src="/src/assets/Architecture.png" alt="Architecture" class="docs__image">
  <p class="docs__image-caption">System Architecture of S3</p>
  <p class="docs__content">The system architecture consists out of four systems:</p>
  <ol class="docs__content-list">
    <li><a href="#server" class="docs__link">Server</a></li>
    <li><a href="#device" class="docs__link">Device</a></li>
    <li><a href="#frontend" class="docs__link">Frontend</a></li>
    <li><a href="#database" class="docs__link">Database</a></li>
  </ol>
  <p class="docs__content">
    We will now go through each system in detail.
  </p>
  <h3 class="docs__subtitle" id="server">Server</h3>
  <p class="docs__content">
    As mentioned earlier, the server is the manager of all systems, making sure all goes well. I communicates with the connected devices, updates the database when changes are made and makes sure the frontend reacts to changes. 
  </p>
  <p class="docs__content">
    All connections are important, but the one with the device is the most vital. This is done via the MQTT communication protocol, where devices can publish (pub) and subscribe (sub) to topics. All devices in on that MQTT network listing in this case to the topic called "S3", but you can name it what ever you want, just change it for all devices. The server pubs on this topic every 5 seconds  a ping, to see which devices are still online, if a device reacts with a pong, the server knows it is online, but if it does not receive anything (so no pong is send) the server knows that device is offline. 
  </p>
  <code class="docs__content-code">
    {@html `S3 {"sender":"server","meth":"get","ping":"ping"}
S3/E05A1B2307DC {"sender":"E05A1B2307DC","meth":"inf","ping":"pong"}`}
  </code>
  <p class="docs__content-footnote">Example of a ping and pong</p>
  <p class="docs__content">
    The server sends this info to the frontend, but before that it compares the info received from the device to the info that is stored in the database, depending on the type of data, it updates the info on the database or device. For example, when the device sends it info (e.g. name, id, ip), the database is then set to those values that are also on the device. But the state of the sockets for example is determined by the database and then send to the device when it comes online. This makes sure both devices are synced correctly. Then the frontend is updated accordingly, which is done via a WebSocket connection. 
  </p>
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
  <h4 class="docs__subsubtitle" id="frontend">Connectivity</h4>

</section>

<style lang="scss">
  @use '../styles/variables' as *;

  .docs {
    font-family: $main-font;
    width: 80%;
    margin: 0 auto;
    color: $secondary-color;
    &__title {
      font-size: $font-large;
      margin: 0.5rem 0;
    }
    &__subtitle {
      font-size: $font-medium-large;
      margin: 0.5rem 0;
    }
    &__subsubtitle {
      font-size: $font-medium;
      margin: 0.5rem 0;
    }
    &__link {
      color: $secondary-color;
      text-decoration: underline;
      &:hover {
        color: $highlight-color;
      }
    }
    &__content {
      margin: 0.5rem 0;
      text-align: justify;
      text-justify: inter-word;
      font-size: $font-medium;
      &-list {
        margin-left: 2rem;
      }
      &-code {
        background-color: $secondary-color;
        padding: 0.5rem;
        margin: 0.5rem 0;
        color: $primary-color;
        font-size: $font-small-medium;
        font-family: $mono-font;
        white-space: pre;
        display: block;
      }
      &-footnote {
        font-size: $font-small-medium;
        font-style: italic;
        margin: 0.5rem auto;
        text-align: left;
      }
    }
    &__image {
      width: 40%;
      margin: 1rem auto;
      display: block;
      &-caption {
        font-size: $font-small-medium;
        font-style: italic;
        margin: 0.5rem auto;
        text-align: center;
      }
    }
  }
</style>
