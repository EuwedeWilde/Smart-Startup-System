const mqtt = require("mqtt");
const mongoose = require('mongoose');
const express = require('express');
const path = require('path');
const WebSocket = require('ws');
const http = require('http');

const app = express();
const server = http.createServer(app);
const wss = new WebSocket.Server({ server });

const PORT = process.env.PORT || 3000;
const MQTT_BROKER = "mqtt://inmat.nl";
const MONGODB_URI = 'mongodb://127.0.0.1:27017/local';
const SERVER_ID = 0;
const MAIN_TOPIC = "S3";
const INFO_TOPIC = `${MAIN_TOPIC}/info`;
const CONFIG_TOPIC = `${MAIN_TOPIC}/config`;
const PING_TOPIC = `${MAIN_TOPIC}/ping`;
const CONTROL_TOPIC = `${MAIN_TOPIC}/control`;
const INACTIVE_THRESHOLD = 15000;

const client = mqtt.connect(MQTT_BROKER);

app.use(express.static(path.join(__dirname, 'frontend/dist')));
app.get('*', (req, res) => {
  res.sendFile(path.join(__dirname, 'frontend/dist', 'index.html'));
});

server.listen(PORT, () => console.log(`Server is running on port ${PORT}`));

mongoose.connect(MONGODB_URI)
  .then(async () => {
    console.log('Connected to MongoDB');
    try {
      await mongoose.connection.dropCollection('device_info');
      console.log('Dropped device_info collection');
    } catch (error) {
      if (error.code !== 26) {
        console.error('Error dropping device_info collection:', error);
      }
    }
  })
  .catch((err) => console.log(err));

const logSchemaMQTT = new mongoose.Schema({
  timestamp: { type: Date, default: Date.now },
  topic: String,
  message: mongoose.Schema.Types.Mixed,
});

const logSchemaInfo = new mongoose.Schema({
  timestamp: { type: Date, default: Date.now },
  did: Number,
  mac: String,
  dname: String,
  ip: String,
});

const LogMQTT = mongoose.model('LogMQTT', logSchemaMQTT, 'mqtt_log');
const LogDeviceInfo = mongoose.model('LogDeviceInfo', logSchemaInfo, 'device_info');

let didList = [];
let deviceInfo = new Map();
let lastPingTime = new Map();

setInterval(() => {
  rqstPingDevices();
  checkAndRemoveInactiveDevices();
  console.log("deviceInfo:", deviceInfo);
  console.log("didList:", didList);
}, 10000);

client.on("connect", () => {
  console.log("Connected to MQTT broker");
  client.subscribe([INFO_TOPIC, CONFIG_TOPIC, PING_TOPIC, CONTROL_TOPIC], (err) => {
    if (err) {
      console.error("Failed to subscribe:", err);
    } else {
      console.log("Successfully subscribed to topics");
      rqstPingDevices();
    }
  });
});

client.on("message", (topic, message) => {
  try {
    const parsedMessage = JSON.parse(message.toString());
    if (topic === PING_TOPIC) {
      handlePingTopic(parsedMessage);
    } else if (topic === INFO_TOPIC) {
      handleInfoTopic(parsedMessage);
    } else if (topic === CONFIG_TOPIC) {
      console.log("Received config message:", parsedMessage);
      handleConfigTopic(parsedMessage);  // Add this line
      logMessage(topic, parsedMessage);
    }
  } catch (error) {
    console.error("Failed to parse message as JSON:", error);
  }
});

function handlePingTopic(parsedMessage) {
  if (parsedMessage.did !== SERVER_ID) {
    const deviceId = parsedMessage.did;
    lastPingTime.set(deviceId, Date.now());
    
    if (!didList.includes(deviceId)) {
      didList.push(deviceId);
      updateDeviceList();
    }
    
    if (!deviceInfo.has(deviceId)) {
      deviceInfo.set(deviceId, { dId: deviceId, dName: parsedMessage.name });
      rqstInfoDevices(deviceId);
    } else {
      const existingDevice = deviceInfo.get(deviceId);
      if (existingDevice.dName !== parsedMessage.name) {
        existingDevice.dName = parsedMessage.name;
        deviceInfo.set(deviceId, existingDevice);
        updateDeviceList();
      }
    }
  }
  logMessage(PING_TOPIC, parsedMessage);
}

function checkAndRemoveInactiveDevices() {
  const currentTime = Date.now();

  for (let [deviceId, lastPing] of lastPingTime) {
    if (currentTime - lastPing > INACTIVE_THRESHOLD) {
      console.log(`Device ${deviceId} is inactive. Removing...`);
      didList = didList.filter(id => id !== deviceId);
      deviceInfo.delete(deviceId);
      lastPingTime.delete(deviceId);

      LogDeviceInfo.deleteOne({ did: deviceId })
        .then(() => console.log(`Device ${deviceId} removed from database`))
        .catch(err => console.error(`Error removing device ${deviceId} from database:`, err));
    }
  }
  updateDeviceList();
}

function handleInfoTopic(parsedMessage) { 
  if (parsedMessage.did !== SERVER_ID) {
    const logEntry = new LogDeviceInfo({
      did: parsedMessage.did,
      mac: parsedMessage.mac,
      dname: parsedMessage.dname,
      ip: parsedMessage.ip,
      sockets: parsedMessage.sockets
    });

    logEntry.save()
      .then(() => console.log("Device info logged successfully"))
      .catch(err => console.error("Failed to log device info:", err));
    
    deviceInfo.set(parsedMessage.did, {
      dId: parsedMessage.did,
      dName: parsedMessage.dname,
      dMac: parsedMessage.mac,
      dIp: parsedMessage.ip,
      sockets: parsedMessage.sockets
    });
    updateDeviceList();
  }  
  logMessage(INFO_TOPIC, parsedMessage);
}

