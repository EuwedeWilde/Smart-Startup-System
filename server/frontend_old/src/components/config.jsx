import React, { useState, useEffect } from 'react';
import { useWebSocket } from '../websocket';

function Config({ selectedDevice }) {
  const [editMode, setEditMode] = useState(false);
  const [deviceName, setDeviceName] = useState('');
  const [deviceIp, setDeviceIp] = useState('');
  const [selectedSocket, setSelectedSocket] = useState('');
  const [socketName, setSocketName] = useState('');
  const [socketState, setSocketState] = useState(false);
  const [socketGroup, setSocketGroup] = useState('');
  const [error, setError] = useState(null);
  const { ws, error: wsError } = useWebSocket();
  const [isSelectOpen, setIsSelectOpen] = useState(false);
  const [sockets, setSockets] = useState([]);
  const [lastSelectedDeviceId, setLastSelectedDeviceId] = useState(null);

  useEffect(() => {
    if (selectedDevice && selectedDevice.did !== lastSelectedDeviceId) {
      console.log('Config component: selectedDevice changed', selectedDevice);
      setDeviceName(selectedDevice.dName || '');
      setDeviceIp(selectedDevice.dIp || '');
      setSockets(selectedDevice.sockets || []);
      setSelectedSocket('');
      setSocketName('');
      setSocketState(false);
      setSocketGroup('');
      setLastSelectedDeviceId(selectedDevice.did);
    } else if (!selectedDevice && lastSelectedDeviceId !== null) {
      // Clear all fields when selectedDevice becomes null
      clearConfigFields();
      setLastSelectedDeviceId(null);
    }
  }, [selectedDevice]);

  useEffect(() => {
    if (wsError) {
      setError(`WebSocket error: ${wsError}`);
    }
  }, [wsError]);

  const handleNameChange = (e) => {
    setDeviceName(e.target.value);
    setEditMode(true);
  };
  const handleIpChange = (e) => {
    setDeviceIp(e.target.value);
    setEditMode(true);
  };
  const handleSocketSelect = (socketName) => {
    setSelectedSocket(socketName);
    setIsSelectOpen(false);
    if (socketName) {
      const socket = selectedDevice.sockets.find(s => s.name === socketName);
      setSocketName(socket.name);
      setSocketState(socket.state);
      setSocketGroup(socket.group || '');
    } else {
      setSocketName('');
      setSocketState(false);
      setSocketGroup('');
    }
    setEditMode(true);
  };
  const handleSocketNameChange = (e) => {
    setSocketName(e.target.value);
    setEditMode(true);
  };
  const handleSocketStateToggle = () => {
    setSocketState(!socketState);
    setEditMode(true);
    ws.send(JSON.stringify({
      type: 'controlSocket',
      did: selectedDevice.did,
      mac: selectedDevice.dMac,
      socketName: selectedSocket,
      state: !socketState
    }));
  };
  const handleSocketGroupChange = (e) => {
    setSocketGroup(e.target.value);
    setEditMode(true);
  };

  const handleSubmit = (e) => {
    e.preventDefault();
    if (ws && ws.readyState === WebSocket.OPEN && selectedDevice) {
      const socketUpdate = selectedSocket ? {
        oldName: selectedSocket,
        newName: socketName,
        state: socketState,
        group: socketGroup
      } : null;

      ws.send(JSON.stringify({
        type: 'updateDevice',
        did: selectedDevice.did,
        mac: selectedDevice.dMac,
        newName: deviceName,
        newIp: deviceIp,
        socketUpdate: socketUpdate
      }));

      if (socketUpdate) {
        const updatedSockets = sockets.map(socket => 
          socket.name === selectedSocket 
            ? { ...socket, name: socketName, state: socketState, group: socketGroup }
            : socket
        );
        setSockets(updatedSockets);
        setSelectedSocket(socketName);
      }
      setEditMode(false);
    } else {
      console.error('WebSocket is not open or no device selected');
      setError('Cannot update device: WebSocket is not connected or no device selected');
    }
  };

  const clearConfigFields = () => {
    setDeviceName('');
    setDeviceIp('');
    setSockets([]);
    setSelectedSocket('');
    setSocketName('');
    setSocketState(false);
    setSocketGroup('');
    setEditMode(false);
  };

  if (error) {
    return <div>Error: {error}</div>;
  }

  return (
    <section>
      <h2>Configuration</h2>
      <form onSubmit={handleSubmit}>
        <div className="config-grid">
          <div><p className="config-text">Device ID</p></div>
          <div><p className="config-text-value">{selectedDevice?.did || ''}</p></div>
          <div><p className="config-text">MAC Address</p></div>
          <div><p className="config-text-value">{selectedDevice?.dMac || ''}</p></div>
          <div><p className="config-text">Device Name</p></div>
          <div>
            <input 
              type="text" 
              value={deviceName} 
              onChange={handleNameChange}
              disabled={!selectedDevice}
            />
          </div>
          <div><p className="config-text">IP Address</p></div>
          <div>
            <input 
              type="text" 
              value={deviceIp} 
              onChange={handleIpChange}
              disabled={!selectedDevice}
            />
          </div>
          <div><p className="config-text">Select Socket</p></div>
          <div className="custom-select">
            <div 
              className={`select-selected ${isSelectOpen ? 'select-arrow-active' : ''}`}
              onClick={() => setIsSelectOpen(!isSelectOpen)}
            >
              {selectedSocket}
            </div>
            <div className={`select-items ${isSelectOpen ? '' : 'select-hide'}`}>
              {sockets.map(socket => (
                <div 
                  key={socket.name}
                  onClick={() => handleSocketSelect(socket.name)}
                  className={selectedSocket === socket.name ? 'same-as-selected' : ''}
                >
                  {socket.name}
                </div>
              ))}
            </div>
          </div>
          <div><p className="config-text">Socket Name</p></div>
          <div>
            <input 
              type="text" 
              value={socketName} 
              onChange={handleSocketNameChange}
              disabled={!selectedSocket}
            />
          </div>
          <div><p className="config-text">Socket State</p></div>
          <div>
            {selectedSocket ? (
              <button
                type="button"
                className="toggle-button"
                onClick={handleSocketStateToggle}
              >
                {socketState ? "ON" : "OFF"}
              </button>
            ) : (
              <button className="config-text-value"></button>
            )}
          </div>
          <div><p className="config-text">Socket Group</p></div>
          <div>
            <input 
              type="text" 
              value={socketGroup} 
              onChange={handleSocketGroupChange}
              disabled={!selectedSocket}
            />
          </div>
        </div>
        <div className="save-button-container">
          <button 
            className="save-button" 
            type="submit" 
            disabled={!selectedDevice || !editMode}
          >
            Save configuration
          </button>
        </div>
      </form>
    </section>
  );
}

export default Config;