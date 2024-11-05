import React from 'react'; 
import Header from './components/header'; 
import Devices from './components/devices'; 
import { WebSocketProvider } from './websocket'; 

function App() {
  return (
    <WebSocketProvider>
      <div className="app-container">
        <Header />
        <Devices />
      </div>
    </WebSocketProvider>
  );
}

export default App;
