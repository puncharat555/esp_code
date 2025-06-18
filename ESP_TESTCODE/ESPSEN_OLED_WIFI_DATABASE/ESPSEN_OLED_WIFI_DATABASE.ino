#include <WiFi.h>
#include <HTTPClient.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// WiFi credentials
const char* ssid = "ESP";
const char* password = "00000000";

const char* serverUrl = "https://backend-water-rf88.onrender.com/distance";

// LoRa module pins
#define LORA_SS    5
#define LORA_RST   14
#define LORA_DIO0  26

// OLED config
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_SDA 21
#define OLED_SCL 22

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Initialize OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    while (1);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("Starting...");
  display.display();

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  display.clearDisplay();
  display.setCursor(0,0);
  display.println("WiFi connected");
  display.display();

  // Initialize LoRa
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(433100000)) { //Node3 433.1Mhz
    Serial.println("Starting LoRa failed!");
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("LoRa start failed");
    display.display();
    while (1);
  }
  LoRa.setSpreadingFactor(12);
  Serial.println("LoRa Receiver ready");
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("LoRa ready");
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

    Serial.println("Received LoRa Message: " + incoming);

    incoming.replace("Distance:", "");
    incoming.replace("cm", "");
    incoming.trim();

    float distance = incoming.toFloat(); 
    Serial.println("Parsed distance: " + String(distance, 2));
    Serial.println("RSSI: " + String(rssi));
    Serial.println("----------------------------");

    // แสดงผลบน OLED
    display.clearDisplay();
    display.setCursor(0,0);
    display.print("Distance: ");
    display.print(distance, 2);
    display.println(" cm");
    display.print("RSSI: ");
    display.println(rssi);
    display.display();

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
