#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <PZEM004Tv30.h>
#include <HTTPClient.h>
#include <ESPmDNS.h>
#include <DNSServer.h>

// WiFi credentials
const char* ssid = "kavitha-siva-5ghz";
const char* password = "siva1103";

// mDNS settings
const char* esp32_hostname = "esp32-energy";
const char* api_hostname = "energy-api";

// API server settings (will be discovered via mDNS)
String api_server_url = "";

// Pin definitions
#define PZEM_SERIAL_RX 16
#define PZEM_SERIAL_TX 17
#define RELAY_PIN 25

// Hardware objects
HardwareSerial pzemSerial(2);
PZEM004Tv30 pzem(&pzemSerial, PZEM_SERIAL_RX, PZEM_SERIAL_TX);
WebServer server(80);
HTTPClient http;

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
unsigned long lastUpdateTime = 0;
const unsigned long updateInterval = 3000; // 3 seconds
bool api_discovered = false;

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
  if (isnan(currentData.voltage)) currentData.voltage = 0;
  if (isnan(currentData.current)) currentData.current = 0;
  if (isnan(currentData.power)) currentData.power = 0;
  if (isnan(currentData.energy)) currentData.energy = 0;
  if (isnan(currentData.frequency)) currentData.frequency = 0;
  if (isnan(currentData.powerFactor)) currentData.powerFactor = 0;
}

void discoverAPIServer() {
  Serial.println("Discovering API server via mDNS...");
  
  int n = MDNS.queryService("http", "tcp");
  for (int i = 0; i < n; ++i) {
    if (MDNS.hostname(i).equals(api_hostname)) {
      api_server_url = "http://" + MDNS.IP(i).toString() + ":3000";
      Serial.print("API server discovered at: ");
      Serial.println(api_server_url);
      api_discovered = true;
      return;
    }
  }
  
  Serial.println("API server not found, retrying...");
}

void publishEnergyData() {
  if (!api_discovered) {
    discoverAPIServer();
    if (!api_discovered) return;
  }
  
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
  
  // Send to API server via HTTP
  String apiUrl = api_server_url + "/api/data";
  http.begin(apiUrl);
  http.addHeader("Content-Type", "application/json");
  
  int httpResponseCode = http.POST(jsonString);
  
  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
  } else {
    Serial.print("Error on HTTP request: ");
    Serial.println(httpResponseCode);
    api_discovered = false; // Reset discovery on error
  }
  
  http.end();
  
  Serial.print("Published: ");
  Serial.println(jsonString);
}

void handleControl() {
  if (server.hasArg("plain")) {
    String body = server.arg("plain");
    DynamicJsonDocument doc(256);
    deserializeJson(doc, body);
    
    if (doc.containsKey("relay")) {
      bool relayState = doc["relay"];
      digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);
      currentData.relayState = relayState;
      
      DynamicJsonDocument response(128);
      response["success"] = true;
      response["relay"] = relayState;
      
      String resp;
      serializeJson(response, resp);
      server.send(200, "application/json", resp);
      
      Serial.print("Relay state changed via HTTP: ");
      Serial.println(relayState ? "ON" : "OFF");
      return;
    }
  }
  
  server.send(400, "application/json", "{\"error\":\"Invalid request\"}");
}

void handleStatus() {
  DynamicJsonDocument doc(512);
  
  doc["status"] = "online";
  doc["hostname"] = esp32_hostname;
  doc["relayState"] = currentData.relayState;
  doc["api_discovered"] = api_discovered;
  doc["api_server"] = api_server_url;
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  
  // Initialize PZEM
  pzemSerial.begin(9600, SERIAL_8N1, PZEM_SERIAL_RX, PZEM_SERIAL_TX);
  
  // Setup WiFi
  setupWiFi();
  
  // Setup mDNS
  if (!MDNS.begin(esp32_hostname)) {
    Serial.println("Error setting up MDNS responder!");
    while(1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
  
  // Add mDNS service
  MDNS.addService("http", "tcp", 80);
  
  // Setup web server
  server.on("/", handleStatus);
  server.on("/control", HTTP_POST, handleControl);
  server.begin();
  
  Serial.println("Smart Energy Monitor Initialized");
  Serial.println("Hostname: " + String(esp32_hostname) + ".local");
  Serial.println("Sending data every 3 seconds to API server");
}

void loop() {
  server.handleClient();
  
  // Send data every 3 seconds
  unsigned long currentTime = millis();
  if (currentTime - lastUpdateTime >= updateInterval) {
    readEnergyData();
    publishEnergyData();
    lastUpdateTime = currentTime;
  }
}

void setupWiFi() {
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nConnected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("mDNS: ");
  Serial.print(esp32_hostname);
  Serial.println(".local");
}
