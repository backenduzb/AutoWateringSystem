#include "SoilSensor.h"
#include "config.h"

SoilSensor::SoilSensor(int pin, int dry, int wet) {
  this->pin = pin;
  this->dryValue = dry;
  this->wetValue = wet;
  pinMode(pin, INPUT);
}

int SoilSensor::readMoisture() {
  int sum = 0;
  for (int i = 0; i < 10; i++) {
    sum += analogRead(pin);
    delay(5);
  }
  
  int rawValue = sum / 10;
  float moisture = (float)(rawValue - wetValue) / (dryValue - wetValue) * 100.0;
  
  if (moisture > 100) moisture = 100;
  if (moisture < 0) moisture = 0;
  
  Serial.print("📊 Raw: ");
  Serial.print(rawValue);
  Serial.print(" → Moisture: ");
  Serial.print(moisture);
  Serial.println("%");
  
  return (int)moisture;
}

void SoilSensor::calibrate(int dry, int wet) {
  this->dryValue = dry;
  this->wetValue = wet;
}