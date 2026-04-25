#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

#define LED_PIN 8

struct WiFiCred {
  const char* ssid;
  const char* pass;
};

WiFiCred WIFI_LIST[] = {
  {"HomeWiFi", "home-password"},
  {"PhoneHotspot", "hotspot-password"},
  {"OfficeWiFi", "office-password"},
};


AsyncWebServer server(80);

const int WIFI_COUNT = sizeof(WIFI_LIST) / sizeof(WIFI_LIST[0]);
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

bool connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
  delay(300);

  Serial.println("Scanning WiFi...");
  int n = WiFi.scanNetworks();

  if (n <= 0) {
    Serial.println("No WiFi networks found");
    return false;
  }

  for (int i = 0; i < WIFI_COUNT; i++) {
    for (int j = 0; j < n; j++) {
      String foundSsid = WiFi.SSID(j);

      if (foundSsid == WIFI_LIST[i].ssid) {
        Serial.print("Trying WiFi: ");
        Serial.println(WIFI_LIST[i].ssid);

        WiFi.begin(WIFI_LIST[i].ssid, WIFI_LIST[i].pass);

        unsigned long started = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - started < 15000) {
          delay(500);
          Serial.print(".");
        }

        Serial.println();

        if (WiFi.status() == WL_CONNECTED) {
          Serial.print("Connected to: ");
          Serial.println(WIFI_LIST[i].ssid);
          Serial.print("IP: ");
          Serial.println(WiFi.localIP());
          WiFi.scanDelete();
          return true;
        }

        Serial.println("Failed, trying next known network...");
        WiFi.disconnect();
        delay(500);
      }
    }
  }

  WiFi.scanDelete();
  Serial.println("Could not connect to any known WiFi");
  return false;
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

  if (!connectWiFi()) {
    Serial.println("WiFi connection failed. Restarting in 5 sec...");
    delay(5000);
    ESP.restart();
  }

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