#include <WiFi.h>
#include <WebServer.h>
#include <config.h>
#include <cycles.h>
#include <dishwasher.h>
#include <web_server.h>

WebServer server(80);

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n🔧 ESP32 Dishwasher Booting...");

  initDishwasher();

  // Connect to WiFi
  Serial.print("📶 Connecting to "); Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  unsigned long t = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t < 15000) {
    delay(500);
    Serial.print(".");
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ Connected! IP: " + WiFi.localIP().toString());
  } else {
    Serial.println("\n❌ WiFi Failed - Starting AP fallback");
    WiFi.softAP("ESP32-Dishwasher", "12345678");
    Serial.println("📡 AP IP: 192.168.4.1");
  }

  setupWebServer(server);
  server.begin();
  Serial.println("🌐 Web server running on port 80");
}

void loop() {
  server.handleClient();
  updateWaterLevels();
  runStep();
}