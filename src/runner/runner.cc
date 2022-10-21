#include <Arduino.h>

#include "arduino-controller.h"
#include "max31855-thermocouple.h"
#include "thermistor.h"

ArduinoController* controller = new ArduinoController();

void setup() {
  SPI.setMISO(ArduinoController::kSpiMiso);
  SPI.setSCLK(ArduinoController::kSpiSck);

  controller->Init();
}

void loop() { controller->Step(); }
