const express = require('express');
const mqtt = require('mqtt');
const cors = require('cors');
const sqlite3 = require('sqlite3').verbose();
const path = require('path');

const app = express();
const PORT = process.env.PORT || 3000;

// Middleware
app.use(cors());
app.use(express.json());

// Database setup
const db = new sqlite3.Database('./energy_data.db');

// Create tables if they don't exist
db.serialize(() => {
  db.run(`
    CREATE TABLE IF NOT EXISTS energy_data (
      id INTEGER PRIMARY KEY AUTOINCREMENT,
      voltage REAL NOT NULL,
      current REAL NOT NULL,
      power REAL NOT NULL,
      energy REAL NOT NULL,
      frequency REAL NOT NULL,
      power_factor REAL NOT NULL,
      relay_state INTEGER NOT NULL,
      timestamp INTEGER NOT NULL
    )
  `);
});

// MQTT setup
const mqttBroker = process.env.MQTT_BROKER || 'mqtt://localhost:1883';
const mqttClient = mqtt.connect(mqttBroker);

mqttClient.on('connect', () => {
  console.log('Connected to MQTT broker');
  mqttClient.subscribe('smartenergy/data');
  mqttClient.subscribe('smartenergy/control');
});

mqttClient.on('message', (topic, message) => {
  if (topic === 'smartenergy/data') {
    try {
      const data = JSON.parse(message.toString());
      
      // Store in database
      db.run(
        `INSERT INTO energy_data 
         (voltage, current, power, energy, frequency, power_factor, relay_state, timestamp)
         VALUES (?, ?, ?, ?, ?, ?, ?, ?)`,
        [
          data.voltage,
          data.current,
          data.power,
          data.energy,
          data.frequency,
          data.powerFactor,
          data.relayState ? 1 : 0,
          data.timestamp || Date.now()
        ]
      );
      
      console.log('Stored energy data:', data);
    } catch (error) {
      console.error('Error processing MQTT data:', error);
    }
  }
});

// Current data storage
let currentData = null;

// API Routes

// Get current energy data
app.get('/api/data', (req, res) => {
  console.log('API: Received request for /api/data');
  if (currentData) {
    console.log('API: Returning current data:', currentData);
    res.json(currentData);
  } else {
    console.log('API: No current data, fetching from database');
    // Get latest data from database
    db.get(
      'SELECT * FROM energy_data ORDER BY timestamp DESC LIMIT 1',
      (err, row) => {
        if (err) {
          console.error('API: Database error:', err);
          res.status(500).json({ error: 'Database error' });
        } else if (row) {
          const data = {
            voltage: row.voltage,
            current: row.current,
            power: row.power,
            energy: row.energy,
            frequency: row.frequency,
            powerFactor: row.power_factor,
            relayState: row.relay_state === 1,
            timestamp: row.timestamp
          };
          console.log('API: Returning database data:', data);
          res.json(data);
        } else {
          console.log('API: No data available');
          res.json({ message: 'No data available' });
        }
      }
    );
  }
});

// Control relay
app.post('/api/control', (req, res) => {
  console.log('API: Received control request:', req.body);
  const { relay } = req.body;
  
  if (typeof relay !== 'boolean') {
    console.log('API: Invalid relay state');
    return res.status(400).json({ error: 'Invalid relay state' });
  }
  
  // Send control command via MQTT
  mqttClient.publish('smartenergy/control', JSON.stringify({ relay }));
  console.log('API: Published control command via MQTT:', { relay });
  
  res.json({ success: true, relay });
});

// Get historical data
app.get('/api/history', (req, res) => {
  const { startDate, endDate } = req.query;
  
  let query = 'SELECT * FROM energy_data';
  let params = [];
  
  if (startDate && endDate) {
    query += ' WHERE timestamp >= ? AND timestamp <= ?';
    params = [parseInt(startDate), parseInt(endDate)];
  }
  
  query += ' ORDER BY timestamp DESC';
  
  db.all(query, params, (err, rows) => {
    if (err) {
      res.status(500).json({ error: 'Database error' });
    } else {
      const data = rows.map(row => ({
        voltage: row.voltage,
        current: row.current,
        power: row.power,
        energy: row.energy,
        frequency: row.frequency,
        powerFactor: row.power_factor,
        relayState: row.relay_state === 1,
        timestamp: row.timestamp
      }));
      res.json(data);
    }
  });
});

// Get analytics
app.get('/api/analytics', (req, res) => {
  const { startDate, endDate } = req.query;
  
  let query = 'SELECT * FROM energy_data';
  let params = [];
  
  if (startDate && endDate) {
    query += ' WHERE timestamp >= ? AND timestamp <= ?';
    params = [parseInt(startDate), parseInt(endDate)];
  }
  
  db.all(query, params, (err, rows) => {
    if (err) {
      res.status(500).json({ error: 'Database error' });
    } else if (rows.length === 0) {
      res.json({
        totalEnergy: 0,
        totalCost: 0,
        averagePower: 0,
        dataPoints: 0
      });
    } else {
      const totalEnergy = rows.reduce((sum, row) => sum + row.energy, 0);
      const totalPower = rows.reduce((sum, row) => sum + row.power, 0);
      const averagePower = totalPower / rows.length;
      
      res.json({
        totalEnergy: totalEnergy.toFixed(2),
        totalCost: (totalEnergy * 0.12).toFixed(2), // Default rate $0.12/kWh
        averagePower: averagePower.toFixed(2),
        dataPoints: rows.length
      });
    }
  });
});

// Store current data when received via MQTT
mqttClient.on('message', (topic, message) => {
  if (topic === 'smartenergy/data') {
    try {
      const data = JSON.parse(message.toString());
      currentData = {
        voltage: data.voltage,
        current: data.current,
        power: data.power,
        energy: data.energy,
        frequency: data.frequency,
        powerFactor: data.powerFactor,
        relayState: data.relayState,
        timestamp: data.timestamp || Date.now()
      };
    } catch (error) {
      console.error('Error parsing MQTT data:', error);
    }
  }
});

// Start server
app.listen(PORT, () => {
  console.log(`Smart Energy API server running on port ${PORT}`);
});

// Graceful shutdown
process.on('SIGINT', () => {
  console.log('Shutting down gracefully...');
  mqttClient.end();
  db.close((err) => {
    if (err) {
      console.error('Error closing database:', err);
    } else {
      console.log('Database connection closed');
    }
    process.exit(0);
  });
});
