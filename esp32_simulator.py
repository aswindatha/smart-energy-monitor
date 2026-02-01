#!/usr/bin/env python3
"""
ESP32 Energy Data Simulator
Simulates realistic energy consumption patterns with fluctuations
"""

import json
import time
import random
import math
import threading
from datetime import datetime, timedelta
import paho.mqtt.client as mqtt
from flask import Flask, jsonify, request

# Configuration
MQTT_BROKER = "localhost"
MQTT_PORT = 1883
MQTT_TOPIC_DATA = "smartenergy/data"
MQTT_TOPIC_CONTROL = "smartenergy/control"
API_PORT = 5000

# Simulation parameters
BASE_VOLTAGE = 230.0  # Base voltage in volts
BASE_CURRENT = 2.0      # Base current in amps
BASE_POWER = 460.0      # Base power in watts
FREQUENCY = 50.0         # Frequency in Hz
POWER_FACTOR = 0.95      # Power factor

# Device state
device_on = False
relay_state = False
total_energy = 0.0  # Cumulative energy in kWh
simulation_running = True

# Flask app for API endpoints
app = Flask(__name__)

class EnergySimulator:
    def __init__(self):
        self.time_counter = 0
        self.cycle_time = 0
        self.pattern_phase = 0
        self.base_values = {
            'voltage': BASE_VOLTAGE,
            'current': BASE_CURRENT,
            'power': BASE_POWER
        }
        
    def calculate_realistic_values(self):
        """Generate realistic energy consumption patterns"""
        current_time = time.time()
        
        # Create different consumption patterns throughout the day
        hour = datetime.now().hour
        
        # Base consumption varies by time of day
        if 6 <= hour < 9:      # Morning peak
            multiplier = 1.3 + 0.2 * math.sin(self.time_counter * 0.1)
        elif 9 <= hour < 17:     # Daytime
            multiplier = 0.8 + 0.1 * math.sin(self.time_counter * 0.05)
        elif 17 <= hour < 22:    # Evening peak
            multiplier = 1.5 + 0.3 * math.sin(self.time_counter * 0.08)
        else:                     # Night
            multiplier = 0.4 + 0.05 * math.sin(self.time_counter * 0.02)
        
        # Add realistic fluctuations
        voltage_fluctuation = random.gauss(0, 2.0)  # ±2V typical fluctuation
        current_fluctuation = random.gauss(0, 0.1)   # ±0.1A fluctuation
        
        # Calculate values
        voltage = self.base_values['voltage'] * (1 + voltage_fluctuation / 100)
        current = self.base_values['current'] * multiplier * (1 + current_fluctuation / 100)
        power = voltage * current * POWER_FACTOR
        
        # Add occasional spikes (simulating appliance startup)
        if random.random() < 0.02:  # 2% chance of spike
            power *= random.uniform(1.5, 2.5)
            current *= random.uniform(1.5, 2.5)
        
        # Add occasional dips (simulating appliance shutdown)
        if random.random() < 0.01:  # 1% chance of dip
            power *= random.uniform(0.3, 0.7)
            current *= random.uniform(0.3, 0.7)
        
        # Ensure realistic ranges
        voltage = max(200, min(250, voltage))
        current = max(0.1, min(10, current))
        power = max(20, min(2500, power))
        
        # Calculate energy accumulation (kWh)
        time_delta = 5 / 3600  # 5 seconds in hours
        energy_increment = (power / 1000) * time_delta
        global total_energy
        total_energy += energy_increment
        
        # Vary power factor slightly
        power_factor = POWER_FACTOR + random.gauss(0, 0.02)
        power_factor = max(0.8, min(1.0, power_factor))
        
        return {
            'voltage': round(voltage, 1),
            'current': round(current, 2),
            'power': round(power, 1),
            'energy': round(total_energy, 3),
            'frequency': round(FREQUENCY + random.gauss(0, 0.1), 1),
            'powerFactor': round(power_factor, 2),
            'relayState': relay_state,
            'timestamp': int(current_time * 1000),
            'deviceId': 'ESP32_SIM_001'
        }
    
    def update_cycle(self):
        """Update simulation cycle counter"""
        self.time_counter += 1
        self.cycle_time += 5  # 5 second intervals
        
        # Reset cycle every 24 hours (simulated)
        if self.cycle_time >= 86400:  # 24 hours in seconds
            self.cycle_time = 0
            self.pattern_phase = (self.pattern_phase + 1) % 4

