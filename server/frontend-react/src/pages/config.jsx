import React, { useState } from 'react';
import HeaderComponent from './components/header.jsx';
import DeviceListComponent from './components/deviceList.jsx';
import DeviceConfigComponent from './components/deviceConfig.jsx';

function ConfigPage() {
  const [selectedDevice, setSelectedDevice] = useState(null);

  return (
    <div className="page">
      <HeaderComponent />
      <DeviceListComponent onDeviceSelect={setSelectedDevice} />
      <DeviceConfigComponent selectedDevice={selectedDevice} />
    </div>
  );
}

export default ConfigPage;