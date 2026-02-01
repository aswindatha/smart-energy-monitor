# Smart Energy IoT Project

A comprehensive smart energy monitoring system with hardware (ESP32), API server, and Flutter mobile app for real-time energy monitoring and analytics.

## Project Structure

```
windsurf-project/
├── hw/                    # Hardware components (ESP32 firmware)
│   ├── energy_monitor.ino   # Main Arduino sketch
│   ├── hardware_config.h    # Hardware configuration
│   └── README.md          # Hardware setup guide
├── flutter_app/           # Flutter Android app
│   ├── lib/
│   │   ├── main.dart
│   │   ├── providers/
│   │   │   └── energy_provider.dart
│   │   ├── services/
│   │   │   └── database_service.dart
│   │   └── screens/
│   │       └── home_screen.dart
│   └── pubspec.yaml
├── api/                  # Node.js API server
│   ├── server.js          # Main server file
│   ├── package.json       # Dependencies
│   └── README.md         # API documentation
└── README.md            # This file
```

## Features

### Hardware (ESP32)
- Real-time energy monitoring using PZEM-004T sensor
- WiFi connectivity for data transmission
- MQTT-based communication
- Relay control for device management
- LED status indicators

### API Server
- Simple REST API with no authentication
- MQTT integration for hardware communication
- SQLite database for data storage
- Real-time data streaming
- Analytics and cost calculations

### Flutter Mobile App
- Large device status switch (green when ON, red when OFF)
- Real-time energy readings display
- Analytics with electricity charges
- Daily/monthly data selection
- Configurable electricity rates
- SQLite local database
- Material Design UI

## Quick Start

### 1. Hardware Setup
1. Connect ESP32 with PZEM-004T energy monitor
2. Upload firmware from `hw/energy_monitor.ino`
3. Configure WiFi and MQTT settings
4. Power on the device

### 2. API Server Setup
```bash
cd api
npm install
npm start
```

### 3. Flutter App Setup
```bash
cd flutter_app
flutter pub get
flutter run
```

## Configuration

### Hardware Configuration
Edit `hw/energy_monitor.ino`:
```cpp
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* mqtt_server = "YOUR_MQTT_BROKER";
```

### API Configuration
Set environment variables:
```bash
export PORT=3000
export MQTT_BROKER=mqtt://localhost:1883
```

### Flutter App Configuration
Update API URL in `lib/providers/energy_provider.dart`:
```dart
final String _apiUrl = 'http://YOUR_API_IP:3000/api';
```

## Data Flow

1. **ESP32** reads energy data from PZEM-004T sensor
2. **ESP32** publishes data to MQTT topic `smartenergy/data`
3. **API Server** subscribes to MQTT and stores data in SQLite
4. **Flutter App** fetches data from REST API endpoints
5. **Flutter App** sends control commands via API to MQTT

## API Endpoints

- `GET /api/data` - Get current energy readings
- `POST /api/control` - Control relay state
- `GET /api/history` - Get historical data
- `GET /api/analytics` - Get analytics and costs

## MQTT Topics

- `smartenergy/data` - Energy data from ESP32
- `smartenergy/control` - Control commands to ESP32

## Requirements

### Hardware
- ESP32 Development Board
- PZEM-004T AC Energy Monitor
- 5V Relay Module
- LED and resistors
- Jumper wires

### Software
- Arduino IDE
- Node.js 14+
- Flutter SDK
- MQTT Broker (Mosquitto recommended)

## Installation Details

See individual README files in each folder for detailed setup instructions:
- `hw/README.md` - Hardware setup and wiring
- `api/README.md` - API server configuration
- `flutter_app/` - Flutter app development

## ESP32 Simulator

For testing without hardware, use the Python simulator:

```bash
# Install dependencies
pip install -r requirements.txt

# Run simulator
python esp32_simulator.py
```

The simulator provides:
- Realistic energy consumption patterns
- Time-based fluctuations (morning/evening peaks)
- Random spikes and dips
- MQTT and REST API endpoints
- Device control via API calls

### Simulator API Endpoints
- `GET http://localhost:5000/api/data` - Get current readings
- `POST http://localhost:5000/api/control` - Control device
- `GET http://localhost:5000/api/status` - Get simulator status
- `POST http://localhost:5000/api/reset` - Reset energy counter

### Testing with Flutter App
Update API URL in `flutter_app/lib/providers/energy_provider.dart`:
```dart
final String _apiUrl = 'http://localhost:5000/api';
```

## Usage

1. Power on the ESP32 device
2. Start the API server
3. Run the Flutter app on Android device
4. Toggle device switch to start monitoring
5. View real-time readings and analytics

## Security Note

This project uses a simple API without authentication for demonstration purposes. For production use, implement proper security measures including:
- API authentication
- HTTPS encryption
- MQTT security
- Input validation
- Rate limiting
