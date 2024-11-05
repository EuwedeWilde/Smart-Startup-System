import React, { useEffect, useState } from 'react';
import { useWebSocket } from '../websocket';
import Config from './config'; 

const Devices = () => {
  const [deviceList, setDeviceList] = useState([]);
  const [selectedDevice, setSelectedDevice] = useState(null); 
  const [isLoading, setIsLoading] = useState(true);
  const [error, setError] = useState(null);
  const { ws, sendMessage, isConnected } = useWebSocket();

  useEffect(() => {
    if (isConnected) {
      console.log('WebSocket connected, requesting device list');
      sendMessage({ type: 'requestDeviceList' });
    }
  }, [isConnected, sendMessage]);

  useEffect(() => {
    if (!ws) return;

    const handleMessage = (event) => {
      try {
        const data = JSON.parse(event.data);
        console.log('Received WebSocket message:', data);
        if (data.type === 'updateDeviceList') {
          setDeviceList(data.deviceInfo);
          setIsLoading(false);
          // Update selectedDevice if it exists in the new list, otherwise set to null
          if (selectedDevice) {
            const updatedDevice = data.deviceInfo.find(device => device.did === selectedDevice.did);
            setSelectedDevice(updatedDevice || null);
          }
        }
      } catch (err) {
        console.error('Error parsing WebSocket message:', err);
        setError('Error parsing WebSocket message');
        setIsLoading(false);
      }
    };

    ws.addEventListener('message', handleMessage);

    return () => {
      ws.removeEventListener('message', handleMessage);
    };
  }, [ws, selectedDevice]);

  function clickedDevice(device) {
    console.log(`Device clicked:`, device);
    setSelectedDevice(device);
    sendMessage({
      type: 'selectDevice',
      device: device,
    });
  }

  return (
    <main>
      <section>
        <h2>Devices in LAN</h2>
        <div className="device-list-container">
          <ul className="device-list">
            {deviceList.map((device) => (
              <li key={device.did}>
                <button 
                  className={`device-button ${selectedDevice?.did === device.did ? 'selected' : ''}`} 
                  onClick={() => clickedDevice(device)}
                >
                  {device.did} | {device.dName}
                </button>
              </li>
            ))}
          </ul>
        </div>
      </section>
      <Config selectedDevice={selectedDevice} />
    </main>
  );
};

export default Devices;
