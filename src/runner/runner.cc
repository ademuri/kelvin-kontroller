#include <Arduino.h>
#include <EasyTransfer.h>

#include "arduino-controller.h"
#include "max31855-thermocouple.h"
#include "thermistor.h"

ArduinoController *controller = new ArduinoController();

void setup() {
  // Set nBOOT_SEL = 0, to allow entering the bootloader using BOOT0 pin
  uint32_t bitmask = 0xFFFFFFFF ^ FLASH_OPTR_nBOOT_SEL_Msk;
  *((uint32_t *)0x1FFF7800) &= bitmask;

  SPI.setMISO(ArduinoController::kSpiMiso);
  SPI.setSCLK(ArduinoController::kSpiSck);
  SPI.begin();

  controller->Init();
}

void loop() { controller->Step(); }
