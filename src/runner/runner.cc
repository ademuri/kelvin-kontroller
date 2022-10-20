#include <Arduino.h>

#include "arduino-controller.h"
#include "max31855-thermocouple.h"
#include "thermistor.h"

ArduinoController controller;

void setup() {
  SPI.setMISO(PA6);
  SPI.setSCLK(PA5);
}

void loop() {}