#include <LoRa.h>
#include <SPI.h>

// LoRa1 - Receiver (VSPI)
#define LORA1_SS    5
#define LORA1_RST   14
#define LORA1_DIO0  26

// LoRa2 - Transmitter (HSPI)
#define LORA2_SS    15
#define LORA2_RST   33
#define LORA2_DIO0  32
#define LORA2_SCK   25
#define LORA2_MISO  27
#define LORA2_MOSI  4

SPIClass LoRa2SPI(HSPI); 

void setup() {
  Serial.begin(115200);
  delay(1000);

  // เริ่ม LoRa1 (Receiver)
  LoRa.setPins(LORA1_SS, LORA1_RST, LORA1_DIO0);
  if (!LoRa.begin(433200000)) {
    Serial.println("LoRa1 init failed");
    while (1);
  }
  LoRa.setSpreadingFactor(12);
  Serial.println("LoRa1 (Receiver) ready");

  // เริ่ม LoRa2 (Transmitter)
  LoRa2SPI.begin(LORA2_SCK, LORA2_MISO, LORA2_MOSI, LORA2_SS);
  LoRa.setSPI(LoRa2SPI);
  LoRa.setPins(LORA2_SS, LORA2_RST, LORA2_DIO0);
  if (!LoRa.begin(433100000)) {
    Serial.println("LoRa2 init failed");
    while (1);
  }
  LoRa.setSpreadingFactor(12);
  Serial.println("LoRa2 (Transmitter) ready");


}

void loop() {
  //LoRa1 รับข้อมูล
  LoRa.setSPI(SPI);  // ตั้ง SPI กลับไป VSPI
  LoRa.setPins(LORA1_SS, LORA1_RST, LORA1_DIO0);

  int packetSize = LoRa.parsePacket();
  if (packetSize > 0) {
    long rssi = LoRa.packetRssi();
    String incoming = "";
    while (LoRa.available()) {
      incoming += (char)LoRa.read();
    }

    Serial.print("Received message: ");
    Serial.println(incoming);
    Serial.print("RSSI: ");
    Serial.println(rssi);
    Serial.println("------------------------");

    // ส่งต่อผ่าน LoRa2
    LoRa.setSPI(LoRa2SPI);  // ตั้ง SPI ไป HSPI
    LoRa.setPins(LORA2_SS, LORA2_RST, LORA2_DIO0);

    LoRa.beginPacket();
    LoRa.print(incoming);
    LoRa.endPacket();

    Serial.println("Forwarded message via LoRa2");
  }

  delay(200);
}
