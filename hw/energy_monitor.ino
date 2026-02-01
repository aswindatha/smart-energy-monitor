#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <PZEM004Tv30.h>

// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// MQTT settings
const char* mqtt_server = "YOUR_MQTT_BROKER";
const int mqtt_port = 1883;
const char* mqtt_topic = "smartenergy/data";

// Pin definitions
#define PZEM_SERIAL_RX 16
#define PZEM_SERIAL_TX 17
#define RELAY_PIN 2
#define LED_PIN 4

// Hardware objects
HardwareSerial pzemSerial(2);
PZEM004Tv30 pzem(&pzemSerial, PZEM_SERIAL_RX, PZEM_SERIAL_TX);
WiFiClient espClient;
PubSubClient client(espClient);

// Energy data structure
struct EnergyData {
  float voltage;
  float current;
  float power;
  float energy;
  float frequency;
  float powerFactor;
  bool relayState;
  unsigned long timestamp;
};

EnergyData currentData;

void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  
  // Initialize relay to OFF
  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
  
  setupWiFi();
  setupMQTT();
  
  Serial.println("Smart Energy Monitor Initialized");
}

void loop() {
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();
  
  // Read sensor data
  readEnergyData();
  
  // Send data via MQTT
  publishEnergyData();
  
  // Control LED based on power consumption
  if (currentData.power > 1000) {
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
  }
  
  delay(5000); // Send data every 5 seconds
}

void setupWiFi() {
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("Connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void setupMQTT() {
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqttCallback);
}

void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    if (client.connect("ESP32_Energy_Monitor")) {
      Serial.println("connected");
      client.subscribe("smartenergy/control");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  // Parse control commands
  DynamicJsonDocument doc(256);
  deserializeJson(doc, message);
  
  if (doc.containsKey("relay")) {
    bool relayState = doc["relay"];
    digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);
    currentData.relayState = relayState;
  }
}

void readEnergyData() {
  currentData.voltage = pzem.voltage();
  currentData.current = pzem.current();
  currentData.power = pzem.power();
  currentData.energy = pzem.energy();
  currentData.frequency = pzem.frequency();
  currentData.powerFactor = pzem.pf();
  currentData.relayState = digitalRead(RELAY_PIN) == HIGH;
  currentData.timestamp = millis();
  
  // Handle sensor errors
  if (isnan(currentData.voltage)) {
    currentData.voltage = 0;
  }
  if (isnan(currentData.current)) {
    currentData.current = 0;
  }
  if (isnan(currentData.power)) {
    currentData.power = 0;
  }
  if (isnan(currentData.energy)) {
    currentData.energy = 0;
  }
  if (isnan(currentData.frequency)) {
    currentData.frequency = 0;
  }
  if (isnan(currentData.powerFactor)) {
    currentData.powerFactor = 0;
  }
}

void publishEnergyData() {
  DynamicJsonDocument doc(1024);
  
  doc["voltage"] = currentData.voltage;
  doc["current"] = currentData.current;
  doc["power"] = currentData.power;
  doc["energy"] = currentData.energy;
  doc["frequency"] = currentData.frequency;
  doc["powerFactor"] = currentData.powerFactor;
  doc["relayState"] = currentData.relayState;
  doc["timestamp"] = currentData.timestamp;
  doc["deviceId"] = "ESP32_001";
  
  String jsonString;
  serializeJson(doc, jsonString);
  
  client.publish(mqtt_topic, jsonString.c_str());
  
  // Also print to serial for debugging
  Serial.print("Published: ");
  Serial.println(jsonString);
}
