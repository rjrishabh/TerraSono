#include <RadioLib.h>
#include <SPI.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

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

// ===== WiFi + MQTT Settings =====
const char* ssid = "Airtel_rish_1320";
const char* password = "Heisenberg";
const char* mqtt_server = "broker.flowfuse.cloud";

const char* mqtt_client_id = "esp32@BJNrDWnLdD";
const char* mqtt_username = "esp32@BJNrDWnLdD";
const char* mqtt_password = "esp32";

const char* mqtt_topic_events = "terrasono/events";

WiFiClient espClient;
PubSubClient client(espClient);

// ===== MQTT Connect =====
void setup_wifi() {
  Serial.print("Connecting to WiFi: ");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect(mqtt_client_id, mqtt_username, mqtt_password)) {
      Serial.println("connected!");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" retry in 5s");
      delay(5000);
    }
  }
}

// ===== Setup =====
void setup() {
  Serial.begin(115200);
  while(!Serial);

  // ===== WiFi =====
  setup_wifi();
  client.setServer(mqtt_server, 1883);

  // ===== LoRa =====
  loraSPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);
  Serial.println("Initializing SX1262 LoRa...");
  int state = radio.begin(866.0); // India LoRa band
  if(state == RADIOLIB_ERR_NONE) {
    Serial.println("SX1262 initialized!");
  } else {
    Serial.print("LoRa init failed, code ");
    Serial.println(state);
  }

  randomSeed(analogRead(0));
}

// ===== Loop =====
void loop() {
  if(!client.connected()) reconnect();
  client.loop();


  String str;
  int state = radio.receive(str);   // non-blocking or short timeout
  if(state == RADIOLIB_ERR_NONE){
    Serial.print("Received via LoRa: ");
    Serial.println(str);

    // Build JSON
    String payload = "{";
    payload += "\"node_id\":\"1\",";
    payload += "\"location\":\"Marine_zone_3\",";
    payload += "\"event\":\"" + str + "\",";
    payload += "\"battery\":90,";        // example
    payload += "\"nodes_active\":15,";
    payload += "\"total_events\":347,";
    payload += "\"lat\":45.4215,";
    payload += "\"lon\":-75.6972,";
    payload += "\"ts\":\"2025-09-26T10:30:00Z\"";
    payload += "}";

    client.publish("terrasono/events", payload.c_str());
    Serial.println("Published to MQTT: " + payload);
  } 
  else if(state != RADIOLIB_ERR_RX_TIMEOUT){
    Serial.print("LoRa receive error: ");
    Serial.println(state);
  }

  delay(100); // short delay, keeps loop responsive
}

