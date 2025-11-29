# 🌍 TerraSono  
### **A Sustainable, Modular Edge-AI Acoustic Intelligence System for Forest & Marine Habitats**

TerraSono is a **solar-powered, low-power, reconfigurable acoustic monitoring platform** designed to protect **forests, wildlife, and marine ecosystems** using **Edge AI + LoRa + Cloud analytics**.

---

## 📘 Project Overview

TerraSono gives nature a digital voice.

It detects harmful or suspicious acoustic events such as:

- 🔊 **Chainsaw** → Illegal logging  
- 🔫 **Gunshot** → Poaching  
- 🚙 **Vehicle noise** → Unauthorized intrusion  
- 🌊 *Future:* Marine noise anomalies (hydrophone module)

On-device TinyML running on the **ESP32-S3** performs real-time classification and sends only meaningful events via **LoRa** to a gateway.  
The gateway pushes data to the cloud for visualization and alerts.

### **Why TerraSono?**

- ♻️ **Sustainable** – Solar charging, recyclable enclosure, long lifetime  
- 🔌 **Low Power** – Runtime of days to weeks  
- 🔄 **Modular** – Swap MEMS mic ↔ hydrophone in seconds  
- 🛰️ **Decentralized** – Multi-node LoRa field deployment  
- 📊 **Cloud-Ready** – MQTT → FlowFuse → Node-RED → InfluxDB → Grafana  

---

## 🛰️ System Architecture

### 🔊 **1. Acoustic Environment**
Forest sounds / marine sounds → chainsaw, gunshot, vehicle, marine noise

### 🎙️ **2. TerraSono Slave Node (Edge AI + LoRa TX)**
- ESP32-S3  
- INMP441 / Hydrophone  
- Edge Impulse TinyML  
- LoRa SX1262 (Transmit)  
- SD Card Logging  

### 📡 **3. TerraSono Gateway Node (LoRa RX → WiFi)**
- ESP32-S3  
- Receives LoRa JSON  
- Publishes MQTT packets  

### ☁️ **4. Cloud Pipeline**
- **FlowFuse MQTT Broker**  
- **Node-RED** (processing + InfluxDB write)  
- **InfluxDB v2** (time-series storage)  

### 📊 **5. Grafana Dashboard**
- Real-time alerts  
- Event table  
- Node health  
- Confidence timelines  
- Battery trends  


---

## 🔧 Hardware Used

### **Core Electronics**
- ESP32-S3  
- SX1262 LoRa transceiver  
- INMP441 I²S MEMS microphone  
- PCM1808 ADC + Piezo Hydrophone (marine prototype)  
- 18650 Li-ion cell (3.4Ah)  
- BQ25185 solar charger IC  
- 6V / 100mA solar panel  
- SD card module (audio snapshot logging)

### **Enclosures**
- IP65 forest housing  
- Marine floating buoy (in development)  
- Modular M8 connector for sensor swapping  

---

## 🧠 Edge AI Model (Edge Impulse)

The TinyML classifier detects:

- **Chainsaw**  
- **Gunshot**  
- **Vehicle engine**  
- **Ambient forest sounds**  

### Training Workflow:
1. Audio data collected using TerraSono node  
2. Uploaded to **Edge Impulse Studio**  
3. MFCC audio features  
4. CNN classifier  
5. Continuous inference enabled  
6. Exported as Arduino library  
7. Integrated into ESP32-S3 firmware  

---

## 📡 Communication Pipeline

### **Slave Node → Gateway (LoRa)**
Slave node runs inference every ~100ms.  
On detection above threshold, it sends JSON:

```json
{
  "node_id": "TS001",
  "event": "chainsaw",
  "confidence": 0.92,
  "timestamp": 1730899000
}

