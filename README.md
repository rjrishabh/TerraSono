🌍 TerraSono
A Sustainable, Modular Edge-AI Acoustic Intelligence System for Forest & Marine Habitats
TerraSono is a solar-powered, low-power, reconfigurable acoustic monitoring platform designed to protect forests, wildlife, and marine ecosystems using Edge AI, LoRa communication, and cloud-based analytics.

🌱 Project Overview
TerraSono gives nature a voice.
It detects harmful and suspicious acoustic events such as:
🔊 Chainsaw sounds → Illegal logging
🔫 Gunshots → Poaching
🚙 Vehicle activity → Intrusion
🌊 Future: Marine acoustic anomalies using hydrophones
Using Edge AI running directly on the ESP32-S3, the system performs real-time audio classification at the edge, sends only meaningful events over LoRa, and logs everything into the cloud for analysis.

TerraSono is designed as:
♻️ Sustainable – Solar-powered, repairable, recyclable enclosure
🔌 Low-Power – Days to weeks of runtime on 18650 battery
🔁 Modular – Swap between MEMS mic (forest) and hydrophone (marine)
🗂️ Decentralized – Multi-node LoRa mesh to gateway
☁️ Cloud-Enabled – MQTT → Node-RED → InfluxDB → Grafana

🛰️ System Architecture
[Forest / Marine Sounds]
        ↓
[TerraSono Slave Node]
  - INMP441 MEMS Mic (Forest)
  - Hydrophone (Marine, in progress)
  - ESP32-S3 + Edge Impulse TinyML
  - LoRa SX1262 TX
        ↓
[TerraSono Gateway Node]
  - LoRa RX
  - ESP32-S3 WiFi
  - MQTT Publisher
        ↓
[FlowFuse MQTT Broker]
        ↓
[Node-RED]
        ↓
[InfluxDB]
        ↓
[Grafana Dashboard]

🔧 Hardware Used
Core Modules
ESP32-S3 Microcontroller
SX1262 LoRa Module
INMP441 I²S MEMS Microphone (forest sensing)
18650 Li-ion Cell (3.4Ah)
BQ25185 Li-ion Solar Charger
6V / 100mA Solar Panel
SD Card for data logging
Enclosures
IP65 forest enclosure

Marine floating buoy module (in development)
Modular connectors (M8) for easy mic swapping

🧠 AI Model (Edge Impulse)

The TinyML model detects:
Chainsaw
Gunshot
Vehicle
Ambient / Normal Forest Noise

Model Training Pipeline:
Dataset collected using TerraSono node
Data uploaded to Edge Impulse Studio
MFCC feature extraction
Classification model (CNN)
Continuous inference enabled
Model exported as Arduino library
Integrated into ESP32-S3 firmware

📡 Communication Flow
Slave Node
Runs Edge AI
Detects events
Creates JSON:

{
  "node_id": "TS001",
  "event": "chainsaw",
  "confidence": 0.92,
  "timestamp": 1730899000
}

Sends JSON over LoRa
Gateway Node
Receives LoRa JSON
Parses and uploads to MQTT broker using WiFi

☁️ Cloud & Dashboard Setup
1. MQTT (FlowFuse)
Topic: terrasano/events
JSON payload format (above)

2. Node-RED
MQTT In → JSON → InfluxDB Out

3. InfluxDB
Bucket: terrasano_data
Retention: 30 days
Measurement: acoustic_events

4. Grafana Dashboard

Panels:
Real-time event stream
Event heatmap
Node activity map
Battery trends
Timestamped event list

🔋 Power Calculations (Demo Setup)
Battery: 18650, 3.4Ah
Avg device current (inference + LoRa TX duty-cycle limited): 40–60mA
Estimated runtime: ~48–72 hours continuous
Solar input: 6V @ 100mA → ~0.6W
Daytime charging restores ~20–30% battery per hour of sun
Designed for multi-week deployment


 had excellent build quality, precise soldering, and on-time delivery.
