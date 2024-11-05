import { StrictMode } from 'react';
import { createRoot } from 'react-dom/client';
import ConfigPage from './pages/config.jsx';
import { WebSocketProvider } from './scripts/WebSocketContext.jsx';
import '/style.scss';

createRoot(document.getElementById('root')).render(
  <StrictMode>
    <WebSocketProvider url="ws://localhost:3000">
      <ConfigPage />
    </WebSocketProvider>
  </StrictMode>,
);