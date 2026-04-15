#include "MotorController.h"
#include <Arduino.h>

bool currentMotorState = false;

void setupMotor() {
  pinMode(MOTOR_PIN, OUTPUT);
  digitalWrite(MOTOR_PIN, LOW);
  currentMotorState = false;
  Serial.println("✅ Motor initialized");
}

void setMotor(bool state) {
  if (currentMotorState != state) {
    currentMotorState = state;
    digitalWrite(MOTOR_PIN, state ? HIGH : LOW);
    
    if (state) {
      Serial.println("🔛 Motor started");
    } else {
      Serial.println("🔴 Motor stopped");
    }
  }
}

void startMotor() {
  setMotor(true);
}

void stopMotor() {
  setMotor(false);
}