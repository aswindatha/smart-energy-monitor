#ifndef HARDWARE_CONFIG_H
#define HARDWARE_CONFIG_H

// ESP32 Pin Configuration
#define PZEM_SERIAL_RX 16    // PZEM-004T RX pin
#define PZEM_SERIAL_TX 17    // PZEM-004T TX pin
#define RELAY_PIN 25         // Relay control pin (corrected from GPIO2)

// WiFi AP Configuration
#define AP_SSID "smart energy meter"
#define AP_PASSWORD "12345678"
#define AP_CHANNEL 1
#define AP_MAX_CONNECTIONS 4

// Web Server Configuration
#define WEB_SERVER_PORT 80
#define REFRESH_INTERVAL 2000  // Auto-refresh every 2 seconds

// PZEM Configuration
#define PZEM_BAUD_RATE 9600
#define PZEM_SERIAL_MODE SERIAL_8N1

// Safety Thresholds
#define MAX_CURRENT 10.0       // Maximum current for PZEM-004T-10A
#define MAX_POWER 2500.0       // Maximum power at 250V, 10A

#endif
