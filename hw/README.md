# Smart Energy Hardware Module

This folder contains the ESP32 firmware for the Smart Energy IoT monitoring system.

## Hardware Components

### Required Hardware
- ESP32 Development Board
- PZEM-004T AC Energy Monitor
- 5V Relay Module
- LED (for status indication)
- Push Button (for manual control)
- Jumper Wires

### Wiring Diagram

```
ESP32    PZEM-004T
GPIO16   RX
GPIO17   TX
3.3V     VCC
GND      GND

ESP32    Relay Module
GPIO2    IN
5V       VCC
GND      GND

ESP32    LED
GPIO4    LED Anode (via 220Ω resistor)
GND      LED Cathode

ESP32    Button
GPIO0    Button Pin
GND      Button Other Pin
```

## Installation

1. Install required Arduino libraries:
   - `WiFi` (built-in)
   - `PubSubClient` by Nick O'Leary
   - `ArduinoJson` by Benoit Blanchon
   - `PZEM004Tv30` by PZEM

2. Configure WiFi and MQTT settings in `energy_monitor.ino`:
   ```cpp
   const char* ssid = "YOUR_WIFI_SSID";
   const char* password = "YOUR_WIFI_PASSWORD";
   const char* mqtt_server = "YOUR_MQTT_BROKER";
   ```

3. Upload the firmware to ESP32 using Arduino IDE.

## Features

- Real-time energy monitoring (voltage, current, power, energy)
- Power factor and frequency measurement
- Remote relay control via MQTT
- Automatic high-power detection with LED indication
- Data validation and error handling
- MQTT-based communication with UI

## MQTT Topics

- **Publish**: `smartenergy/data` - Energy monitoring data
- **Subscribe**: `smartenergy/control` - Control commands

## Data Format

### Published Data (smartenergy/data)
```json
{
  "voltage": 220.5,
  "current": 2.3,
  "power": 506.15,
  "energy": 1234.56,
  "frequency": 50.0,
  "powerFactor": 0.95,
  "relayState": false,
  "timestamp": 1234567890,
  "deviceId": "ESP32_001"
}
```

### Control Commands (smartenergy/control)
```json
{
  "relay": true
}
```

## Troubleshooting

1. **PZEM-004T not responding**: Check wiring and ensure proper power supply
2. **WiFi connection issues**: Verify SSID and password, check signal strength
3. **MQTT connection failed**: Confirm broker address and network connectivity
4. **Inaccurate readings**: Calibrate PZEM-004T if needed
