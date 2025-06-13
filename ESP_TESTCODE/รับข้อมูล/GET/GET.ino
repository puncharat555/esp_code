#include <LoRa.h>

#define LORA_SS    5
#define LORA_RST   14
#define LORA_DIO0  26

void setup() {
  Serial.begin(115200);
  delay(1000);

  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

  if (!LoRa.begin(433E6)) {
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

    Serial.print("Received message: ");
    Serial.println(incoming);
    Serial.print("RSSI: ");
    Serial.println(rssi);
    Serial.println("------------------------");
  }
}
