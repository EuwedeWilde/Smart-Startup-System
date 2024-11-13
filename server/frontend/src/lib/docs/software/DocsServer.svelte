<section>
  <h3 class="docs__subtitle" id="server">Server</h3>
  <p class="docs__content">
    To test the server go to the backend folder and run the following command:
  </p>
  <code class="docs__content-code">
    {@html `// --- Structure ---
Smart-Startup-System
└──server
   ├──frontend
   ├──dist
   └──backend
       └──... // all server files

// --- Commands ---
npm run dev // to test the frontend
npm run start // to build the frontend`}
  </code>
  <p class="docs__content-footnote">Structure and commands</p>
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
  <p class="docs__content">
    The server has not much going on. It just sends this info in one form or the other to one of the other systems in the architecture. Not all systems need the same info, the device does not care about the socket name for example. This are all the possible variables it can send:
  </p>
  <code class="docs__content-code">
    {@html `timestamp: date
dMac: String
dId: Number
dName: String
dIp: String
dGroup: String
dNumSockets: Number
dSockets: [{
  sId: Number
  sName: String
  sState: Boolean
  sGroup: String
  sInUse: Boolean
  sLastInUse: Date
  sLastUpdate: Date
}]
dConnected: Boolean
dLastConnection: Date
dLastUpdate: Date`}
  </code>
  <p class="docs__content-footnote">Possible variables, with dSockets[] being an object</p>
</section>