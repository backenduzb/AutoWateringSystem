#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

#include <Arduino.h>

#define MOTOR_PIN 25

void setupMotor();
void setMotor(bool state);  // Yangi funksiya
void startMotor();
void stopMotor();

#endif