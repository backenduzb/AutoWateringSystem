#ifndef SOIL_SENSOR_H
#define SOIL_SENSOR_H

#include <Arduino.h>

class SoilSensor {
private:
  int pin;
  int dryValue;
  int wetValue;
  
public:
  SoilSensor(int pin, int dry, int wet);
  int readMoisture();
  void calibrate(int dry, int wet);
};

#endif