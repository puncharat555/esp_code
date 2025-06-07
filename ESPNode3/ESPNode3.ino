#include <WiFi.h>
#include <HTTPClient.h>
#include <LoRa.h>
#include <ArduinoJson.h>  // ใช้จัดการ JSON

const char* ssid = "ESP";
const char* password = "00000000";
const char* serverUrl = "http://172.20.10.4:3000/distance";

// กำหนด Node ID ของ ESP2
#define NODE_ID "ESP3"

#define LORA_SS    5
#define LORA_RST   14
#define LORA_DIO0  26

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi connected");

  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(433E6)) {
    Serial.println("❌ LoRa init failed!");
    while (1);
  }
  LoRa.setSpreadingFactor(12);
  Serial.println("📡 LoRa Receiver ready");
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize > 0) {
    String incoming = "";
    while (LoRa.available()) {
      incoming += (char)LoRa.read();
    }

    Serial.println("📩 Received: " + incoming);

    // แปลง JSON
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, incoming);
    if (error) {
      Serial.println("❌ JSON parse failed");
      return;
    }

    const char* dest = doc["destination"];
    const char* id = doc["id"];
    float distance = doc["distance"];
    int hop = doc["hop"];

    Serial.printf("📦 Message from %s, distance: %.2f, to: %s\n", doc["source"].as<const char*>(), distance, dest);

    // ถ้าเราเป็นปลายทาง (เช่น ESP2 → Server)
    if (strcmp(dest, NODE_ID) == 0) {
      if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(serverUrl);
        http.addHeader("Content-Type", "application/json");

        String jsonOut;
        serializeJson(doc, jsonOut);  // ส่งข้อมูลทั้งหมด

        int httpResponseCode = http.POST(jsonOut);
        Serial.println(httpResponseCode > 0 ? "✅ Sent to server" : "❌ HTTP POST Failed");

        http.end();
      }
    } else {
      // ไม่ใช่ปลายทาง → ส่งต่อไป LoRa (เพิ่ม hop)
      doc["hop"] = hop + 1;

      String forwardMsg;
      serializeJson(doc, forwardMsg);

      Serial.println("🔁 Forwarding to next hop: " + forwardMsg);
      LoRa.beginPacket();
      LoRa.print(forwardMsg);
      LoRa.endPacket();
    }

    delay(1000); // กันลูปเร็วเกิน
  }
}
