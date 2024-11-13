const mqtt = require("mqtt");
const mongoose = require('mongoose');
const express = require('express');
const path = require('path');
const WebSocket = require('ws');
const http = require('http');
const { info } = require("console");

const PORT = process.env.PORT || 1407;
const MQTT_BROKER = "mqtt://inmat.nl";
const MONGODB_URI = 'mongodb://127.0.0.1:27017/local';
const SERVER_NAME = "server";

const MAIN_TOPIC = "S3";

const PING_INTERVAL = 5000;
const INACTIVE_THRESHOLD = 10000;

const app = express();
const server = http.createServer(app);
const wss = new WebSocket.Server({ server });

const client = mqtt.connect(MQTT_BROKER);

app.use(
  express.static(path.join(__dirname, '../frontend/dist'))
);

app.get('*', (req, res) => {
  res.sendFile(path.join(__dirname, '../frontend/dist', 'index.html'));
});

server.listen(PORT, () => console.log(`Server is running on port ${PORT}`));

mongoose.connect(MONGODB_URI)
  .then(async () => {
    console.log('Connected to MongoDB');
  })
  .catch((err) => console.log(err));

const lsMqttMessages = new mongoose.Schema({
  timestamp: { type: Date, default: Date.now },
  topic: String,
  message: mongoose.Schema.Types.Mixed,
});

const lsDeviceInfo = new mongoose.Schema({
  timestamp: { type: Date, default: Date.now }, // from Device means the data can not be changed by the client, but to the device it means it can
  dMac: String, // EEPROM from Device 
  dId: Number, // EEPROM from and to Device
  dName: String, // EEPROM from and to Device
  dIp: String, // EEPROM from and to Device
  dGroup: String, 
  dNumSockets: Number, // EEPROM from Device
  dSockets: [{
    sId: Number, // from Device (1 - dNumSockets)
    sName: String,
    sState: Boolean, // from and to Device
    sGroup: String,
    sInUse: Boolean, // to Device
    sLastInUse: Date,
    sLastUpdate: Date
  }],
  dConnected: Boolean,
  dLastConnection: Date,
  dLastUpdate: Date
});

const dbMqttMessages = mongoose.model('dbMqttMessages', lsMqttMessages, 'mqtt_messages');
const dbDeviceInfo = mongoose.model('dbDeviceInfo', lsDeviceInfo, 'device_info');

setInterval(periodicTasks, PING_INTERVAL);

function periodicTasks() {
  pingToDevices();
  updateInactiveDevices();
}

async function updateInactiveDevices() {
  const currentTime = new Date();
  try {
    const devices = await dbDeviceInfo.find({ dConnected: true });
    let deviceUpdated = false;
    
    for (const device of devices) {
      if (currentTime - new Date(device.dLastConnection) > INACTIVE_THRESHOLD) {
        device.dConnected = false;
        await device.save();
        deviceUpdated = true;
      }
    }
    
    // Only broadcast if a device was actually updated
    if (deviceUpdated) {
      devicesToWebSocket(wss);
    }
  } catch (err) {
    console.error("Error finding connected devices:", err);
  }
}

wss.on('connection', onWebSocketConnection);

function onWebSocketConnection(ws) {
  console.log('New WebSocket connection established');

  ws.on('message', (message) => {messageFromWebSocket(ws, message)});  
  ws.on('error', (error) => console.error('WebSocket error:', error));
  ws.on('close', () => console.log('WebSocket connection closed'));

  devicesToWebSocket(ws);
}

function devicesToWebSocket(ws) {
  dbDeviceInfo.find({})
    .then(devices => {
      const deviceList = devices.map((device) => ({
        dMac: device.dMac,
        dId: device.dId,
        dName: device.dName,
        dIp: device.dIp,
        dNumSockets: device.dNumSockets,
        dSockets: device.dSockets,
        dConnected: device.dConnected,
        dLastConnection: device.dLastConnection,
        dLastUpdate: device.dLastUpdate
      }));

      // If ws is the WebSocket server (wss), broadcast to all clients
      if (ws instanceof WebSocket.Server) {
        ws.clients.forEach(client => {
          if (client.readyState === WebSocket.OPEN) {
            client.send(JSON.stringify({ devices: deviceList }));
          }
        });
      }
      // If ws is a single client connection, send only to that client
      else if (ws && ws.readyState === WebSocket.OPEN) {
        ws.send(JSON.stringify({ devices: deviceList }));
      }
    })
    .catch(err => {
      console.error('Error finding connected devices:', err);
    });
}

function messageFromWebSocket(ws, message) {
  try {
    const jsonMessage = JSON.parse(message);
    console.log('Parsed WebSocket message:', jsonMessage); 
    if ('config' in jsonMessage) {
      configFromWebSocket(jsonMessage.config.device, jsonMessage.config.config);
    } else if ('control' in jsonMessage) {
      controlFromWebSocket(jsonMessage.device, jsonMessage.control);
    }
  } catch (error) {
    console.error('Failed to parse message as JSON:', error);
  }
}

