#include <RadioLib.h>
#include <SPI.h>

// ===== LoRa Pin Mapping =====
#define LORA_CS    4
#define LORA_RST   3
#define LORA_BUSY  15
#define LORA_DIO1  16
#define LORA_SCK   7
#define LORA_MISO  8
#define LORA_MOSI  9

// ===== LoRa Module =====
SPIClass loraSPI(FSPI);   // use FSPI peripheral
SX1262 radio = new Module(LORA_CS, LORA_DIO1, LORA_RST, LORA_BUSY, loraSPI);

// ===== Event List =====
String events[] = {"Gunshot", "Chainsaw", "Dolphin Sound", "Vehicle", "Logging"};

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // Initialize custom SPI
  loraSPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);

  Serial.println("SX1262 init test...");

  int state = radio.begin(866.0);   // India LoRa band 865–867 MHz
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println("SX1262 initialized successfully!");
  } else {
    Serial.print("SX1262 init failed, code ");
    Serial.println(state);
  }

  // Seed random generator
  randomSeed(analogRead(0));
}

void loop() {
  // 1️⃣ Simulate event detection
  int eventIndex = random(0, 5);
  String event_type = events[eventIndex];

  Serial.println("Event detected: " + event_type);

  // 2️⃣ Build LoRa payload (simple string)
  String payload = event_type;

  Serial.println("Sending event over LoRa...");
Serial.println("Event logged in SD Card...");
  int state = radio.transmit(payload);

  if (state == RADIOLIB_ERR_NONE) {
    Serial.println("Event sent successfully!");
  } else {
    Serial.print("Transmission failed, code ");
    Serial.println(state);
  }

  Serial.println("Going to sleep \n");
  delay(10000);
}
