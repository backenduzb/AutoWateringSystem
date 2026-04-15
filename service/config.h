#ifndef CONFIG_H
#define CONFIG_H

extern const char* ssid;
extern const char* password;

extern const char* serverHost;
extern const int serverPort;
extern const char* secretToken;

#define SOIL_SENSOR_PIN 34

extern const int DRY_VALUE;
extern const int WET_VALUE;

extern const unsigned long sendInterval;

extern const int INM;

#endif