# MQTT setup
def on_connect(client, userdata, flags, rc):
    print(f"Connected to MQTT broker with result code {rc}")
    client.subscribe(MQTT_TOPIC_CONTROL)

def on_message(client, userdata, msg):
    global relay_state, device_on
    try:
        message = json.loads(msg.payload.decode())
        if 'relay' in message:
            relay_state = message['relay']
            device_on = relay_state
            print(f"Relay state changed to: {relay_state}")
    except Exception as e:
        print(f"Error processing MQTT message: {e}")

# Initialize MQTT client
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

try:
    client.connect(MQTT_BROKER, MQTT_PORT, 60)
    client.loop_start()
    print("MQTT client started")
except Exception as e:
    print(f"Could not connect to MQTT broker: {e}")
    print("Continuing without MQTT...")

# Initialize simulator
simulator = EnergySimulator()

def publish_data():
    """Publish energy data via MQTT"""
    if device_on:
        data = simulator.calculate_realistic_values()
        try:
            client.publish(MQTT_TOPIC_DATA, json.dumps(data))
            print(f"Published: {data['power']}W, {data['current']}A, {data['voltage']}V")
        except Exception as e:
            print(f"Error publishing MQTT data: {e}")

# API endpoints
@app.route('/api/data', methods=['GET'])
def get_data():
    """Get current energy data"""
    if device_on:
        data = simulator.calculate_realistic_values()
        return jsonify(data)
    else:
        return jsonify({
            'voltage': 0,
            'current': 0,
            'power': 0,
            'energy': total_energy,
            'frequency': 0,
            'powerFactor': 0,
            'relayState': False,
            'timestamp': int(time.time() * 1000),
            'deviceId': 'ESP32_SIM_001'
        })

@app.route('/api/control', methods=['POST'])
def control_relay():
    """Control relay state"""
    global relay_state, device_on
    data = request.get_json()
    
    if 'relay' in data:
        relay_state = bool(data['relay'])
        device_on = relay_state
        
        # Also publish via MQTT if available
        try:
            client.publish(MQTT_TOPIC_CONTROL, json.dumps({'relay': relay_state}))
        except:
            pass
            
        return jsonify({'success': True, 'relay': relay_state})
    else:
        return jsonify({'error': 'Invalid request'}), 400

@app.route('/api/status', methods=['GET'])
def get_status():
    """Get simulator status"""
    return jsonify({
        'device_on': device_on,
        'relay_state': relay_state,
        'total_energy': total_energy,
        'simulation_running': simulation_running,
        'mqtt_connected': client.is_connected()
    })

@app.route('/api/reset', methods=['POST'])
def reset_energy():
    """Reset energy counter"""
    global total_energy
    total_energy = 0.0
    return jsonify({'success': True, 'total_energy': total_energy})

def simulation_loop():
    """Main simulation loop"""
    print("Starting ESP32 simulation...")
    print("Device is OFF. Use API or MQTT to turn ON")
    print(f"API available at: http://localhost:{API_PORT}")
    print(f"MQTT topics: {MQTT_TOPIC_DATA} (data), {MQTT_TOPIC_CONTROL} (control)")
    
    while simulation_running:
        try:
            # Update simulator
            simulator.update_cycle()
            
            # Publish data if device is on
            publish_data()
            
            # Wait 5 seconds
            time.sleep(5)
            
        except KeyboardInterrupt:
            print("\nSimulation stopped by user")
            break
        except Exception as e:
            print(f"Error in simulation loop: {e}")
            time.sleep(5)

# Start simulation in background thread
sim_thread = threading.Thread(target=simulation_loop, daemon=True)
sim_thread.start()

# Start Flask API
if __name__ == '__main__':
    try:
        print(f"Starting API server on port {API_PORT}")
        app.run(host='0.0.0.0', port=API_PORT, debug=False, use_reloader=False)
    except KeyboardInterrupt:
        print("\nShutting down simulator...")
        simulation_running = False
        client.loop_stop()
        client.disconnect()
