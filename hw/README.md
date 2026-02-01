# 🚀 Smart Energy ESP32 - mDNS Auto-Discovery

## 📱 What This Does

**ESP32 automatically discovers and connects to your API server using mDNS (Bonjour)** - no IP configuration needed!

### 🌐 Architecture
```
ESP32 (esp32-energy.local) 
   ↓ HTTP POST (auto-discovered)
API Server (energy-api.local)
   ↓ HTTP GET/POST  
Flutter App (auto-discovers API)
```

## ⚙️ Configuration Required

### 1. WiFi Only
```cpp
const char* ssid = "YOUR_WIFI_SSID";        // Your WiFi network
const char* password = "YOUR_WIFI_PASSWORD"; // Your WiFi password
```

### 2. Install Libraries
- **ESPmDNS** (built-in)
- **ArduinoJson**
- **PZEM004Tv30**
- **HTTPClient**

## 🔄 How It Works

### Auto-Discovery Process:
1. **ESP32 boots** → registers as `esp32-energy.local`
2. **API server boots** → registers as `energy-api.local`
3. **ESP32 discovers API** → finds IP via mDNS
4. **Data flows** → ESP32 → API → Flutter App

### Data Flow:
- **ESP32** reads PZEM every 3 seconds
- **HTTP POST** to discovered API server
- **Flutter app** gets data from API server
- **Control commands** via HTTP to ESP32

## 📊 Endpoints

### ESP32 Endpoints:
- `GET http://esp32-energy.local/` - Status
- `POST http://esp32-energy.local/control` - Relay control

### API Server Endpoints:
- `GET http://energy-api.local/api/data` - Current data
- `POST http://energy-api.local/api/control` - Control ESP32

## 🚀 Setup Steps

1. **Update WiFi credentials** in `energy_monitor.ino`
2. **Install required libraries**
3. **Upload to ESP32**
4. **Start API server** (`npm install multicast-dns`)
5. **Run Flutter app**

## ✨ Benefits

- **Zero IP configuration**
- **Works with dynamic IPs**
- **Plug-and-play setup**
- **Professional local deployment**

**Your smart energy system now works regardless of IP changes!** 🎉
