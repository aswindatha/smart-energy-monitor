# Smart Energy API

Simple Node.js API server for the Smart Energy IoT system with no authentication.

## Installation

1. Install dependencies:
```bash
npm install
```

2. Start the server:
```bash
npm start
```

For development with auto-restart:
```bash
npm run dev
```

## Environment Variables

- `PORT`: Server port (default: 3000)
- `MQTT_BROKER`: MQTT broker URL (default: mqtt://localhost:1883)

## API Endpoints

### GET /api/data
Get current energy readings from the latest data.

**Response:**
```json
{
  "voltage": 220.5,
  "current": 2.3,
  "power": 506.15,
  "energy": 1234.56,
  "frequency": 50.0,
  "powerFactor": 0.95,
  "relayState": false,
  "timestamp": 1234567890
}
```

### POST /api/control
Control the relay state.

**Request:**
```json
{
  "relay": true
}
```

**Response:**
```json
{
  "success": true,
  "relay": true
}
```

### GET /api/history
Get historical energy data.

**Query Parameters:**
- `startDate`: Unix timestamp (optional)
- `endDate`: Unix timestamp (optional)

**Response:**
```json
[
  {
    "voltage": 220.5,
    "current": 2.3,
    "power": 506.15,
    "energy": 1234.56,
    "frequency": 50.0,
    "powerFactor": 0.95,
    "relayState": false,
    "timestamp": 1234567890
  }
]
```

### GET /api/analytics
Get energy analytics and cost calculations.

**Query Parameters:**
- `startDate`: Unix timestamp (optional)
- `endDate`: Unix timestamp (optional)

**Response:**
```json
{
  "totalEnergy": "123.45",
  "totalCost": "14.81",
  "averagePower": "456.78",
  "dataPoints": 100
}
```

## MQTT Topics

- **Subscribe**: `smartenergy/data` - Receives energy data from ESP32
- **Publish**: `smartenergy/control` - Sends control commands to ESP32

## Database

Uses SQLite database (`energy_data.db`) with the following table structure:

```sql
CREATE TABLE energy_data (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  voltage REAL NOT NULL,
  current REAL NOT NULL,
  power REAL NOT NULL,
  energy REAL NOT NULL,
  frequency REAL NOT NULL,
  power_factor REAL NOT NULL,
  relay_state INTEGER NOT NULL,
  timestamp INTEGER NOT NULL
);
```