function updateDeviceList() {
  const deviceListArray = Array.from(deviceInfo.entries()).map(([id, device]) => ({
    did: id,
    dName: device.dName,
    dMac: device.dMac,
    dIp: device.dIp,
    sockets: device.sockets || []
  }));

  wss.clients.forEach((ws) => {
    if (ws.readyState === WebSocket.OPEN) {
      ws.send(JSON.stringify({ 
        type: 'updateDeviceList', 
        deviceInfo: deviceListArray
      }));
    }
  });
}

function logMessage(topic, message) {
  new LogMQTT({ topic, message }).save()
    .then(() => console.log("Log entry saved successfully"))
    .catch(err => console.error("Failed to save log entry:", err));
}

function rqstPingDevices() {
  client.publish(PING_TOPIC, JSON.stringify({ did: SERVER_ID, ping: "ALL" }));
  console.log("Pinged all devices");
}

function rqstInfoDevices(deviceId) {
  client.publish(INFO_TOPIC, JSON.stringify({ did: SERVER_ID, requestInfo: deviceId }));
  console.log(`Requested info from device ${deviceId}`);
}

wss.on('connection', (ws) => {
  console.log('New WebSocket connection established');

  ws.on('message', (message) => {
    try {
      const data = JSON.parse(message);
      console.log('Received message:', data);

      if (data.type === 'updateDevice') {
        updateDevice(data.did, data.newName, data.newIp, data.socketUpdate);
      } else if (data.type === 'selectDevice') {
        console.log('Device selected:', data);
        // Handle device selection if needed
      } else if (data.type === 'requestDeviceList') {
        // Send the current device list to the client
        updateDeviceList();
      } else if (data.type === 'controlSocket') {
        console.log('Control socket:', data);
        controlSocket(data.did, data.mac, data.socketName, data.state);
      } 
    } catch (error) {
      console.error('Error processing WebSocket message:', error);
    }
  });

  ws.on('error', (error) => {
    console.error('WebSocket error:', error);
  });

  ws.on('close', () => {
    console.log('WebSocket connection closed');
  });

  // Send initial device list to the client
  const deviceListArray = Array.from(deviceInfo.entries()).map(([id, device]) => {
    return [id, device];
  });
  ws.send(JSON.stringify({ 
    type: 'updateDeviceList', 
    deviceInfo: deviceListArray
  }));
});

function updateDevice(did, newName, newIp, socketUpdate) {
  console.log(`Updating device: did=${did}, newName=${newName}, newIp=${newIp}, socketUpdate=`, socketUpdate);
  LogDeviceInfo.findOneAndUpdate(
    { did: did },
    { $set: { dname: newName, ip: newIp } },
    { new: true, runValidators: true }
  )
    .then((updatedDevice) => {
      if (updatedDevice) {
        console.log(`Device ${did} updated:`, updatedDevice);
        
        const currentDevice = deviceInfo.get(did);
        const updatedDeviceInfo = {
          dId: updatedDevice.did,
          dMac: updatedDevice.mac,
          dName: updatedDevice.dname,
          dIp: updatedDevice.ip,
          sockets: currentDevice.sockets.map(socket => ({ ...socket }))
        };

        if (socketUpdate) {
          const socketIndex = updatedDeviceInfo.sockets.findIndex(s => s.name === socketUpdate.oldName);
          if (socketIndex !== -1) {
            updatedDeviceInfo.sockets[socketIndex] = {
              name: socketUpdate.newName,
              state: socketUpdate.state,
              group: socketUpdate.group
            };
          }
        }
        
        deviceInfo.set(did, updatedDeviceInfo);
        
        updateDeviceList();
        
        const configMessage = {
          sId: SERVER_ID,
          dConfig: {
            dId: updatedDeviceInfo.dId,
            dMac: updatedDeviceInfo.dMac,
            dName: updatedDeviceInfo.dName,
            dIp: updatedDeviceInfo.dIp,
            sockets: updatedDeviceInfo.sockets
          }
        };
        
        client.publish(CONFIG_TOPIC, JSON.stringify(configMessage));
        console.log("Sent config update via MQTT:", configMessage);
      } else {
        console.log(`Device ${did} not found`);
      }
    })
    .catch((err) => {
      console.error(`Error updating device ${did}:`, err);
      if (err.name === 'ValidationError') {
        console.error('Validation Error:', err.message);
      }
    });
}

function handleConfigTopic(parsedMessage) {
  if (parsedMessage.did && parsedMessage.status === "updated") {
    const deviceId = parsedMessage.did;
    const updatedInfo = {
      dId: deviceId,
      dMac: deviceInfo.get(deviceId)?.dMac || '',  // Preserve the MAC address
      dName: parsedMessage.name,
      dIp: parsedMessage.ip,
      sockets: parsedMessage.outlets.map(outlet => ({
        name: outlet.name,
        state: outlet.state,
        group: outlet.group
      }))
    };
    deviceInfo.set(deviceId, updatedInfo);
    updateDeviceList();
  }
}

function controlSocket(did, mac, socketName, state) {
  console.log(`Control socket: did=${did}, mac=${mac}, socketName=${socketName}, state=${state}`);
  const controlMessage = {
    sId: SERVER_ID,
    dControl: {
      did: did,
      mac: mac,
      socketName: socketName,
      state: state
    }
  };
  client.publish(CONTROL_TOPIC, JSON.stringify(controlMessage));
  console.log("Sent control update via MQTT:", controlMessage);
}