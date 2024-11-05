<script context="module">
  import { writable } from 'svelte/store';

  export const devices = writable([]);
  export const connectionStatus = writable('disconnected');
  export const selectedDevice = writable(null);
  let socket;

  export function getWebSocket() {
    return socket;
  }

  function initializeWebSocket() {
    if (socket) return;

    devices.set([]);
    connectionStatus.set('connecting');
    console.log('Attempting to connect WebSocket...');
    
    socket = new WebSocket('ws://localhost:3000');

    socket.onopen = () => {
      console.log('WebSocket connected successfully');
      connectionStatus.set('connected');
      socket.send(JSON.stringify({ 
        type: 'getDevices'
      }));
    };

    socket.onmessage = (event) => {
      console.log('Received raw message:', event.data);
      try {
        const data = JSON.parse(event.data);
        console.log('Parsed message data:', data);
        
        if (data.devices && Array.isArray(data.devices)) {
          console.log('Setting devices from array:', data.devices);
          devices.set(data.devices);
          
          selectedDevice.update(current => {
            if (current) {
              const updatedDevice = data.devices.find(d => d.dMac === current.dMac);
              return updatedDevice || current;
            }
            return current;
          });
        }
        else if (data.type === "deviceStatus" && data.device) {
          console.log('Updating single device:', data.device);
          devices.update(currentDevices => {
            const existingDeviceIndex = currentDevices.findIndex(d => d.dMac === data.device.dMac);
            if (existingDeviceIndex >= 0) {
              currentDevices[existingDeviceIndex] = data.device;
              return [...currentDevices];
            } else {
              return [...currentDevices, data.device];
            }
          });
        }
      } catch (error) {
        console.error('Error parsing WebSocket message:', error);
      }
    };

    socket.onerror = (error) => {
      console.error('WebSocket error:', error);
      connectionStatus.set('error');
    };

    socket.onclose = () => {
      console.log('WebSocket connection closed');
      connectionStatus.set('disconnected');
      setTimeout(initializeWebSocket, 5000);
    };
  }

  export function connectWebSocket() {
    initializeWebSocket();
  }
</script>
