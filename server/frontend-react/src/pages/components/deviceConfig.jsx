import React, { useState, useEffect } from 'react';
import { useWebSocket } from '../../scripts/WebSocketContext';

function DeviceConfigComponent({ selectedDevice }) {
  const [mode, setMode] = useState('device'); 
  const [socketConfig, setSocketConfig] = useState([]);
  const [selectedSocketIndex, setSelectedSocketIndex] = useState(0); 
  const [deviceConfig, setDeviceConfig] = useState({}); 
  const ws = useWebSocket(); 
  useEffect(() => {
    if (selectedDevice) {
      setSocketConfig(selectedDevice.dSockets);
      setDeviceConfig({
        dId: selectedDevice.dId,
        dName: selectedDevice.dName,
        dIp: selectedDevice.dIp,
      });
    }
  }, [selectedDevice]);

  useEffect(() => {
    if (ws) {
      ws.onopen = () => {
        console.log('WebSocket connection established');
      };
      ws.onerror = (error) => {
        console.error('WebSocket error:', error);
      };
      ws.onclose = () => {
        console.log('WebSocket connection closed');
      };
    }
  }, [ws]);

  const handleSocketChange = (index, field, value) => {
    const updatedSockets = [...socketConfig];
    updatedSockets[index][field] = value;
    setSocketConfig(updatedSockets);
  };

  const handleDeviceChange = (field, value) => {
    const updatedDevice = { ...deviceConfig };
    updatedDevice[field] = value;
    setDeviceConfig(updatedDevice);
  };

  const toggleSocketState = (index) => {
    const updatedSockets = [...socketConfig];
    updatedSockets[index].sState = updatedSockets[index].sState === 'on' ? 'off' : 'on';
    setSocketConfig(updatedSockets);
  };

  const handleSubmit = (e) => {
    e.preventDefault();
    if (ws) {
      const message = {
        device: selectedDevice.dMac,
        config: {
          ...deviceConfig,
          dSockets: socketConfig,
        },
      };
      const jsonMessage = JSON.stringify({ config: message });
      console.log('Sending WebSocket message:', jsonMessage); // Add this line
      ws.send(jsonMessage);
    }
  };

  return (
    <section className="device-config">
      <h2 className="device-config__title">Device Configuration</h2>
      <form className="device-config__form" onSubmit={handleSubmit}>
        <div className="device-config__mode-switch">
          <button 
            type="button"
            className={`device-config__button ${mode === 'device' ? 'device-config__mode-button--active' : ''}`} 
            onClick={() => setMode('device')}
          >
            Device
          </button>
          <button 
            type="button"
            className={`device-config__button ${mode === 'socket' ? 'device-config__mode-button--active' : ''}`} 
            onClick={() => setMode('socket')}
          >
            Sockets
          </button>
        </div>
        {mode === 'device' && selectedDevice && (
          <div className="device-config__device-menu">
            <label className="device-config__label">Device ID:</label>
            <input className="device-config__input" type="text" value={deviceConfig.dId} onChange={(e) => handleDeviceChange('dId', e.target.value)} />
            <label className="device-config__label">Device Name:</label>
            <input className="device-config__input" type="text" value={deviceConfig.dName} onChange={(e) => handleDeviceChange('dName', e.target.value)}/>
            <label className="device-config__label">Device IP:</label>
            <input className="device-config__input" type="text" value={deviceConfig.dIp} onChange={(e) => handleDeviceChange('dIp', e.target.value)}/>
          </div>
        )}
        {mode === 'socket' && selectedDevice && (
          <div className="devive-config__socket-config">
            <div className="device-config__socket-select">
              {socketConfig.map((socket, index) => (
                <button 
                  key={socket.sId} 
                  type="button" 
                  className={`device-config__button ${selectedSocketIndex === index ? 'device-config__mode-button--active' : ''}`} 
                  onClick={() => setSelectedSocketIndex(index)}
                >
                  {socket.sName || `Socket ${index + 1}`}
                </button>
              ))}
            </div>
            {socketConfig[selectedSocketIndex] && (
              <div className="device-config__socket-menu">
                <label className="device-config__label">Socket Name:</label>
                <input 
                  className="device-config__input" 
                  type="text" 
                  value={socketConfig[selectedSocketIndex].sName} 
                  onChange={(e) => handleSocketChange(selectedSocketIndex, 'sName', e.target.value)} 
                />
                <label className="device-config__label">Socket State:</label>
                <button 
                  className="device-config__input" 
                  type="button" 
                  onClick={() => toggleSocketState(selectedSocketIndex)}
                >
                  {socketConfig[selectedSocketIndex].sState === 'on' ? 'On' : 'Off'}
                </button>
                <label className="device-config__label">Socket Group:</label>
                <input 
                  className="device-config__input" 
                  type="text" 
                  value={socketConfig[selectedSocketIndex].sGroup} 
                  onChange={(e) => handleSocketChange(selectedSocketIndex, 'sGroup', e.target.value)} 
                />
              </div>
            )}
          </div>
        )}
        <button className="device-config__button" type="submit">Save</button>
      </form>
    </section>
  );
}

export default DeviceConfigComponent;