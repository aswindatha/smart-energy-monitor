# ⚡ Quick Setup Guide

## 📋 Requirements

### Hardware:
- ESP32
- PZEM-004T-10A
- 5V Relay
- Isolated 5V supply

### Software:
- Arduino IDE
- Libraries: ArduinoJson, PZEM004Tv30, HTTPClient

## 🚀 5-Minute Setup

### 1. Configure WiFi
Edit `energy_monitor.ino`:
```cpp
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
```

### 2. Upload Code
- Select ESP32 board
- Upload to ESP32

### 3. Start API Server
```bash
cd api
npm install multicast-dns
npm start
```

### 4. Run Flutter App
```bash
cd flutter_app
flutter run
```

## ✅ Done!

Your system will auto-discover and work regardless of IP changes!
