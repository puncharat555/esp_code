#include <WiFi.h>
#include <HTTPClient.h>
#include <LoRa.h>

// WiFi credentials
const char* ssid = "ESP";
const char* password = "00000000";

const char* serverUrl = "https://backend-water-rf88.onrender.com/distance";

// LoRa module pins
#define LORA_SS    5
#define LORA_RST   14
#define LORA_DIO0  26

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  // Initialize LoRa
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(433200000)) { //433.1h
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  LoRa.setSpreadingFactor(12);
  Serial.println("LoRa Receiver ready");
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize > 0) {
    long rssi = LoRa.packetRssi();

    String incoming = "";
    while (LoRa.available()) {
      incoming += (char)LoRa.read();
    }

    Serial.println("Received LoRa Message: " + incoming);

    incoming.replace("Distance:", "");
    incoming.replace("cm", "");
    incoming.trim();

    float distance = incoming.toFloat(); 
    Serial.println("Parsed distance: " + String(distance, 2));
    Serial.println("RSSI: " + String(rssi));
    Serial.println("----------------------------");

    // ส่งข้อมูลไปยัง server
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin(serverUrl);
      http.addHeader("Content-Type", "application/json");

      String jsonData = "{\"distance\":" + String(distance, 2) + ",\"rssi\":" + String(rssi) + "}";

      int httpResponseCode = http.POST(jsonData);

      if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.println("Sent to server: " + jsonData);
        Serial.println("Server response: " + response);
      } else {
        Serial.print("Failed to POST. HTTP error: ");
        Serial.println(httpResponseCode);
      }

      http.end();
    } else {
      Serial.println("WiFi not connected");
    }

    delay(2000); 
  }
}
