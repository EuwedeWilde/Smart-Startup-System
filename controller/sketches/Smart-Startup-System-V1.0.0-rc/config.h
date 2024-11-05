// --- DEFINE ---
#ifndef CONFIG_H
#define CONFIG_H

// --- DEVICE ---
#define DEVICE_ID 1

// --- LED ---
#define STAT_LED_PIN 32
#define CONNECTED_INTERVAL 1000
#define DISCONNECTED_INTERVAL 100

// --- MQTT ---
#define MQTT_TOPIC "S3"  
#define MQTT_SERVER "inmat.nl"  
#define MESSAGE_LENGTH 1024
#define TOPIC_LENGTH 32
#define MQTT_PORT 1883

// --- EEPROM ---
#define EEPROM_CFG_DEFINED 8
#define EEPROM_CONFIG 10  
#define EEPROM_SIZE 1024
#define IS_DEFINED 0
#define CREDENTIAL_LENGTH 32

// --- DEBUG ---
#define dbf Serial.printf

// --- CONNECTIVITY ---
#define ETH_OR_WIFI 1 // 0 is ETH, 1 is WIFI

// Wifi
#define WIFI_SSID "Inmat"
#define WIFI_PASSWORD "striptang"
#define WIFI_INTERVAL 1000

// Ethernet
#define ETH_PHY_CS            15 // Chip Select
#define ETH_PHY_IRQ           27 // Interupt
#define ETH_PHY_RST           4  // Reset 
#define ETH_SPI_SCK           14 // Clock
#define ETH_SPI_MISO          12 // MISO
#define ETH_SPI_MOSI          13 // MOSI
#define ETH_PHY_TYPE ETH_PHY_W5500
#define ETH_PHY_ADDR 1

// --- SOCKETS ---
#define NUM_SOCKETS 2
const int SOCKET_PINS[NUM_SOCKETS] = {25, 33};
const int BUTTON_PINS[NUM_SOCKETS] = {35, 34};

// --- TIMEOUTS --- 
#define TIMEOUT 20000
#define INTERVAL 1000

#endif
