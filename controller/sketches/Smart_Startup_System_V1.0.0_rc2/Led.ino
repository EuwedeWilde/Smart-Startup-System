int ledState;
unsigned long previousMillisLed = 0; 

void setupLed () {
  pinMode(STAT_LED_PIN, OUTPUT);
  ledState = LOW;
  digitalWrite(STAT_LED_PIN, ledState);
}

void ledConnected() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillisLed >= CONNECTED_INTERVAL) {
    previousMillisLed = currentMillis;  // Changed from previousMillis to previousMillisLed
    ledState = !ledState;
    digitalWrite(STAT_LED_PIN, ledState);
  }
}

void ledDisconnectedConnection() {
  ledState = HIGH;
  for (int i = 0; i < 5; i++) {
    digitalWrite(STAT_LED_PIN, ledState);
    delay(DISCONNECTED_INTERVAL);
    ledState = !ledState;
  }
  digitalWrite(STAT_LED_PIN, ledState);
}

void ledDisconnectedMqtt() {
  ledState = HIGH;
  for (int i = 0; i < 9; i++) {
    digitalWrite(STAT_LED_PIN, ledState);
    delay(DISCONNECTED_INTERVAL);
    ledState = !ledState;
  }
  digitalWrite(STAT_LED_PIN, ledState);
}
