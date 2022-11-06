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

#include <bitset>
#include <sstream>

#include "constants.h"
#include "html.h"
#include "types.h"

constexpr size_t kJsonBufferSize = 1000;
constexpr int kRx = 16;
constexpr int kTx = 17;

constexpr char kTemp[] = "temp";
constexpr char kFan[] = "fan";
constexpr char kReset[] = "reset";
constexpr char kP[] = "p";
constexpr char kI[] = "i";
constexpr char kD[] = "d";

AsyncWebServer server(80);
AsyncWebSocket websocket("/websocket");

RunnerCommand command;
RunnerStatus status;
EasyTransfer<RunnerStatus> transfer_in{&status};
EasyTransfer<RunnerCommand> transfer_out{&command};
uint32_t received_at = 0;

constexpr uint8_t kScreenWidth = 64;
constexpr uint8_t kScreenHeight = 128;
constexpr int kSda = 13;
constexpr int kScl = 4;
Adafruit_SH1107 oled = Adafruit_SH1107(kScreenWidth, kScreenHeight, &Wire);

constexpr size_t kFirmwareBufferSize = 65536;
uint8_t firmware_buffer[kFirmwareBufferSize];

const char *faultToBinaryString(const RunnerFault &fault) {
  static constexpr size_t kBufferSize = 20;
  static char buffer[kBufferSize];
  snprintf(buffer, kBufferSize, "%u%u%u%u%u%u%u%u%u", fault.bean_temp_low,
           fault.bean_temp_high, fault.env_temp_low, fault.env_temp_high,
           fault.ambient_temp_low, fault.ambient_temp_high,
           fault.bean_temp_read_error != 0, fault.env_temp_read_error != 0,
           fault.no_comms);
  return buffer;
}

const char *faultToDebugString(const RunnerFault &fault) {
  static constexpr size_t kBufferSize = 500;
  static char buffer[kBufferSize];
  std::stringstream stream;

  if (fault.bean_temp_high) {
    stream << "bean_temp_high, ";
  }
  if (fault.bean_temp_low) {
    stream << "bean_temp_low, ";
  }
  if (fault.env_temp_high) {
    stream << "env_temp_high, ";
  }
  if (fault.env_temp_low) {
    stream << "env_temp_low, ";
  }
  if (fault.ambient_temp_high) {
    stream << "ambient_temp_high, ";
  }
  if (fault.ambient_temp_low) {
    stream << "ambient_temp_low, ";
  }
  if (fault.bean_temp_read_error != 0) {
    stream << "bean_error: "
           << std::bitset<3>(fault.bean_temp_read_error).to_string() << ", ";
  }
  if (fault.env_temp_read_error != 0) {
    stream << "env_error: "
           << std::bitset<3>(fault.env_temp_read_error).to_string() << ", ";
  }

  snprintf(buffer, kBufferSize, "%s", stream.str().c_str());

  return buffer;
}

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
      data["HEATER"] = status.heater_output;
      data["FAULT"] = faultToDebugString(status.fault_since_reset);
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
      JsonObject params = request_message["params"];

      if (params.containsKey(kTemp)) {
        command.target_temp = params[kTemp];
      }
      if (params.containsKey(kFan)) {
        command.fan_speed = params[kFan];
      }
      if (params.containsKey(kReset)) {
        Serial.println("Resetting fault");
        command.reset = true;
      }
      if (params.containsKey(kP)) {
        command.p = params[kP];
      }
      if (params.containsKey(kI)) {
        command.i = params[kI];
      }
      if (params.containsKey(kD)) {
        command.d = params[kD];
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
  transfer_in.begin(&Serial2);
  transfer_out.begin(&Serial2);
  Serial.println(" done.");

  Serial.print("Initializing display...");
  Wire.begin(kSda, kScl, /*i2c frequency=*/(uint32_t)1000000);
  oled.begin(/*address=*/0x3C, /*reset=*/false);
  oled.setRotation(1);
  oled.clearDisplay();
  oled.display();
  oled.setTextColor(SH110X_WHITE);
  oled.setTextSize(2);
  Serial.println(" done.");
}

void loop() {
  websocket.cleanupClients();
  bool do_reset = command.reset;
  if (transfer_in.receiveData()) {
    received_at = millis();
    transfer_out.sendData();
    if (do_reset) {
      command.reset = false;
    }

    oled.clearDisplay();
    oled.setCursor(0, 0);
    oled.printf("BEAN: %3.0fF\n", status.bean_temp);
    oled.printf(" ENV: %3.0fF\n", status.env_temp);
    oled.printf("%s", faultToBinaryString(status.fault_since_reset));
    oled.display();
  }
}
