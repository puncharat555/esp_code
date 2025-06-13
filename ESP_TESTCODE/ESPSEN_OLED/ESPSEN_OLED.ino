#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED config
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// LoRa config
#define LORA_SS    5
#define LORA_RST   14
#define LORA_DIO0  26

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C typical
    Serial.println("OLED not found");
    while (true);
  }

  display.clearDisplay();
  display.setTextSize(1);      // ขนาดตัวอักษร
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("LoRa Receiver...");
  display.display();

  // Init LoRa
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(433100000)) {
    Serial.println("Starting LoRa failed!");
    display.println("LoRa failed!");
    display.display();
    while (1);
  }

  LoRa.setSpreadingFactor(12);
  Serial.println("LoRa ready");
  display.println("LoRa Ready ");
  display.display();
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize > 0) {
    long rssi = LoRa.packetRssi();
    String incoming = "";

    while (LoRa.available()) {
      incoming += (char)LoRa.read();
    }

    Serial.println("Received: " + incoming);
    Serial.println("RSSI: " + String(rssi));

    // Show on OLED
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Msg: ");
    display.println(incoming);
    display.print("RSSI: ");
    display.println(rssi);
    display.display();
  }
}
