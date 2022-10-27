#include <Arduino.h>
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <EasyTransfer.h>
#include <ModbusServerTCPasync.h>
#include <WiFi.h>

#include "constants.h"
#include "types.h"

constexpr bool kDebug = true;

constexpr size_t kJsonBufferSize = 1000;
constexpr int kRx = 16;
constexpr int kTx = 17;

ModbusServerTCPasync modbus_server;
constexpr uint8_t kModbusId = 1;
constexpr int kModbusPort = 502;
constexpr int kModbusMaxClients = 4;
constexpr uint32_t kModbusTimeout = 10000;

RunnerCommand command;
RunnerStatus status;
EasyTransfer transfer_in;
EasyTransfer transfer_out;

ModbusMessage modbusRead(ModbusMessage request) {
  if (kDebug) {
    Serial.printf("[modbus] serverID: %u, function: %u, data: ", request.getServerID(), request.getFunctionCode());
    for (uint16_t i = 2; i < request.size(); i++) {
      uint16_t value;
      request.get(i, value);
      Serial.printf("%02X %02X", (value << 8) & 0xFF, value & 0xFF);
    }
    Serial.println();
  }

  ModbusMessage response{/*dataLen=*/8};
  uint16_t address = 0;
  uint16_t words = 0;
  request.get(2, address);
  request.get(4, words);

  if (address == 0 || address > 3) {
    response.setError(request.getServerID(), request.getFunctionCode(),
                      ILLEGAL_DATA_ADDRESS);
    return response;
  }

  if (words != 1) {
    Serial.printf(
        "Warning: received modbus read requesting multiple words: %u\n", words);
  }

  response.add(request.getServerID(), request.getFunctionCode(),
               /* response length */ 2);
  switch (address) {
    case 1:
      response.add(status.bean_temp);
      break;

    case 2:
      response.add(status.env_temp);
      break;

    case 3:
      response.add(status.ambient_temp);
      break;

    default:
      // Should never happen!
      response.add((uint16_t) 0);
      Serial.printf("Illegal state: invalid modbus address: %u\n", address);
      break;
  }

  return response;
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(kSsid, kPassword);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.print("Initializing serial...");
  Serial2.begin(kSerialBaud, SERIAL_8N1, kRx, kTx);
  transfer_in.begin(details(status), &Serial2);
  transfer_out.begin(details(command), &Serial2);
  Serial.println(" done.");

  Serial.print("Initialing ModBus server...");
  modbus_server.registerWorker(kModbusId, READ_HOLD_REGISTER, &modbusRead);
  modbus_server.start(kModbusPort, kModbusMaxClients, kModbusTimeout);
  Serial.println(" done.");
}

void loop() {
  if (transfer_in.receiveData()) {
    transfer_out.sendData();
  }
}