async function configFromWebSocket(device, config) {
  try {
    const deviceInfoOld = await infoFromDataBase(device);
    const deviceInfoNew = config;

    const deviceConfig = {
      dId: deviceInfoNew.dId,
      dName: deviceInfoNew.dName,
      dIp: deviceInfoNew.dIp,
    };

    if (
      deviceInfoOld.dId !== deviceInfoNew.dId ||
      deviceInfoOld.dName !== deviceInfoNew.dName ||
      deviceInfoOld.dIp !== deviceInfoNew.dIp
    ) {
      configToDevice(device, deviceConfig);
    }
    updateDeviceConfig(device, config);
  } catch (err) {
    console.error('Error updating device info:', err);
  }
}

async function controlFromWebSocket(device, control) {
  try {
    dbDeviceInfo.findOneAndUpdate(
      { dMac: device },
      { $set: { dSockets: control.control } },  // Extract the control array
      { upsert: true, new: true, setDefaultsOnInsert: true }
    )
    
    // Pass just the control array, not the entire control object
    controlToDevice(control.device, control.control);
  } catch (err) {
    console.error('Error updating device control:', err);
  }
}

client.on("connect", onMqttConnect);
client.on("message", onMqttMessage);

function onMqttConnect() {
  console.log("Connected to MQTT broker");
  client.subscribe([MAIN_TOPIC+"/#"], (err) => { // subscribe to all topics under MAIN_TOPIC
    if (err) {
      console.error("Failed to subscribe:", err);
    } else {
      console.log("Successfully subscribed to topics");
      pingToDevices();
    }
  });
}

function onMqttMessage(topic, message) {
  try {
    const jsonMessage = JSON.parse(message.toString());
    if (jsonMessage.sender !== SERVER_NAME) {
      // Handle control messages first, even if they come with a ping
      if ('control' in jsonMessage && Array.isArray(jsonMessage.control)) {
        controlFromDevice(jsonMessage);
      }
      
      if ('ping' in jsonMessage) {
        pingFromDevice(jsonMessage);
      } else if ('info' in jsonMessage) {
        infoFromDevice(jsonMessage);
      }
    }
  } catch (error) {
    console.error("Failed to parse message as JSON:", error);
  }
}

function pingToDevices() { 
  client.publish(MAIN_TOPIC, JSON.stringify({ sender: SERVER_NAME, meth: "get", ping: "ping" })); // Send ping to all devices, they respond in their corresponding subtopics.
  console.log('Sent ping to devices');
}

function configToDevice(device, config) {
  client.publish(device === "*" ? MAIN_TOPIC : MAIN_TOPIC + `/${device}`, JSON.stringify({ sender: SERVER_NAME, meth: "put", config: config })); // Send config to changed device
}

function controlToDevice(device, control) {
  const topic = device === "*" ? MAIN_TOPIC : MAIN_TOPIC + `/${device}`;
  client.publish(topic, JSON.stringify({ 
    sender: SERVER_NAME, 
    meth: "put", 
    control: control 
  }));
}

function pingFromDevice(message) { // typical message structure: "S3/XXXXXXXXXXXX" {sender: "XXXXXXXXXXXX", meth: "inf", ping: "pong" }
  const deviceMac = message.sender;
  updateDevicePing(deviceMac);
  console.log('Received ping from device:', deviceMac);
}

async function infoFromDevice(message) {
  const deviceMac = message.sender;
  const deviceInfo = message.info;
  
  try {
    const updatedDevice = await updateDeviceInfo(deviceMac, deviceInfo);
    
    const socketStates = updatedDevice.dSockets.map(socket => ({
      sId: socket.sId,
      sState: socket.sState
    })).filter(socket => socket.sState === true);

    if (socketStates.length > 0) {
      controlToDevice(deviceMac, socketStates);
      console.log('Restored socket states for device:', deviceMac, socketStates);
    }
  } catch (err) {
    console.error('Error handling device info:', err);
  }
}

function controlFromDevice(message) {
  const deviceMac = message.sender;
  const deviceControl = message.control;
  console.log('Received control from device:', deviceMac, deviceControl);
  
  updateSocketState(deviceMac, deviceControl)
    .then(() => {
      devicesToWebSocket(wss);
    })
    .catch(err => {
      console.error('Error updating socket state:', err);
    });
}

async function updateSocketState(deviceMac, deviceControl) {
  try {
    const existingDevice = await dbDeviceInfo.findOne({ dMac: deviceMac });
    if (existingDevice) {
      let updated = false;
      
      for (const control of deviceControl) {
        // Update all sockets that were included in the control message
        existingDevice.dSockets = existingDevice.dSockets.map(socket => {
          if (socket.sId === control.sId) {
            updated = true;
            return {
              ...socket,
              sState: control.sState,
              sLastUpdate: new Date()
            };
          }
          return socket;
        });
      }
      
      if (updated) {
        await existingDevice.save();
      }
    }
  } catch (err) {
    console.error(`Error updating socket state for device ${deviceMac}:`, err);
    throw err;
  }
}

