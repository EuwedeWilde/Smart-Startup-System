void setupRelays() {
  for (int i = 0; i < NUM_SOCKETS; i++) {
    pinMode(SOCKET_PINS[i], OUTPUT);
    pinMode(BUTTON_PINS[i], INPUT); 
    relayState[i] = false;
    buttonState[i] = HIGH; 
  }
}

void controlFromServer(int relay, bool onOff) {
  digitalWrite(SOCKET_PINS[relay], onOff);
  relayState[relay] = onOff; 
}

void controlFromButtons() {
  for (int i = 0; i < NUM_SOCKETS; i++) {
    bool currentButtonState = digitalRead(BUTTON_PINS[i]);
    
    if (currentButtonState != buttonState[i]) {
      delay(50);
      currentButtonState = digitalRead(BUTTON_PINS[i]);
      if (currentButtonState != buttonState[i]) {
        buttonState[i] = currentButtonState;
        if (currentButtonState == LOW) {
          relayState[i] = !relayState[i];
          digitalWrite(SOCKET_PINS[i], relayState[i]);
          Serial.print("Relay ");
          Serial.print(i);
          Serial.print(" toggled to ");
          Serial.println(relayState[i] ? "ON" : "OFF");
          pubControl(); 
        }
      }
    }
  }
}
