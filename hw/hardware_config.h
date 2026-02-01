#ifndef HARDWARE_CONFIG_H
#define HARDWARE_CONFIG_H

// ESP32 Pin Configuration
#define PZEM_SERIAL_RX 16    // PZEM-004T RX pin
#define PZEM_SERIAL_TX 17    // PZEM-004T TX pin
#define RELAY_PIN 2          // Relay control pin
#define LED_PIN 4            // Status LED pin
#define BUTTON_PIN 0         // Manual control button

// Sensor Configuration
#define PZEM_ADDRESS 0x01    // Default PZEM-004T address
#define SAMPLE_INTERVAL 5000 // Data sampling interval (ms)

// WiFi Configuration
#define MAX_WIFI_RETRIES 10
#define WIFI_TIMEOUT 10000   // Connection timeout (ms)

// MQTT Configuration
#define MQTT_KEEPALIVE 60
#define MQTT_QOS 1
#define MQTT_RETAIN false

// Energy Thresholds
#define HIGH_POWER_THRESHOLD 1000.0  // Watts
#define LOW_POWER_THRESHOLD 100.0     // Watts
#define OVERVOLTAGE_THRESHOLD 250.0   // Volts
#define UNDERVOLTAGE_THRESHOLD 180.0  // Volts

// Data Validation
#define MIN_VOLTAGE 0.0
#define MAX_VOLTAGE 300.0
#define MIN_CURRENT 0.0
#define MAX_CURRENT 100.0
#define MIN_POWER 0.0
#define MAX_POWER 50000.0

#endif
