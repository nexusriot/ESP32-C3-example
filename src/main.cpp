#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

#define LED_PIN 8

const char* WIFI_SSID = "SSID";
const char* WIFI_PASS = "Password";

AsyncWebServer server(80);

const char* NTP_SERVER = "pool.ntp.org";
const long GMT_OFFSET_SEC = 4 * 3600; // Armenia UTC+4
const int DAYLIGHT_OFFSET_SEC = 0;

bool ledState = false;
unsigned long lastBlinkMs = 0;

String timeString() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo, 100)) {
    return "not_synced";
  }

  char buf[32];
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(buf);
}

String makeJson() {
  JsonDocument doc;

  doc["ok"] = true;
  doc["device"] = "ESP32-C3 Super Mini";
  doc["chip_model"] = ESP.getChipModel();
  doc["chip_revision"] = ESP.getChipRevision();
  doc["cpu_mhz"] = ESP.getCpuFreqMHz();
  doc["flash_size"] = ESP.getFlashChipSize();
  doc["free_heap"] = ESP.getFreeHeap();
  doc["min_free_heap"] = ESP.getMinFreeHeap();
  doc["uptime_ms"] = millis();

  doc["wifi_ssid"] = WiFi.SSID();
  doc["ip"] = WiFi.localIP().toString();
  doc["gateway"] = WiFi.gatewayIP().toString();
  doc["subnet"] = WiFi.subnetMask().toString();
  doc["dns"] = WiFi.dnsIP().toString();
  doc["mac"] = WiFi.macAddress();
  doc["rssi"] = WiFi.RSSI();

  doc["time"] = timeString();

  String out;
  serializeJsonPretty(doc, out);
  return out;
}

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Connected. IP: ");
  Serial.println(WiFi.localIP());
}

void setupRoutes() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(200, "text/plain",
      "ESP32-C3 Super Mini demo\n"
      "GET /health\n"
      "GET /info\n"
    );
  });

  server.on("/health", HTTP_GET, [](AsyncWebServerRequest* request) {
    JsonDocument doc;
    doc["ok"] = true;
    doc["ip"] = WiFi.localIP().toString();
    doc["mac"] = WiFi.macAddress();
    doc["uptime_ms"] = millis();
    doc["time"] = timeString();

    String out;
    serializeJson(doc, out);
    request->send(200, "application/json", out);
  });

  server.on("/info", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(200, "application/json", makeJson());
  });

  server.onNotFound([](AsyncWebServerRequest* request) {
    request->send(404, "application/json", "{\"ok\":false,\"error\":\"not_found\"}");
  });
}

void setup() {
  delay(2000);

  Serial.begin(115200);
  delay(500);

  Serial.println();
  Serial.println("Booting ESP32-C3 Super Mini...");

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  connectWiFi();

  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
  Serial.print("NTP time: ");
  Serial.println(timeString());

  setupRoutes();
  server.begin();

  Serial.println("Async web server started");
}

void loop() {
  unsigned long now = millis();

  if (now - lastBlinkMs >= 1000) {
    lastBlinkMs = now;
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState ? HIGH : LOW);

    Serial.print("Ping ");
    Serial.print(WiFi.localIP());
    Serial.print(" ");
    Serial.println(timeString());
  }
}