async function updateDevicePing(deviceMac) {
  try {
    const existingDevice = await dbDeviceInfo.findOne({ dMac: deviceMac });
    if (existingDevice) {
      existingDevice.dLastConnection = new Date();
      existingDevice.dConnected = true; 
      await existingDevice.save();
    } else {
      console.error(`Device ${deviceMac} not found`);
    }
  } catch (err) {
    console.error(`Error finding device ${deviceMac}:`, err);
  }
}

async function updateDeviceInfo(deviceMac, deviceInfo) {
  try {
    console.log('Updating device info:', deviceMac, deviceInfo);
    
    const existingDevice = await dbDeviceInfo.findOne({ dMac: deviceMac });
    
    let initializedSockets;
    if (existingDevice) {
      initializedSockets = Array.from({ length: deviceInfo.dNumSockets }, (_, index) => {
        const existingSocket = existingDevice.dSockets.find(s => s.sId === (index + 1));
        return {
          sId: index + 1,
          sName: existingSocket?.sName || "Undefined",
          sState: existingSocket ? existingSocket.sState : (deviceInfo.dSockets?.[index]?.sState || false),
          sGroup: existingSocket?.sGroup || "",
          sInUse: existingSocket?.sInUse || false,
          sLastInUse: existingSocket?.sLastInUse || null,
          sLastUpdate: existingSocket?.sLastUpdate || new Date()
        };
      });
    } else {
      initializedSockets = Array.from({ length: deviceInfo.dNumSockets }, (_, index) => ({
        sId: index + 1,
        sName: "Undefined",
        sState: deviceInfo.dSockets?.[index]?.sState || false,
        sGroup: "",
        sInUse: false,
        sLastInUse: null,
        sLastUpdate: new Date()
      }));
    }

    const deviceData = {
      dMac: deviceMac,
      dId: deviceInfo.dId,
      dName: deviceInfo.dName,
      dIp: deviceInfo.dIp, 
      dNumSockets: deviceInfo.dNumSockets,
      dSockets: initializedSockets,
      dConnected: true,
      dLastConnection: new Date()
    };

    const device = await dbDeviceInfo.findOneAndUpdate(
      { dMac: deviceMac },
      deviceData,
      { upsert: true, new: true }
    );

    // Broadcast updated device list to all connected WebSocket clients
    devicesToWebSocket(wss);

    console.log('Device info updated successfully:', device);
    return device;
  } catch (err) {
    console.error(`Error finding device ${deviceMac}:`, err);
    throw err;
  }
}

async function updateDeviceConfig(device, config) {
  try {
    const oldDeviceInfo = await dbDeviceInfo.findOne({ dMac: device });
    if (!oldDeviceInfo) {
      throw new Error(`Device with MAC ${device} not found`);
    }

    const updatedSockets = config.dSockets.map((newSocket, index) => {
      let currentSocket = oldDeviceInfo.dSockets[index];

      let updatedSocket = {
        sId: newSocket.sId,
        sName: newSocket.sName,
        sState: newSocket.sState,
        sGroup: newSocket.sGroup,
        sInUse: newSocket.sInUse,
        sLastUpdate: currentSocket ? currentSocket.sLastUpdate : new Date()
      };

      if (
        !currentSocket ||
        currentSocket.sName !== newSocket.sName || 
        currentSocket.sGroup !== newSocket.sGroup || 
        currentSocket.sInUse !== newSocket.sInUse
      ) {
        updatedSocket.sLastUpdate = new Date();
      }

      return updatedSocket;
    });
    const updatedDevice = await dbDeviceInfo.findOneAndUpdate(
      { dMac: device }, 
      {   
        dId: config.dId,
        dName: config.dName,
        dIp: config.dIp,
        dGroup: config.dGroup,
        dSockets: updatedSockets,
        dLastUpdate: new Date(), 
      },
      { upsert: true, new: true, setDefaultsOnInsert: true }
    );

    if (!updatedDevice) {
      console.error("Failed to update device info in the database.");
    } else {
      console.log("Device info updated successfully:", updatedDevice);
    }
  } catch (err) {
    console.error("Failed to update device info:", err);
  }
}

async function infoFromDataBase(device) {
  try {
    const devices = await dbDeviceInfo.find({ dMac: device });
    return devices.map((device) => ({
      dMac: device.dMac,
      dId: device.dId,
      dName: device.dName,
      dIp: device.dIp,
      dNumSockets: device.dNumSockets,
      dSockets: device.dSockets,
      dConnected: device.dConnected,
      dLastConnection: device.dLastConnection,
      dLastUpdate: device.dLastUpdate
    }));
  } catch (err) {
    console.error('Error finding connected devices:', err);
    throw err;
  }
}