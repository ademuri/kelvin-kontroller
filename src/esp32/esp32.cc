#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <EasyTransfer.h>
#include <WiFi.h>
#include <Wire.h>

#include "constants.h"
#include "html.h"
#include "types.h"

constexpr size_t kJsonBufferSize = 1000;
constexpr int kRx = 16;
constexpr int kTx = 17;

constexpr char kTemp[] = "temp";
constexpr char kFan[] = "fan";
constexpr char kReset[] = "reset";

AsyncWebServer server(80);
AsyncWebSocket websocket("/websocket");

RunnerCommand command;
RunnerStatus status;
EasyTransfer transfer_in;
EasyTransfer transfer_out;
uint32_t received_at = 0;

constexpr uint8_t kScreenWidth = 128;
constexpr uint8_t kScreenHeight = 64;
constexpr int kSda = 13;
constexpr int kScl = 4;
Adafruit_SH1107 oled = Adafruit_SH1107(kScreenWidth, kScreenHeight, &Wire);

constexpr size_t kFirmwareBufferSize = 65536;
uint8_t firmware_buffer[kFirmwareBufferSize];

// Handle a web socket message. Compatible with Artisan ().
// For (sparse) documentation on the protocol, see:
// Official documentation: https://artisan-scope.org/devices/websockets/
// Discussion: https://github.com/artisan-roaster-scope/artisan/discussions/701
// Source:
// https://github.com/artisan-roaster-scope/artisan/blob/192bb47f9ecc1123f194926b43bca269a1d5b9a5/src/artisanlib/wsport.py
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  static StaticJsonDocument<kJsonBufferSize> request_message;
  static StaticJsonDocument<kJsonBufferSize> response_message;
  static char buffer[kJsonBufferSize];

  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len &&
      info->opcode == WS_TEXT) {
    data[len] = 0;
    Serial.printf("WebSocket message received: %s\n", data);
    DeserializationError err = deserializeJson(request_message, data);
    if (err) {
      Serial.printf("WebSocket error while deserializing json: %s\n",
                    err.c_str());
      return;
    }

    response_message.clear();
    response_message["id"] = request_message["id"];
    if (strcmp(request_message["command"], "getData") == 0) {
      JsonObject data = response_message.createNestedObject("data");
      // TODO: set these from the controller
      data["BT"] = status.bean_temp;
      data["ET"] = status.env_temp;
      data["AT"] = status.ambient_temp;
      data["FAN"] = status.fan_speed;
      data["FAULT"] = status.fault.Faulty();
      data["UPDATED"] = millis() - received_at;

      // Note: can use websocket.makeBuffer(len) if this is slow:
      // https://github.com/me-no-dev/ESPAsyncWebServer#direct-access-to-web-socket-message-buffer
      size_t len = serializeJson(response_message, buffer);
      if (len == 0 || len > kJsonBufferSize) {
        Serial.printf(
            "WebSocket error while serializing json: wrote %u bytes\n", len);
        return;
      }
      websocket.textAll(buffer, len);
    } else if (strcmp(request_message["command"], "setParams") == 0) {
      if (!request_message.containsKey("params")) {
        Serial.printf("WebSocket error: no params in 'setParams' message");
        return;
      }

      if (request_message.containsKey(kTemp)) {
        command.target_temp = request_message[kTemp];
      }
      if (request_message.containsKey(kFan)) {
        command.fan_speed = request_message[kFan];
      }
      if (request_message.containsKey(kReset) && request_message[kReset]) {
        command.reset = true;
      }
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
             AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(),
                    client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_ERROR:
      Serial.printf("WebSocket client #%u error %u: %s\n", client->id(),
                    *((uint16_t *)arg), (char *)data);
      break;
    case WS_EVT_PONG:
      Serial.printf("WebSocket client #%u pong: %s\b", client->id(),
                    (len) ? (char *)data : "");
      break;
  }
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
  WiFi.setSleep(false);

  if (MDNS.begin("roaster")) {
    // Add service to MDNS-SD
    MDNS.addService("http", "tcp", 80);
    Serial.println("mDNS responder started");
  } else {
    Serial.println("Error setting up MDNS responder!");
  }

  Serial.print("Starting server...");
  websocket.onEvent(onEvent);
  server.addHandler(&websocket);
  server.begin();
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", index_html);
  });
  server.on(
      "/", HTTP_POST,
      [](AsyncWebServerRequest *request) {
        AsyncWebServerResponse *response =
            request->beginResponse(200, "text/plain", "OK");
        response->addHeader("Connection", "close");
        request->send(response);
      },
      [](AsyncWebServerRequest *request, String filename, size_t index,
         uint8_t *data, size_t len, bool final) {
        static size_t received_size = 0;
        static bool failed = false;
        if (index == 0) {
          Serial.printf("Begin update: %s\n", filename.c_str());
          received_size = 0;
          failed = false;
          memset(firmware_buffer, 0, kFirmwareBufferSize);
        }
        received_size += len;
        if (received_size > kFirmwareBufferSize) {
          Serial.printf(
              "Error: received firmware larger than buffer: %u vs %u\n",
              received_size, kFirmwareBufferSize);
          failed = true;
        }
        if (!failed) {
          memcpy((firmware_buffer + index), data, len);
        }

        if (final && !failed) {
          Serial.printf(
              "Update received (flashing not yet implemented). Received %u "
              "bytes.\n",
              received_size);
        }
      });
  Serial.println(" done.");

  Serial.print("Initializing serial...");
  Serial2.begin(kSerialBaud, SERIAL_8N1, kRx, kTx);
  transfer_in.begin(details(status), &Serial2);
  transfer_out.begin(details(command), &Serial2);
  Serial.println(" done.");

  Serial.print("Initializing display...");
  Wire.begin(kSda, kScl);
  oled.begin(/*address=*/0x3C);
  oled.clearDisplay();
  oled.setCursor(0, 0);
  oled.setTextSize(3);
  oled.println("Hello, world");
  oled.display();
  Serial.println(" done.");
}

void loop() {
  websocket.cleanupClients();
  if (transfer_in.receiveData()) {
    received_at = millis();
    transfer_out.sendData();
  }
}
