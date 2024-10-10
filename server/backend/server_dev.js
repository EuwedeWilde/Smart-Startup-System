const mqtt = require("mqtt");
const mongoose = require('mongoose');
const express = require('express');
const path = require('path');
const WebSocket = require('ws');
const http = require('http');

const PORT = process.env.PORT || 3000;
const MQTT_BROKER = "mqtt://inmat.nl";
const MONGODB_URI = 'mongodb://127.0.0.1:27017/local';
const SERVER_NAME = "server";

const MAIN_TOPIC = "S3";

const PING_INTERVAL = 10000;
const INACTIVE_THRESHOLD = 15000;

const app = express();
const server = http.createServer(app);
const wss = new WebSocket.Server({ server });

const client = mqtt.connect(MQTT_BROKER);

app.use(express.static(path.join(__dirname, '../frontend/dist')));
app.get('*', (res) => {
  res.sendFile(path.join(__dirname, '../frontend/dist', 'index.html'));
});

server.listen(PORT, () => console.log(`Server is running on port ${PORT}`));

async function disconnectDevicesDb() {
  try {
    const result = await LogDeviceInfo.updateMany(
      {},
      { $set: { dConnected: false } }
    );
    console.log(`${result.modifiedCount} devices updated to disconnected status.`);
  } catch (error) {
    console.error('Error updating device connection status:', error);
  }
}

mongoose.connect(MONGODB_URI)
  .then(async () => {
    console.log('Connected to MongoDB');
    await disconnectDevicesDb();
  })
  .catch((err) => console.log(err));

const logSchemaMQTT = new mongoose.Schema({
  timestamp: { type: Date, default: Date.now },
  topic: String,
  message: mongoose.Schema.Types.Mixed,
});

const logSchemaInfo = new mongoose.Schema({
  timestamp: { type: Date, default: Date.now },
  dMac: String,
  dId: Number,
  dName: String,
  dIp: String,
  dGroup: String,
  dNumSockets: Number,
  dSockets: [{
    sId: Number,
    sName: String,
    sState: Boolean,
    sGroup: String,
    sInUse: Boolean,
    sLastInUse: Date,
    sLastUpdate: Date
  }],
  dConnected: Boolean,
  dLastConnection: Date,
  dLastUpdate: Date
});

const LogMQTT = mongoose.model('LogMQTT', logSchemaMQTT, 'mqtt_log');
const LogDeviceInfo = mongoose.model('LogDeviceInfo', logSchemaInfo, 'device_info');

setInterval(periodicTasks, PING_INTERVAL);

client.on("connect", onMQTTConnect);
client.on("message", onMQTTMessage);

wss.on('connection', onWebSocketConnection);

function periodicTasks() {
  getPingDevices();
  periodicInfoRequest(); // Add this line
}

function periodicInfoRequest() {
  const inactiveThreshold = Date.now() - (5 * 60 * 1000); // 5 minutes ago
  LogDeviceInfo.find({
    dConnected: true,
    $or: [
      { dLastUpdate: { $lt: new Date(inactiveThreshold) } },
      { dLastUpdate: { $exists: false } }
    ]
  }).then(devices => {
    devices.forEach(device => {
      console.log(`Requesting update for device ${device.dMac} (last update: ${device.dLastUpdate})`);
      getInfoDevices(device.dMac);
    });
  });
}

function getPingDevices() {
  client.publish(MAIN_TOPIC, JSON.stringify({ sender: SERVER_NAME, meth: "get", ping: "ping" }));
}

function handlePingDevices(parsedMessage) {
  const currentTime = Date.now();
  const deviceMac = parsedMessage.sender;
  LogDeviceInfo.findOne({ dMac: deviceMac }).then(device => {
    if (device) {
      const wasDisconnected = !device.dConnected;
      device.dLastConnection = new Date(currentTime);
      device.dConnected = true;
      device.save()
        .then(() => {
          console.log(`Device ${deviceMac} connection updated`);
          if (wasDisconnected) {
            console.log(`Device ${deviceMac} reconnected, requesting info`);
            getInfoDevices(deviceMac);
          }
        })
        .catch(err => console.error(`Error updating device ${deviceMac} connection status:`, err));
    } else {
      console.log(`New device ${deviceMac} detected, requesting info`);
      getInfoDevices(deviceMac);
    }
  });
}

function getInfoDevices(deviceMac) {
  client.publish("S3/"+deviceMac, JSON.stringify({ sender: SERVER_NAME, meth: "get", info: "?"}));
}

function handleInfoDevices(parsedMessage) {
  console.log("Received info from device:", parsedMessage);
  const deviceMac = parsedMessage.info.dMac;
  const deviceId = parsedMessage.info.dId;
  const deviceName = parsedMessage.info.dName;
  const deviceIp = parsedMessage.info.dIp;
  const deviceGroup = parsedMessage.info.dGroup;
  const deviceNumSockets = parsedMessage.info.dNumSockets;
  const deviceSockets = parsedMessage.info.dSockets;
  const currentTime = Date.now();
  
  LogDeviceInfo.findOneAndUpdate(
    { dMac: deviceMac },
    {
      $set: {
        dId: deviceId,
        dName: deviceName,
        dIp: deviceIp,
        dGroup: deviceGroup,
        dNumSockets: deviceNumSockets,
        dSockets: deviceSockets,
        dLastUpdate: new Date(currentTime),
        dConnected: true
      }
    },
    { new: true, upsert: true }
  )
    .then(updatedDevice => {
      console.log(`Device ${deviceMac} info updated`);
      updateDeviceList(); // Update the device list after saving
    })
    .catch(err => console.error(`Error updating device ${deviceMac} info:`, err));
}

