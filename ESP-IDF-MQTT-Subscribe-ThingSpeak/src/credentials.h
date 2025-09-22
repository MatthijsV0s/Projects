#include "mqtt_secrets.h"       // Create ThingSpeak MQTT Device see: https://nl.mathworks.com/help/thingspeak/mqtt-basics.html
#ifndef CREDENTIALS_H
#define CREDENTIALS_H

//ThingSpeak channel ID
#define CHANNEL_ID                "3066180"  // replace by Thingspeak Channel ID
//MQTT configuration
#define URI                       "mqtt://mqtt3.thingspeak.com"
#define PORT                      1883
#define USERNAME                  SECRET_MQTT_USERNAME
#define CLIENT_ID                 SECRET_MQTT_CLIENT_ID
#define MQTT_PASSWORD             SECRET_MQTT_PASSWORD

#endif