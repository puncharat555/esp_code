#include <LoRa.h>
#include "esp_sleep.h"

// LoRa pins
#define LORA_SS    5
#define LORA_RST   14
#define LORA_DIO0  26

// Ultrasonic HC-SR04 pins
#define TRIG_PIN   12
#define ECHO_PIN   13

// Deep Sleep time: 20 seconds (in microseconds)
#define SLEEP_TIME_US   (20ULL * 1000000ULL)

void setup() {
  Serial.begin(115200);
  delay(1000);

  // ตรวจ wake-up cause
  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER) {
    Serial.println("Woke up from deep sleep");
  } else {
    Serial.println("Booting normally");
  }

  // ตั้งค่า Ultrasonic pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // วัดระยะ
  long duration;
  float distanceCm;
  
  // ส่ง pulse
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // อ่านเวลาสัญญาณสะท้อนกลับ
  duration = pulseIn(ECHO_PIN, HIGH, 30000);  // timeout 30ms
  if (duration == 0) {
    distanceCm = -1;  // เกินขอบเขตหรือตรวจไม่พบ
  } else {
    distanceCm = (duration / 2.0) * 0.0343;  // ระยะทาง = (เวลา / 2) * ความเร็วเสียง (cm/µs)
  }

  Serial.print("Measured distance: ");
  if (distanceCm < 0) {
    Serial.println("Out of range");
  } else {
    Serial.print(distanceCm);
    Serial.println(" cm");
  }

  // เริ่มต้น LoRa
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(433200000 )) { //channel 1 = 433.2 MHz
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  LoRa.setSpreadingFactor(12);
  Serial.println("LoRa ready");

  // สร้างข้อความส่ง
  char buf[64];
  if (distanceCm < 0) {
    snprintf(buf, sizeof(buf), "Distance: Out of range");
  } else {
    snprintf(buf, sizeof(buf), "Distance: %.1f cm", distanceCm);
  }

  // ส่งข้อมูลผ่าน LoRa
  Serial.print("Sending: ");
  Serial.println(buf);
  LoRa.beginPacket();
  LoRa.print(buf);
  LoRa.endPacket();

  // รอให้ Serial พิมพ์ให้หมด
  delay(100);

  Serial.println("Going to deep sleep for 20 seconds...");
  // ตั้ง timer wake-up
  esp_sleep_enable_timer_wakeup(SLEEP_TIME_US);
  esp_deep_sleep_start();
}

void loop() {
  // ว่างเปล่า: ทุกอย่างอยู่ใน setup()
}
