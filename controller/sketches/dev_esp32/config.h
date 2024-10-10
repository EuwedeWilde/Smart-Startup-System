#ifndef CONFIG_H
#define CONFIG_H

#define DEVICE_ID 1
#define MQTT_TOPIC "S3"  
#define MQTT_SERVER "inmat.nl"  

#define WIFI_SSID "Inmat"
#define WIFI_PASSWORD "striptang"

#define WIFI_INTERVAL 1000

#define SERVER_NAME "server"

#define DEVICE_NAME "CTRLR2"

#define MAX_SOCKETS 3

const int SOCKET_PINS[MAX_SOCKETS] = {33, 25, 26};

#endif