#include <LoRa.h>
#include "esp_sleep.h"
#include <ArduinoJson.h>

// LoRa pins
#define LORA_SS    5
#define LORA_RST   14
#define LORA_DIO0  26

// Ultrasonic sensor pins
#define TRIG_PIN   12
#define ECHO_PIN   13

// Sleep time (20 วินาที)
#define SLEEP_TIME_US (20ULL * 1000000ULL)

// Node IDs
#define NODE_ID        "ESP1"
#define DESTINATION_ID "ESP3"

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Setup ultrasonic pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Trigger ultrasonic pulse
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Read echo time
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);  // 30ms timeout
  float distanceCm = (duration == 0) ? -1 : (duration / 2.0) * 0.0343;

  Serial.print("📏 Distance: ");
  if (distanceCm < 0) {
    Serial.println("Out of range");
  } else {
    Serial.print(distanceCm);
    Serial.println(" cm");
  }

  // Start LoRa
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(433E6)) {
    Serial.println("❌ LoRa init failed!");
    while (1);
  }
  LoRa.setSpreadingFactor(12);
  Serial.println("📡 LoRa initialized");

  // Prepare JSON data
  StaticJsonDocument<128> doc;
  doc["id"] = String(millis()); // หรือใช้ timestamp จริงถ้ามี RTC
  doc["source"] = NODE_ID;
  doc["destination"] = DESTINATION_ID;
  doc["hop"] = 0;
  doc["distance"] = distanceCm;

  char jsonBuf[128];
  serializeJson(doc, jsonBuf);

  // Send over LoRa
  Serial.print("📤 Sending: ");
  Serial.println(jsonBuf);
  LoRa.beginPacket();
  LoRa.print(jsonBuf);
  LoRa.endPacket();

  delay(100);

  Serial.println("😴 Going to deep sleep for 20 seconds...");
  esp_sleep_enable_timer_wakeup(SLEEP_TIME_US);
  esp_deep_sleep_start();
}

void loop() {
  // Not used
}
