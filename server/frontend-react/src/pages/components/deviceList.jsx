import React, { useEffect, useState } from 'react';
import { useWebSocket } from '../../scripts/WebSocketContext';

function DeviceListComponent({ onDeviceSelect }) {
  const ws = useWebSocket();
  const [devices, setDevices] = useState([]);
  const [error, setError] = useState(null);

  useEffect(() => {
    if (ws) {
      ws.onopen = () => {
        console.log('WebSocket connection established');
        ws.send(JSON.stringify({ action: 'getDevices' }));
      };

      ws.onmessage = (event) => {
        try {
          const data = JSON.parse(event.data);
          if (data.devices) {
            setDevices(data.devices);
          } else if (data.action === 'deviceStatus' && data.device) {
            setDevices((prevDevices) => {
              const deviceExists = prevDevices.some(device => device.dId === data.device.dId);
              if (deviceExists) {
                return prevDevices.map((device) =>
                  device.dId === data.device.dId ? data.device : device
                );
              } else {
                return [...prevDevices, data.device];
              }
            });
          }
        } catch (err) {
          setError('Failed to parse WebSocket message');
        }
      };

      ws.onerror = (err) => {
        setError('WebSocket error');
        console.error('WebSocket error:', err);
      };

      ws.onclose = () => {
        console.log('WebSocket connection closed');
      };
    }
  }, [ws]);

  return (
    <section className="device-list">
      <h2 className='device-list__title'>Devices</h2>
      {error && <div className="device-list__error">{error}</div>}
      <ul className="device-list__items">
        {devices.map((device, index) => (
          <li 
            key={index}
            className={`device-list__item device-list__item--${device.dConnected ? 'connected' : 'disconnected'}`}
            onClick={() => onDeviceSelect(device)}
          >
            {`├── ${device.dMac} | ${device.dId} | ${device.dName} | ${device.dNumSockets} | ${device.dConnected ? "connected" : "disconnected"}`}
            <ul className="device-list__sockets">
              {device.dSockets.map((socket, index) => (
                <li 
                  key={index} 
                  className={`device-list__socket device-list__socket--${socket.sState ? 'on' : 'off'}`}
                >
                  {`│   ├── ${socket.sName === undefined ? "Socket" : socket.sName} | ${socket.sState ? "on" : "off"}`.replace(/ /g, '\u00A0')}
                </li>
              ))}
            </ul>
          </li>
        ))}
      </ul>
    </section>
  );
}

export default DeviceListComponent;