function sendConfigDevices() {
  const config = []; // Structure: {dMac: string, dSocket: {sId: number, sName: string, sGroup: string, sInUse: boolean, sLastInUse: date, sLastUpdate: date}} 
  client.publish(MAIN_TOPIC, JSON.stringify({ sender: SERVER_NAME, meth: "put", config: config}));
  console.log("Requested info from all devices");
}

function handleConfigDevices(parsedMessage) {
  console.log("Received config from device:", parsedMessage);
}

function sendControlDevices() {
  const control = []; // Structure: {socket: {sId: number, state: boolean}} 
  client.publish(MAIN_TOPIC, JSON.stringify({ sender: SERVER_NAME, meth: "put", control: control}));
  console.log("Requested info from all devices");
}

function handleControlDevices(parsedMessage) {
  console.log("Received control from device:", parsedMessage);
}

function onMQTTConnect() {
  console.log("Connected to MQTT broker");
  client.subscribe([MAIN_TOPIC+"/#"], (err) => {
    if (err) {
      console.error("Failed to subscribe:", err);
    } else {
      console.log("Successfully subscribed to topics");
      getPingDevices();
    }
  });
}

function onMQTTMessage(topic, message) {
  try {
    const parsedMessage = JSON.parse(message.toString());
    if (parsedMessage.sender !== SERVER_NAME) {
      if ('ping' in parsedMessage) {
        handlePingDevices(parsedMessage);
      } else if ('info' in parsedMessage) {
        handleInfoDevices(parsedMessage);
      } else if ('config' in parsedMessage) {
        handleConfigDevices(parsedMessage);
      } else if ('control' in parsedMessage) {
        handleControlDevices(parsedMessage);
      }
    }
  } catch (error) {
    console.error("Failed to parse message as JSON:", error);
  }
}

function updateExistingDevice(device) {
  existingDevice.save()
    .then(updatedDevice => {
      console.log(`Device ${deviceId} (MAC: ${existingDevice.dMac}) updated`);
      updateDeviceList();
    })
    .catch(err => console.error(`Error updating device ${deviceId} (MAC: ${existingDevice.dMac}):`, err));
}

function createNewDevice(deviceMac, deviceId, deviceName, deviceIp, deviceGroup, deviceNumSockets, deviceSockets, currentTime) {
  const newDevice = new LogDeviceInfo({
    dMac: deviceMac,
    dId: deviceId, // Use the provided deviceId
    dName: deviceName,
    dIp: deviceIp,
    dGroup: deviceGroup,
    dNumSockets: deviceNumSockets,
    dSockets: deviceSockets,
    dConnected: true,
    dLastConnection: new Date(currentTime),
    dLastUpdate: new Date(currentTime)
  });

  newDevice.save()
    .then(savedDevice => {
      console.log(`New device ${deviceId} (MAC: ${deviceMac}) created`);
      updateDeviceList();
    })
    .catch(err => console.error(`Error creating new device ${deviceId} (MAC: ${deviceMac}):`, err));
}

function updateDeviceList() {
  LogDeviceInfo.find({})
    .then(devices => {
      const deviceList = devices.map(device => device.toObject());
      wss.clients.forEach(client => {
        if (client.readyState === WebSocket.OPEN) {
          client.send(JSON.stringify({ type: 'deviceList', devices: deviceList }));
        }
      });
      // We don't need to request info here anymore
    })
    .catch(err => console.error('Error fetching device list:', err));
}

function sendInitialDeviceList(ws) {
  LogDeviceInfo.find({})
    .then(devices => {
      const deviceList = devices.map(device => device.toObject());
      ws.send(JSON.stringify({ type: 'deviceList', devices: deviceList }));
    })
    .catch(err => console.error('Error fetching initial device list:', err));
}

function onWebSocketConnection(ws) {
  console.log('New WebSocket connection established');

  ws.on('message', (message) => handleWebSocketMessage(ws, message));
  ws.on('error', (error) => console.error('WebSocket error:', error));
  ws.on('close', () => console.log('WebSocket connection closed'));

  sendInitialDeviceList(ws);
}

function handleWebSocketMessage(ws, message) {
  try {
    const data = JSON.parse(message);
    console.log('Received message:', data);

    switch (data.type) {
      case 'updateDevice':
        updateDevice(data.did, data.newName, data.newIp, data.socketUpdate);
        break;
      case 'selectDevice':
        console.log('Device selected:', data);
        break;
      case 'requestDeviceList':
        updateDeviceList();
        break;
      case 'controlSocket':
        console.log('Control socket:', data);
        controlSocket(data.did, data.mac, data.socketName, data.state);
        break;
      default:
        console.log('Unknown message type:', data.type);
    }
  } catch (error) {
    console.error('Error processing WebSocket message:', error);
  }
}

function logMessage(topic, message) {
  const log = new LogMQTT({
    topic: topic,
    message: message
  });
  
  log.save()
    .then(() => console.log(`Logged message for topic: ${topic}`))
    .catch(err => console.error('Error logging message:', err));
}

// Add this function to periodically clean up inactive devices
function cleanupInactiveDevices() {
  const currentTime = Date.now();
  const inactiveThreshold = 5 * 60 * 1000; // 5 minutes

  activeDevices.forEach((deviceMac, index) => {
    LogDeviceInfo.findOne({ dMac: deviceMac })
      .then(device => {
        if (!device || (currentTime - device.dLastConnection.getTime() > inactiveThreshold)) {
          console.log(`Removing inactive device: ${deviceMac}`);
          activeDevices.splice(index, 1);
        }
      })
      .catch(err => console.error(`Error checking device activity: ${deviceMac}`, err));
  });
}

// Call this function periodically, e.g., every 5 minutes
setInterval(cleanupInactiveDevices, 5 * 60 * 1000);