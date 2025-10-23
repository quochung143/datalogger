# DATALOGGER - IoT Environmental Monitoring System

A complete production-grade IoT data acquisition system for environmental monitoring with real-time visualization, cloud storage, and remote device control capabilities.

## System Architecture

```
┌─────────────┐    I2C     ┌─────────────┐    UART      ┌─────────────┐    WiFi/MQTT    ┌──────────────┐
│   SHT3X     │────────────│    STM32    │──────────────│    ESP32    │─────────────────│ MQTT Broker  │
│   Sensor    │            │  F103C8T6   │   115200bps  │  Gateway    │  ws://x.x.x.x   │ (Mosquitto)  │
└─────────────┘            └─────────────┘              └─────────────┘                 └──────────────┘
                                  │                             │                                │
                           ┌──────┴──────┐              ┌──────┴──────┐                        │
                           │  DS3231 RTC │              │ Relay GPIO4 │                        │
                           │  Timestamp  │              │   Control   │                        │
                           └─────────────┘              └─────────────┘                        │
                                  │                                                             │
                           ┌──────┴──────┐                                             ┌───────┴────────┐
                           │  SD Card    │                                             │  Web Dashboard │
                           │  Buffer     │                                             │  + Firebase DB │
                           │  (Offline)  │                                             └────────────────┘
                           └─────────────┘
                                  │
                           ┌──────┴──────┐
                           │  ILI9225    │
                           │  Display    │
                           │  176x220px  │
                           └─────────────┘
```

**Communication Flow:**
- **Upstream (Data)**: STM32 → UART (115200bps) → ESP32 → MQTT (QoS 0/1) → Web Dashboard
- **Downstream (Commands)**: Web Dashboard → MQTT → ESP32 → UART → STM32 → Sensor/Relay
- **Offline Buffering**: STM32 → SD Card (204,800 records) → Sync when ESP32 reconnects
- **Cloud Storage**: MQTT → Firebase Realtime Database → Historical data retrieval

## Overview

This project implements a **production-ready environmental monitoring system** designed for industrial, laboratory, and IoT applications. The system provides:

### Core Capabilities
- **Real-time sensor monitoring** with SHT3X temperature/humidity sensors (±0.2°C, ±2% RH accuracy)
- **Offline data buffering** to SD card (204,800 records capacity) during network outages
- **Automatic data synchronization** when connectivity is restored
- **Real-time visualization** via web dashboard with Chart.js line charts
- **Cloud data persistence** with Firebase Realtime Database integration
- **Remote device control** via MQTT commands (relay switching, measurement modes)
- **Local LCD display** (ILI9225 176x220) showing live readings and system status
- **Accurate timestamping** using DS3231 RTC module with battery backup

### System Components
1. **STM32F103C8T6 Firmware** - Data acquisition, sensor interfacing, SD buffering, and display control
2. **ESP32 Gateway** - WiFi/MQTT bridge, relay control, and UART communication
3. **Mosquitto MQTT Broker** - Message routing with WebSocket support (ports 1883, 8083)
4. **Web Dashboard** - Real-time monitoring, historical data analysis, and device control interface

## Key Features

### Data Acquisition & Storage
- **High-accuracy sensing**: SHT3X sensor with ±0.2°C temperature and ±2% RH humidity accuracy
- **Flexible sampling rates**: Configurable from 0.5Hz to 10Hz for single reads or periodic monitoring
- **Offline data buffering**: SD card storage with 204,800 records capacity during network outages
- **Automatic synchronization**: Buffered data automatically syncs when connectivity is restored
- **Accurate timestamping**: DS3231 RTC with battery backup (±2ppm accuracy, 0-40°C)
- **Local display**: ILI9225 LCD (176x220px) showing live temperature, humidity, timestamp, and system status

### Communication & Control
- **MQTT v5 protocol**: Robust messaging with QoS 0/1 support for reliable data delivery
- **WebSocket support**: Real-time web client connectivity on port 8083
- **Remote relay control**: GPIO-based switching for connected devices (pumps, fans, alarms)
- **Command interface**: Web-based control for sensor configuration and device management
- **Interrupt-driven UART**: Efficient 115200bps STM32↔ESP32 communication with 256-byte ring buffer
- **FreeRTOS task management**: Priority-based processing on ESP32 gateway

### Web Dashboard & Visualization
- **7-page single-page application**: Dashboard, Live Data, Device Settings, Time Settings, Data Management, Logs, Configuration
- **Real-time charts**: Chart.js line graphs with live sensor data updates
- **Firebase integration**: Cloud database for historical data storage and retrieval
- **Component health monitoring**: Live status indicators for all system components
- **Data filtering & export**: Date range filtering with CSV export capability
- **Multi-mode operation**: Single read, periodic monitoring, and interval configuration
- **Time synchronization**: Internet NTP or manual time setting with RTC update

### Reliability & Security
- **Bcrypt authentication**: Secure MQTT broker access with encrypted passwords
- **Persistent messaging**: MQTT message storage survives broker restarts
- **Automatic reconnection**: Intelligent retry logic for WiFi and MQTT failures
- **Error handling**: Comprehensive sensor failure detection and recovery
- **Modular architecture**: 9 ESP32 components, cleanly separated concerns
- **Production-ready**: Extensively tested with detailed documentation

## Detailed Subsystem Information

### 1. STM32 Firmware (Cortex-M3, 72MHz)

**Primary Functions:**
- SHT3X sensor data acquisition via I2C (100kHz, address 0x44)
- DS3231 RTC time synchronization (I2C address 0x68)
- SD card buffering (FAT32, 204,800 records capacity)
- ILI9225 LCD display driver (176x220px, SPI interface)
- UART command interface (115200bps, 8N1, interrupt-driven)
- JSON data formatting with Unix timestamps

**Command Interface:**
```
START_PERIODIC:<interval_ms>  - Begin periodic sampling
STOP_PERIODIC                 - Stop periodic sampling
READ_SINGLE                   - Single sensor read
SET_TIME:<unix_timestamp>     - Set RTC time
```

**JSON Output Format:**
```json
{
  "temperature": 25.43,
  "humidity": 60.21,
  "timestamp": 1735689600,
  "mode": "periodic"
}
```

**Key Specifications:**
- Interrupt-driven UART with 256-byte ring buffer
- I2C bus at 100kHz for sensor/RTC communication
- SD card FAT32 with buffered writes
- SPI LCD interface at maximum STM32 SPI clock
- HAL-based driver architecture

**Documentation:** See `firmware/STM32/README.md` for complete details

---

### 2. ESP32 Gateway (Dual-Core Xtensa, 240MHz)

**Primary Functions:**
- WiFi connectivity (802.11 b/g/n, WPA2-PSK)
- MQTT v5 client with QoS 0/1 support
- UART↔MQTT bidirectional bridge
- Relay control on GPIO4 (active high)
- FreeRTOS task-based architecture

**Component Architecture (9 Components):**
1. **wifi_manager**: WiFi connection and credential management
2. **mqtt_handler**: MQTT v5 client with auto-reconnection
3. **stm32_uart**: UART communication with STM32 (115200bps)
4. **relay_control**: GPIO4 relay switching
5. **json_sensor_parser**: Parse STM32 sensor JSON
6. **json_utils**: Generic JSON utilities
7. **button_handler**: Physical button input (optional)
8. **ring_buffer**: Circular buffer for UART data
9. **coap_handler**: CoAP protocol support (optional)

**MQTT Topics:**
```
datalogger/command           - Incoming commands (QoS 1)
datalogger/relay             - Relay control (QoS 1)
datalogger/state             - Device state reports (QoS 0)
datalogger/data/single       - Single read data (QoS 0)
datalogger/data/periodic     - Periodic data stream (QoS 0)
```

**Configuration (menuconfig):**
- WiFi SSID/Password
- MQTT broker IP/port
- UART GPIO pins (default GPIO16 RX, GPIO17 TX)
- Relay GPIO (default GPIO4)

**Documentation:** See `firmware/ESP32/README.md` for complete details

---

### 3. MQTT Broker (Mosquitto 2.x)

**Deployment:**
- Docker containerized for easy deployment
- Port 1883: Standard MQTT protocol
- Port 8083: WebSocket for web clients (ws://broker-ip:8083/mqtt)

**Security:**
- Bcrypt password authentication
- Username/password storage in `broker/config/auth/passwd.txt`
- TLS support (optional, configure in mosquitto.conf)

**Configuration:**
```
listener 1883                    # Standard MQTT
listener 8083                    # WebSocket
protocol websockets
persistence true
persistence_location /mosquitto/data/
log_dest stdout
log_dest file /mosquitto/log/mosquitto.log
password_file /mosquitto/config/auth/passwd.txt
```

**Monitoring:**
```bash
docker logs -f mosquitto-broker
mosquitto_sub -h localhost -t '#' -v
```

**Documentation:** See `broker/README.md` for complete details

---

### 4. Web Dashboard (HTML5 + JavaScript)

**Technology Stack:**
- **Frontend**: HTML5, CSS3 (Flexbox/Grid), Vanilla JavaScript
- **MQTT**: MQTT.js library with WebSocket transport
- **Charts**: Chart.js for real-time line graphs
- **Database**: Firebase Realtime Database for historical storage
- **Architecture**: Single-page application with dynamic page loading

**7-Page Interface:**
1. **Dashboard**: System overview, live sensor readings, component health
2. **Live Data**: Real-time chart with Chart.js visualization
3. **Device Settings**: Relay control, periodic mode toggle, interval configuration
4. **Time Settings**: Internet NTP sync or manual time setting
5. **Data Management**: Date range filtering, CSV export, Firebase queries
6. **Logs**: System event logs with filtering
7. **Configuration**: MQTT/Firebase connection settings

**Core Features:**
- Real-time MQTT message handling with automatic reconnection
- Firebase Realtime Database integration for historical data
- Component health monitoring (STM32, ESP32, MQTT, Firebase status)
- Responsive design for desktop and mobile devices
- CSV data export functionality
- Live Chart.js graphs with 50-point rolling window

**MQTT Integration:**
```javascript
// Subscribe to all datalogger topics
client.subscribe('datalogger/#', { qos: 1 });

// Publish commands
client.publish('datalogger/command', JSON.stringify({
  command: 'START_PERIODIC',
  interval: 5000
}), { qos: 1 });
```

**Deployment Options:**
1. **Python HTTP Server** (Development):
   ```bash
   cd web
   python -m http.server 8000
   ```

2. **Nginx** (Production):
   ```nginx
   server {
     listen 80;
     root /var/www/datalogger;
     index index.html;
   }
   ```

**Documentation:** See `web/README.md` for complete details

## Hardware Requirements

### Electronic Components

#### Core Microcontrollers
- **STM32F103C8T6** "Blue Pill" or equivalent (ARM Cortex-M3, 72MHz, 64KB Flash, 20KB RAM)
- **ESP32-WROOM-32** or ESP32-DevKitC (Dual-core Xtensa LX6, 240MHz, WiFi 802.11 b/g/n)

#### Sensors & Peripherals
- **SHT30/SHT31/SHT35** temperature/humidity sensor (I2C, ±0.2°C, ±2% RH accuracy)
- **DS3231** high-precision RTC module (I2C, ±2ppm accuracy, battery backup)
- **MicroSD card module** (SPI, FAT32 formatted, 1GB+ recommended for buffering)
- **ILI9225 LCD display** (176x220px, 2.2", SPI interface) - for local status display
- **5V relay module** (optional, for device control - pumps, fans, alarms)

#### Programming & Debug Tools
- **ST-Link V2** programmer for STM32 (or compatible JTAG/SWD debugger)
- **USB-to-TTL adapter** for UART debugging (CP2102, CH340, or FTDI)

#### Power Supply
- **5V DC power supply** (1A minimum recommended for entire system)
- **CR2032 battery** for DS3231 RTC backup

---

### Hardware Connections

#### STM32 Pinout Configuration

**I2C Bus (Shared: SHT3X + DS3231 + Display)**
| STM32 Pin | I2C Function | Connected Devices |
|-----------|--------------|-------------------|
| PB6 | I2C1_SCL | SHT3X (0x44), DS3231 (0x68) |
| PB7 | I2C1_SDA | SHT3X, DS3231 |

**UART Communication (STM32 ↔ ESP32)**
| STM32 Pin | UART Function | ESP32 Pin | Signal Direction |
|-----------|---------------|-----------|------------------|
| PA9 | USART1_TX | GPIO16 (RX2) | STM32 → ESP32 |
| PA10 | USART1_RX | GPIO17 (TX2) | ESP32 → STM32 |
| GND | Ground | GND | Common ground |

**SPI Bus (SD Card + ILI9225 Display)**
| STM32 Pin | SPI Function | SD Card Pin | ILI9225 Pin |
|-----------|--------------|-------------|-------------|
| PA5 | SPI1_SCK | CLK | SCK |
| PA6 | SPI1_MISO | MISO | - |
| PA7 | SPI1_MOSI | MOSI | SDI |
| PA4 | SPI1_NSS (SD) | CS | - |
| PB0 | GPIO (LCD CS) | - | CS |
| PB1 | GPIO (LCD RS) | - | RS |

**Power Distribution**
| STM32 Pin | Function | Notes |
|-----------|----------|-------|
| 3.3V | Power output | For SHT3X, DS3231, SD card logic |
| 5V | Power input | From USB or external 5V regulator |
| GND | Ground | Common ground for all modules |

#### ESP32 Pinout Configuration

**UART Communication (ESP32 ↔ STM32)**
| ESP32 Pin | UART Function | STM32 Pin | Signal Direction |
|-----------|---------------|-----------|------------------|
| GPIO16 | UART2_RX | PA9 (TX) | STM32 → ESP32 |
| GPIO17 | UART2_TX | PA10 (RX) | ESP32 → STM32 |
| GND | Ground | GND | Common ground |

**Relay Control**
| ESP32 Pin | Function | Relay Module Pin | Signal Type |
|-----------|----------|------------------|-------------|
| GPIO4 | Digital Output | IN | Active HIGH (3.3V) |
| 5V | Power supply | VCC | Relay power |
| GND | Ground | GND | Common ground |

**WiFi Antenna**
- Built-in PCB antenna (ESP32-WROOM-32)
- External antenna connector (optional, for better range)

**Power Supply**
| ESP32 Pin | Function | Notes |
|-----------|----------|-------|
| 5V (VIN) | Power input | From USB or external 5V regulator |
| 3.3V | Power output | Internal LDO, max 500mA |
| GND | Ground | Common ground |

---

### Complete System Wiring Diagram

```
                                    5V Power Supply
                                          │
                    ┌─────────────────────┼─────────────────────┐
                    │                     │                     │
              ┌─────┴─────┐         ┌─────┴─────┐         ┌─────┴─────┐
              │  STM32    │         │   ESP32   │         │   Relay   │
              │ F103C8T6  │         │ WROOM-32  │         │  Module   │
              └───────────┘         └───────────┘         └───────────┘
                    │                     │                     │
      ┌─────────────┼─────────────┐      │                     │
      │             │             │      │                     │
┌─────┴─────┐ ┌────┴────┐ ┌─────┴┴─────┐│              ┌──────┴──────┐
│   SHT3X   │ │ DS3231  │ │  SD Card   ││              │ GPIO4 (OUT) │
│  Sensor   │ │   RTC   │ │  Module    ││              └─────────────┘
└───────────┘ └─────────┘ └────────────┘│
   I2C 0x44     I2C 0x68      SPI        │
                                         │
                                   ┌─────┴──────┐
                                   │  ILI9225   │
                                   │   Display  │
                                   └────────────┘
                                    SPI + GPIO

UART Connection (115200bps, 8N1):
STM32 PA9 (TX) ──────────────────► ESP32 GPIO16 (RX)
STM32 PA10 (RX) ◄──────────────── ESP32 GPIO17 (TX)
STM32 GND ───────────────────────── ESP32 GND
```

---

### I2C Device Addresses

| Device | I2C Address | Bus Speed |
|--------|-------------|-----------|
| SHT3X Sensor | 0x44 (default) or 0x45 | 100kHz |
| DS3231 RTC | 0x68 | 100kHz |

**Note**: Both devices share the same I2C bus (PB6/PB7 on STM32)

## Software Installation & Deployment

### Prerequisites

**Development Tools:**
- **STM32CubeIDE** v1.10+ or **ARM GCC toolchain** for STM32 development
- **ESP-IDF** v5.0+ for ESP32 development (install via `get_idf` script)
- **Docker** v20.10+ for MQTT broker deployment
- **Python** 3.7+ with `http.server` module (for web development server)
- **Git** for repository cloning

**Optional Tools:**
- **STM32CubeProgrammer** for STM32 firmware flashing
- **esptool.py** for ESP32 firmware flashing (included with ESP-IDF)
- **Nginx** or **Apache** for production web hosting
- **Firebase Console** account for cloud database setup

---

### Step-by-Step Installation

#### 1. Clone the Repository

```bash
git clone https://github.com/yourusername/DATALOGGER-Dev2.0.git
cd DATALOGGER-Dev2.0
```

---

#### 2. STM32 Firmware Build & Flash

**Build with STM32CubeIDE:**
```bash
# Open STM32CubeIDE
# File -> Import -> Existing Projects into Workspace
# Select: firmware/STM32/
# Build: Project -> Build All (Ctrl+B)
```

**Build with ARM GCC (Command Line):**
```bash
cd firmware/STM32
make clean
make -j4
```

**Flash Firmware:**
```bash
# Using STM32CubeProgrammer (GUI)
# Connect ST-Link V2 to STM32 SWD pins
# Load firmware/STM32/build/STM32_DATALOGGER.hex
# Click "Start Programming"

# Using st-flash (Command Line)
st-flash write build/STM32_DATALOGGER.bin 0x08000000
```

**Verify Installation:**
- Connect USB-to-TTL adapter to STM32 UART (PA9/PA10)
- Open serial terminal: `115200 baud, 8N1, no flow control`
- Send command: `READ_SINGLE`
- Expected response: JSON with temperature/humidity data

**Detailed Documentation:** See `firmware/STM32/README.md`

---

#### 3. ESP32 Firmware Build & Flash

**Setup ESP-IDF Environment:**
```bash
# Install ESP-IDF (if not already installed)
# Windows:
$env:IDF_PATH = "C:\esp\esp-idf"
. $env:IDF_PATH\export.ps1

# Linux/macOS:
export IDF_PATH=~/esp/esp-idf
. $IDF_PATH/export.sh
```

**Configure WiFi & MQTT:**
```bash
cd firmware/ESP32
idf.py menuconfig

# Navigate to:
# Component config -> WiFi Manager
#   - WiFi SSID: [Your WiFi Name]
#   - WiFi Password: [Your WiFi Password]
#
# Component config -> MQTT Handler
#   - MQTT Broker URI: mqtt://[broker-ip]:1883
#   - MQTT Username: [broker-username]
#   - MQTT Password: [broker-password]
#
# Save and exit (S -> Enter -> Q)
```

**Build & Flash:**
```bash
idf.py build
idf.py -p COM3 flash monitor  # Replace COM3 with your ESP32 port

# Linux example:
idf.py -p /dev/ttyUSB0 flash monitor
```

**Verify Installation:**
- Check serial monitor output for WiFi connection success
- Verify MQTT connection: `MQTT connected successfully`
- Check STM32 UART communication: `UART initialized on GPIO16/17`

**Detailed Documentation:** See `firmware/ESP32/README.md`

---

#### 4. MQTT Broker Deployment

**Deploy with Docker:**
```bash
cd broker

# Build Docker image
docker build -t mosquitto-datalogger .

# Run broker container
docker run -d \
  --name mosquitto-broker \
  -p 1883:1883 \
  -p 8083:8083 \
  -v $(pwd)/config:/mosquitto/config \
  -v mosquitto-data:/mosquitto/data \
  -v mosquitto-logs:/mosquitto/log \
  --restart unless-stopped \
  mosquitto-datalogger
```

**Windows PowerShell:**
```powershell
cd broker
docker build -t mosquitto-datalogger .
docker run -d --name mosquitto-broker -p 1883:1883 -p 8083:8083 `
  -v ${PWD}/config:/mosquitto/config `
  -v mosquitto-data:/mosquitto/data `
  -v mosquitto-logs:/mosquitto/log `
  --restart unless-stopped mosquitto-datalogger
```

**Configure Authentication:**
```bash
# Add MQTT users (inside container)
docker exec -it mosquitto-broker sh
mosquitto_passwd -c /mosquitto/config/auth/passwd.txt admin
# Enter password when prompted

# Restart broker to apply changes
docker restart mosquitto-broker
```

**Verify Installation:**
```bash
# Check broker logs
docker logs -f mosquitto-broker

# Test MQTT connection
mosquitto_sub -h localhost -t 'datalogger/#' -u admin -P [your-password] -v

# Expected output: MQTT messages from ESP32
```

**Detailed Documentation:** See `broker/README.md`

---

#### 5. Web Dashboard Setup

**Option A: Development Server (Python)**
```bash
cd web
python -m http.server 8000

# Open browser: http://localhost:8000
```

**Option B: Production Server (Nginx)**
```bash
# Install Nginx (Ubuntu/Debian)
sudo apt update
sudo apt install nginx

# Copy web files to Nginx root
sudo cp -r web/* /var/www/html/datalogger/

# Configure Nginx
sudo nano /etc/nginx/sites-available/datalogger
```

**Nginx Configuration:**
```nginx
server {
    listen 80;
    server_name datalogger.local;  # or your domain
    
    root /var/www/html/datalogger;
    index index.html;
    
    location / {
        try_files $uri $uri/ =404;
    }
    
    # Enable gzip compression
    gzip on;
    gzip_types text/css application/javascript application/json;
}
```

```bash
# Enable site and restart Nginx
sudo ln -s /etc/nginx/sites-available/datalogger /etc/nginx/sites-enabled/
sudo nginx -t
sudo systemctl restart nginx
```

**Configure MQTT & Firebase:**

Edit `web/app.js`:
```javascript
// MQTT Configuration (line ~15)
const MQTT_CONFIG = {
    broker: 'ws://[YOUR-BROKER-IP]:8083/mqtt',  // WebSocket address
    username: 'admin',
    password: '[your-password]',
    clientId: 'datalogger-web-' + Math.random().toString(16).substr(2, 8)
};

// Firebase Configuration (line ~30)
const firebaseConfig = {
    apiKey: "[YOUR-FIREBASE-API-KEY]",
    authDomain: "[YOUR-PROJECT-ID].firebaseapp.com",
    databaseURL: "https://[YOUR-PROJECT-ID].firebaseio.com",
    projectId: "[YOUR-PROJECT-ID]",
    storageBucket: "[YOUR-PROJECT-ID].appspot.com",
    messagingSenderId: "[YOUR-SENDER-ID]",
    appId: "[YOUR-APP-ID]"
};
```

**Verify Installation:**
- Open browser to `http://localhost:8000` (development) or `http://your-domain` (production)
- Check **Configuration** page for MQTT connection status: `Connected`
- Navigate to **Dashboard** to see live sensor readings
- Check **Live Data** page for real-time chart updates

**Detailed Documentation:** See `web/README.md`

---

### System Integration Testing

After installing all components, verify complete system integration:

**1. Hardware Verification:**
```bash
# STM32: Check UART output (115200 baud)
# Expected: JSON sensor data on READ_SINGLE command

# ESP32: Check serial monitor
# Expected: "MQTT connected", "WiFi connected", UART data forwarding
```

**2. MQTT Communication Test:**
```bash
# Subscribe to all topics
mosquitto_sub -h [broker-ip] -t 'datalogger/#' -u admin -P [password] -v

# Expected output:
# datalogger/data/single {"temperature":25.3,"humidity":60.2,"timestamp":1735689600}
# datalogger/state {"status":"connected","uptime":12345}
```

**3. Web Dashboard Test:**
- Open web dashboard in browser
- Click **"Read Single"** button in Device Settings
- Verify sensor data appears in Dashboard within 2 seconds
- Enable **Periodic Mode** and check Live Data chart updates
- Test relay control (should hear click from relay module)

**4. Offline Buffering Test:**
```bash
# Disconnect ESP32 from WiFi (power off ESP32 or disconnect router)
# STM32 should continue logging to SD card
# Check ILI9225 display: "SD: WRITING" status

# Reconnect ESP32 to WiFi
# STM32 should sync buffered data to MQTT automatically
# Check web dashboard: Historical data should appear
```

---

### Troubleshooting Common Issues

**STM32 Not Responding:**
- Check ST-Link connection (SWDIO, SWCLK, GND, 3.3V)
- Verify 5V power supply to STM32
- Check UART baud rate: must be 115200
- Press RESET button on STM32 after flashing

**ESP32 WiFi Connection Failed:**
- Verify WiFi SSID/password in `idf.py menuconfig`
- Check WiFi signal strength (move closer to router)
- Ensure WiFi is 2.4GHz (ESP32 doesn't support 5GHz)
- Check serial monitor for error messages: `idf.py monitor`

**MQTT Connection Failed:**
- Verify broker IP address in ESP32 configuration
- Check broker is running: `docker ps | grep mosquitto`
- Test broker port: `telnet [broker-ip] 1883`
- Verify username/password authentication
- Check broker logs: `docker logs mosquitto-broker`

**Web Dashboard Not Connecting:**
- Check WebSocket URL: must be `ws://` not `wss://`
- Verify broker port 8083 is accessible (firewall/router)
- Check browser console (F12) for JavaScript errors
- Test MQTT WebSocket: `wscat -c ws://[broker-ip]:8083/mqtt`

**Sensor Reading Errors:**
- Check SHT3X I2C connections (PB6/PB7)
- Verify I2C address: default 0x44 (use I2C scanner to detect)
- Check 3.3V power supply to sensor
- Ensure pull-up resistors on I2C lines (4.7kΩ recommended)

**SD Card Not Detected:**
- Verify SD card is FAT32 formatted
- Check SPI connections (PA4-PA7)
- Ensure SD card is inserted properly
- Check 3.3V power to SD card module

**ILI9225 Display Blank:**
- Verify SPI connections and CS/RS GPIO pins
- Check display power supply (5V or 3.3V depending on module)
- Test with LCD test pattern in firmware
- Ensure correct SPI mode configuration

For detailed troubleshooting guides, refer to component-specific README files in `firmware/STM32/`, `firmware/ESP32/`, `broker/`, and `web/` directories.
- Web browser with JavaScript enabled

## Quick Start

### MQTT Broker Setup

Create the required directory structure and user credentials:

```bash
mkdir -p broker/config/auth broker/data broker/log
docker run --rm -v "$PWD/broker/config/auth:/work" eclipse-mosquitto:2 \
  mosquitto_passwd -c /work/passwd.txt DataLogger
```

Start the MQTT broker with Docker:

```bash
docker run -d --name mqtt-broker \
  -p 1883:1883 \
  -p 8083:8083 \
  -v "$PWD/broker/mosquitto.conf:/mosquitto/config/mosquitto.conf" \
  -v "$PWD/broker/config/auth:/mosquitto/config/auth" \
  -v "$PWD/broker/data:/mosquitto/data" \
  -v "$PWD/broker/log:/mosquitto/log" \
  eclipse-mosquitto:2
```

The broker will listen on:
- Port 1883: Standard MQTT protocol (for ESP32)
- Port 8083: WebSocket protocol (for web dashboard)

### STM32 Firmware Installation

Using STM32CubeIDE:
1. Open the project directory: `firmware/STM32/`
2. Build the project: Project -> Build All
3. Connect ST-Link programmer to STM32
4. Flash firmware: Run -> Debug

Using command-line GCC toolchain:
```bash
cd firmware/STM32/
make clean && make all
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg \
  -c "program build/STM32_DATALOGGER.elf verify reset exit"
```

### ESP32 Firmware Installation

Configure WiFi and MQTT settings:
```bash
cd firmware/ESP32/
idf.py menuconfig
```

Navigate to configuration sections:
- Example Connection Configuration: Set WiFi SSID and password
- ESP32 MQTT5 Bridge Configuration: Set MQTT broker IP and credentials

Build and flash:
```bash
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

### Web Dashboard Deployment

For development:
```bash
cd web/
python -m http.server 8080
```

Access at http://localhost:8080 and configure MQTT broker settings.

---

## MQTT Communication Protocol

### Complete Topic Structure

| Topic | Direction | QoS | Payload Type | Purpose |
|-------|-----------|-----|--------------|---------|
| `datalogger/command` | Web/App → STM32 | 1 | JSON | Sensor control commands (single, periodic, interval) |
| `datalogger/relay` | Web/App → ESP32 | 1 | JSON | Relay on/off control |
| `datalogger/state` | STM32 → Web/App | 0 | JSON | Device state reports (connection, uptime, errors) |
| `datalogger/data/single` | STM32 → Web/App | 0 | JSON | Single sensor read results |
| `datalogger/data/periodic` | STM32 → Web/App | 0 | JSON | Periodic monitoring data stream |
| `datalogger/time/sync` | Web/App → STM32 | 1 | JSON | RTC time synchronization |

---

### Command Message Formats

#### 1. Single Sensor Read
**Topic:** `datalogger/command`  
**Payload:**
```json
{
  "command": "READ_SINGLE"
}
```

**Response on** `datalogger/data/single`:
```json
{
  "temperature": 25.43,
  "humidity": 60.21,
  "timestamp": 1735689600,
  "mode": "single"
}
```

---

#### 2. Start Periodic Monitoring
**Topic:** `datalogger/command`  
**Payload:**
```json
{
  "command": "START_PERIODIC",
  "interval": 5000
}
```
- `interval`: Milliseconds between readings (500ms to 60000ms)

**Continuous responses on** `datalogger/data/periodic`:
```json
{
  "temperature": 24.87,
  "humidity": 58.92,
  "timestamp": 1735689605,
  "mode": "periodic"
}
```

---

#### 3. Stop Periodic Monitoring
**Topic:** `datalogger/command`  
**Payload:**
```json
{
  "command": "STOP_PERIODIC"
}
```

---

#### 4. Relay Control
**Topic:** `datalogger/relay`  
**Payload:**
```json
{
  "state": "ON"
}
```
or
```json
{
  "state": "OFF"
}
```

**Response:** Relay clicks audibly, GPIO4 changes state

---

#### 5. Time Synchronization
**Topic:** `datalogger/time/sync`  
**Payload:**
```json
{
  "command": "SET_TIME",
  "timestamp": 1735689600
}
```
- `timestamp`: Unix timestamp (seconds since 1970-01-01 00:00:00 UTC)

**Response on** `datalogger/state`:
```json
{
  "status": "time_updated",
  "timestamp": 1735689600,
  "message": "RTC synchronized"
}
```

---

#### 6. Device State Report
**Topic:** `datalogger/state` (published by STM32/ESP32)  
**Payload:**
```json
{
  "status": "connected",
  "uptime": 12345,
  "sensor": "OK",
  "rtc": "OK",
  "sd_card": "writing",
  "display": "OK",
  "wifi_rssi": -65
}
```

---

### Command Line Examples (mosquitto_pub/sub)

**Subscribe to all datalogger topics:**
```bash
mosquitto_sub -h [broker-ip] -p 1883 -t 'datalogger/#' -u admin -P [password] -v
```

**Single sensor read:**
```bash
mosquitto_pub -h [broker-ip] -p 1883 -t 'datalogger/command' \
  -u admin -P [password] \
  -m '{"command":"READ_SINGLE"}'
```

**Start periodic monitoring (5 second interval):**
```bash
mosquitto_pub -h [broker-ip] -p 1883 -t 'datalogger/command' \
  -u admin -P [password] \
  -m '{"command":"START_PERIODIC","interval":5000}'
```

**Stop periodic monitoring:**
```bash
mosquitto_pub -h [broker-ip] -p 1883 -t 'datalogger/command' \
  -u admin -P [password] \
  -m '{"command":"STOP_PERIODIC"}'
```

**Turn relay ON:**
```bash
mosquitto_pub -h [broker-ip] -p 1883 -t 'datalogger/relay' \
  -u admin -P [password] \
  -m '{"state":"ON"}'
```

**Turn relay OFF:**
```bash
mosquitto_pub -h [broker-ip] -p 1883 -t 'datalogger/relay' \
  -u admin -P [password] \
  -m '{"state":"OFF"}'
```

**Sync RTC time:**
```bash
# Get current Unix timestamp
TIMESTAMP=$(date +%s)

mosquitto_pub -h [broker-ip] -p 1883 -t 'datalogger/time/sync' \
  -u admin -P [password] \
  -m "{\"command\":\"SET_TIME\",\"timestamp\":$TIMESTAMP}"
```

---

### WebSocket MQTT (for Web Dashboard)

The web dashboard connects via WebSocket on port 8083:

```javascript
// JavaScript example (already implemented in web/app.js)
const client = mqtt.connect('ws://[broker-ip]:8083/mqtt', {
    username: 'admin',
    password: '[your-password]',
    clientId: 'datalogger-web-' + Math.random().toString(16).substr(2, 8)
});

// Subscribe to all topics
client.subscribe('datalogger/#', { qos: 1 });

// Publish single read command
client.publish('datalogger/command', JSON.stringify({
    command: 'READ_SINGLE'
}), { qos: 1 });

// Handle incoming messages
client.on('message', (topic, message) => {
    const data = JSON.parse(message.toString());
    console.log(`Received on ${topic}:`, data);
});
```

---

## System Performance Specifications

### Data Acquisition Performance
| Metric | Specification | Notes |
|--------|--------------|-------|
| **Sampling Rate** | 0.5Hz to 10Hz | Configurable via web dashboard |
| **Temperature Accuracy** | ±0.2°C | SHT3X sensor specification |
| **Humidity Accuracy** | ±2% RH | SHT3X sensor specification |
| **Timestamp Accuracy** | ±2ppm (0-40°C) | DS3231 RTC with battery backup |
| **End-to-End Latency** | <150ms | Sensor read → Web dashboard display |

### Communication Performance
| Metric | Specification | Notes |
|--------|--------------|-------|
| **UART Baud Rate** | 115200bps | STM32 ↔ ESP32, 8N1, no flow control |
| **I2C Bus Speed** | 100kHz | SHT3X + DS3231 shared bus |
| **SPI Clock Speed** | 18MHz (STM32 max) | SD card + ILI9225 display |
| **WiFi Standard** | 802.11 b/g/n | 2.4GHz only (ESP32 limitation) |
| **MQTT QoS** | 0 (data), 1 (commands) | QoS 0 for high-throughput, QoS 1 for reliability |
| **WebSocket Latency** | <50ms | Web dashboard real-time updates |

### Storage & Buffering
| Metric | Specification | Notes |
|--------|--------------|-------|
| **SD Card Capacity** | 204,800 records | ~14 days at 1 sample/second |
| **UART Ring Buffer** | 256 bytes | Interrupt-driven, prevents data loss |
| **MQTT Buffer** | 8KB (ESP32) | FreeRTOS queue-based |
| **Firebase Storage** | Unlimited | Cloud historical data retention |

### Power Consumption
| Component | Typical Current | Peak Current | Notes |
|-----------|----------------|--------------|-------|
| **STM32F103C8T6** | 30-50mA | 80mA | Active mode at 72MHz |
| **ESP32 (WiFi active)** | 80-120mA | 240mA | Transmitting/receiving |
| **SHT3X Sensor** | 1.5µA (idle) | 800µA (measuring) | Ultra-low power |
| **DS3231 RTC** | 100µA | 100µA | Battery backup mode: 3µA |
| **SD Card** | 20-50mA | 100mA | Write operations |
| **ILI9225 Display** | 30-50mA | 80mA | Backlight on |
| **Relay Module** | 15-20mA | 70mA | Coil energized |
| **Total System** | ~200mA @ 5V | ~500mA @ 5V | All components active |

### Display Specifications
| Metric | Specification | Notes |
|--------|--------------|-------|
| **LCD Resolution** | 176×220 pixels | ILI9225 controller |
| **Display Size** | 2.2 inch diagonal | TFT LCD |
| **Color Depth** | 262K colors (18-bit) | RGB565 format |
| **Refresh Rate** | 1Hz (dashboard mode) | Configurable update interval |

---

## Project Directory Structure

```
DATALOGGER-Dev2.0/
├── LICENSE.md                             # MIT License
├── README.md                              # This file - project overview
│
├── broker/                                # MQTT Broker (Mosquitto)
│   ├── README.md                          # Broker setup & configuration guide
│   ├── mosquitto.conf                     # Main broker configuration
│   └── config/
│       ├── mosquitto.conf                 # Docker-mounted configuration
│       └── auth/
│           └── passwd.txt                 # Bcrypt password file
│
├── documents/                             # Technical Documentation
│   ├── STM32/                             # STM32 firmware diagrams
│   │   ├── FLOW_DIAGRAM_STM32.md          # 15 flowcharts (Mermaid)
│   │   ├── SEQUENCE_DIAGRAM_STM32.md      # 10 sequence diagrams
│   │   └── UML_CLASS_DIAGRAM_STM32.md     # 11 architecture diagrams
│   ├── ESP32/                             # ESP32 gateway diagrams
│   │   ├── FLOW_DIAGRAM_ESP32.md          # 15 flowcharts (Mermaid)
│   │   ├── SEQUENCE_DIAGRAM_ESP32.md      # 10 sequence diagrams
│   │   └── UML_CLASS_DIAGRAM_ESP32.md     # 11 architecture diagrams
│   ├── firmware/                          # Complete firmware system diagrams
│   │   ├── FLOW_DIAGRAM_FIRMWARE.md       # 31 comprehensive flowcharts
│   │   ├── SEQUENCE_DIAGRAM_FIRMWARE.md   # System-level sequences
│   │   └── UML_DIAGRAM_FIRMWARE.md        # Complete system UML
│   ├── WEB/                               # Web dashboard diagrams
│   │   ├── FLOW_DIAGRAM_WEB.md            # 15 web application flowcharts
│   │   ├── SEQUENCE_DIAGRAM_WEB.md        # 10 web interaction sequences
│   │   └── UML_DIAGRAM_WEB.md             # 11 web architecture diagrams
│   └── SYSTEM/                            # Overall system documentation
│       ├── FLOW_DIAGRAM_SYSTEM.md         # System-level data flow
│       ├── SEQUENCE_DIAGRAM_SYSTEM.md     # End-to-end sequences
│       └── UML_DIAGRAM_SYSTEM.md          # Complete system architecture
│
├── firmware/                              # Embedded Firmware
│   ├── README.md                          # Firmware overview
│   │
│   ├── STM32/                             # STM32F103C8T6 Firmware
│   │   ├── README.md                      # 855 lines - comprehensive documentation
│   │   ├── STM32_DATALOGGER.ioc           # STM32CubeMX configuration
│   │   ├── STM32F103C8TX_FLASH.ld         # Linker script
│   │   ├── CMakeLists.txt                 # CMake build configuration
│   │   ├── Core/                          # HAL-generated code
│   │   │   ├── Inc/                       # Header files
│   │   │   ├── Src/                       # Main application code
│   │   │   └── Startup/                   # Startup assembly
│   │   ├── Datalogger_Lib/                # Custom libraries
│   │   │   ├── inc/                       # Library headers
│   │   │   │   ├── sht3x_driver.h         # SHT3X sensor driver
│   │   │   │   ├── ds3231_driver.h        # DS3231 RTC driver
│   │   │   │   ├── sd_buffer.h            # SD card buffering system
│   │   │   │   ├── ili9225_driver.h       # ILI9225 LCD driver
│   │   │   │   ├── uart_handler.h         # UART communication
│   │   │   │   └── json_formatter.h       # JSON data formatting
│   │   │   └── src/                       # Library implementations
│   │   └── Drivers/                       # STM32 HAL drivers
│   │       ├── CMSIS/                     # ARM CMSIS headers
│   │       └── STM32F1xx_HAL_Driver/      # STM32F1 HAL library
│   │
│   └── ESP32/                             # ESP32 Gateway Firmware
│       ├── README.md                      # 609 lines - gateway documentation
│       ├── CMakeLists.txt                 # ESP-IDF project configuration
│       ├── sdkconfig.defaults             # Default SDK configuration
│       ├── partitions.csv                 # Flash partition table
│       │
│       ├── components/                    # ESP-IDF Components (Modular)
│       │   ├── wifi_manager/              # WiFi connection management
│       │   │   ├── wifi_manager.c
│       │   │   ├── wifi_manager.h
│       │   │   ├── CMakeLists.txt
│       │   │   └── README.md
│       │   ├── mqtt_handler/              # MQTT v5 client
│       │   │   ├── mqtt_handler.c
│       │   │   ├── mqtt_handler.h
│       │   │   ├── Kconfig                # menuconfig options
│       │   │   └── CMakeLists.txt
│       │   ├── stm32_uart/                # UART bridge to STM32
│       │   │   ├── stm32_uart.c
│       │   │   ├── stm32_uart.h
│       │   │   └── CMakeLists.txt
│       │   ├── relay_control/             # GPIO relay control
│       │   │   ├── relay_control.c
│       │   │   ├── relay_control.h
│       │   │   └── CMakeLists.txt
│       │   ├── json_sensor_parser/        # Parse STM32 sensor JSON
│       │   │   ├── json_sensor_parser.c
│       │   │   ├── json_sensor_parser.h
│       │   │   └── CMakeLists.txt
│       │   ├── json_utils/                # JSON utilities
│       │   ├── ring_buffer/               # Circular buffer for UART
│       │   ├── button_handler/            # Physical button input
│       │   └── coap_handler/              # CoAP protocol (optional)
│       │
│       └── main/                          # Main application
│           ├── main.c                     # FreeRTOS task initialization
│           ├── CMakeLists.txt
│           ├── Kconfig.projbuild          # Project configuration options
│           └── idf_component.yml          # Component dependencies
│
└── web/                                   # Web Dashboard
    ├── README.md                          # 2148 lines - dashboard documentation
    ├── index.html                         # 2045 lines - main HTML (single-page app)
    ├── app.js                             # 3062 lines - JavaScript application logic
    ├── style.css                          # 629 lines - responsive CSS styling
    └── index_original_backup.html         # Original backup file
```

**Total Project Statistics:**
- **Documentation**: 100+ pages with 120+ Mermaid diagrams
- **STM32 Firmware**: ~8,000 lines C code (HAL-based)
- **ESP32 Firmware**: ~6,000 lines C code (ESP-IDF + FreeRTOS)
- **Web Dashboard**: 5,736 lines (HTML + JavaScript + CSS)
- **Configuration Files**: 20+ component configurations

---
## Comprehensive Documentation

### Component-Specific Documentation
- **[MQTT Broker Setup](broker/README.md)** - Docker deployment, authentication, monitoring, and security
- **[STM32 Firmware](firmware/STM32/README.md)** - 855 lines: Hardware setup, command interface, JSON format, SD buffering, display driver
- **[ESP32 Firmware](firmware/ESP32/README.md)** - 609 lines: Component architecture, MQTT topics, WiFi configuration, troubleshooting
- **[Web Dashboard](web/README.md)** - 2148 lines: Features, deployment, MQTT integration, Firebase setup, API reference

### Technical Diagrams (Mermaid Format)

#### STM32 Firmware Diagrams
- **[STM32 Flow Diagrams](documents/STM32/FLOW_DIAGRAM_STM32.md)** - 15 flowcharts: Initialization, command processing, sensor reads, UART, I2C, SD buffering, display updates
- **[STM32 Sequence Diagrams](documents/STM32/SEQUENCE_DIAGRAM_STM32.md)** - 10 sequences: Single read, periodic mode, time sync, SD write, error handling
- **[STM32 UML Diagrams](documents/STM32/UML_CLASS_DIAGRAM_STM32.md)** - 11 architecture diagrams: Class structure, module relationships, data flow

#### ESP32 Gateway Diagrams
- **[ESP32 Flow Diagrams](documents/ESP32/FLOW_DIAGRAM_ESP32.md)** - 15 flowcharts: WiFi connection, MQTT client, UART bridge, relay control, message routing
- **[ESP32 Sequence Diagrams](documents/ESP32/SEQUENCE_DIAGRAM_ESP32.md)** - 10 sequences: MQTT publish/subscribe, relay switching, WiFi reconnection
- **[ESP32 UML Diagrams](documents/ESP32/UML_CLASS_DIAGRAM_ESP32.md)** - 11 architecture diagrams: Component dependencies, FreeRTOS tasks, message queues

#### Complete Firmware System Diagrams
- **[Firmware Flow Diagrams](documents/firmware/FLOW_DIAGRAM_FIRMWARE.md)** - 31 comprehensive flowcharts: End-to-end data flow, offline buffering, error recovery
- **[Firmware Sequence Diagrams](documents/firmware/SEQUENCE_DIAGRAM_FIRMWARE.md)** - System-level interaction sequences
- **[Firmware UML Diagrams](documents/firmware/UML_DIAGRAM_FIRMWARE.md)** - Complete firmware architecture

#### Web Dashboard Diagrams
- **[Web Flow Diagrams](documents/WEB/FLOW_DIAGRAM_WEB.md)** - 15 flowcharts: App initialization, MQTT processing, device control, data management, chart updates
- **[Web Sequence Diagrams](documents/WEB/SEQUENCE_DIAGRAM_WEB.md)** - 10 sequences: MQTT connection, Firebase sync, relay control, time sync, error handling
- **[Web UML Diagrams](documents/WEB/UML_DIAGRAM_WEB.md)** - 11 architecture diagrams: Class diagram (12 classes), MQTT topics, Firebase structure, state management, UI hierarchy

#### Overall System Diagrams
- **[System Flow Diagrams](documents/SYSTEM/FLOW_DIAGRAM_SYSTEM.md)** - Complete system data flow from sensor to cloud
- **[System Sequence Diagrams](documents/SYSTEM/SEQUENCE_DIAGRAM_SYSTEM.md)** - End-to-end operational sequences
- **[System UML Diagrams](documents/SYSTEM/UML_DIAGRAM_SYSTEM.md)** - Complete system architecture with all components

**Total Documentation Stats:**
- **120+ Mermaid diagrams** covering all system aspects
- **100+ pages** of technical documentation
- **4 major subsystems** fully documented
- **Component READMEs** totaling 4,600+ lines

---

## Development Guide

### Adding New Sensor Types

**1. STM32 Driver Development:**
```c
// Create new driver in firmware/STM32/Datalogger_Lib/src/
// Example: bme280_driver.c

HAL_StatusTypeDef BME280_Init(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef BME280_ReadData(float *temp, float *hum, float *press);

// Add to datalogger_main.c
#include "bme280_driver.h"
BME280_Init(&hi2c1);
```

**2. Command Parser Update:**
```c
// In firmware/STM32/Core/Src/cmd_parser.c
if (strncmp(cmd, "READ_BME280", 11) == 0) {
    BME280_ReadData(&temp, &hum, &press);
    JSON_FormatBME280(buffer, temp, hum, press);
    UART_Transmit(buffer);
}
```

**3. ESP32 Message Forwarding:**
```c
// In firmware/ESP32/components/json_sensor_parser/
void parse_bme280_json(const char *json) {
    cJSON *root = cJSON_Parse(json);
    // Extract temperature, humidity, pressure
    // Publish to MQTT: datalogger/data/bme280
}
```

**4. Web Dashboard Integration:**
```javascript
// In web/app.js
client.on('message', (topic, message) => {
    if (topic === 'datalogger/data/bme280') {
        const data = JSON.parse(message.toString());
        updateBME280Display(data.temperature, data.humidity, data.pressure);
    }
});
```

---

### Code Style Guidelines

#### STM32 Firmware (C99)
```c
/**
 * @brief Reads temperature and humidity from SHT3X sensor
 * @param temp Pointer to store temperature value
 * @param hum Pointer to store humidity value
 * @retval HAL_StatusTypeDef HAL_OK on success, error code otherwise
 */
HAL_StatusTypeDef SHT3X_ReadData(float *temp, float *hum) {
    uint8_t data[6];
    HAL_StatusTypeDef status;
    
    // Function implementation with error checking
    status = HAL_I2C_Mem_Read(&hi2c1, SHT3X_ADDR, CMD_READ, 1, data, 6, 100);
    if (status != HAL_OK) {
        return status;
    }
    
    // Process data
    *temp = calculate_temperature(data);
    *hum = calculate_humidity(data + 3);
    
    return HAL_OK;
}
```

**Conventions:**
- Doxygen-style comments for all public functions
- HAL error checking on every peripheral call
- UPPERCASE for defines and macros
- snake_case for function names
- Prefix module name (e.g., `SHT3X_`, `DS3231_`)

---

#### ESP32 Firmware (ESP-IDF Style)
```c
#include "esp_log.h"
#include "esp_err.h"

static const char *TAG = "MQTT_HANDLER";

esp_err_t mqtt_handler_init(void) {
    ESP_LOGI(TAG, "Initializing MQTT handler");
    
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = CONFIG_MQTT_BROKER_URI,
        .credentials.username = CONFIG_MQTT_USERNAME,
        .credentials.authentication.password = CONFIG_MQTT_PASSWORD,
    };
    
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    if (client == NULL) {
        ESP_LOGE(TAG, "Failed to initialize MQTT client");
        return ESP_FAIL;
    }
    
    ESP_ERROR_CHECK(esp_mqtt_client_start(client));
    return ESP_OK;
}
```

**Conventions:**
- ESP-IDF logging macros (`ESP_LOGI`, `ESP_LOGE`, `ESP_LOGW`)
- `ESP_ERROR_CHECK()` for critical operations
- `esp_err_t` return type for error handling
- Component-specific TAG for logging
- Kconfig integration for configuration

---

#### Web Dashboard (Modern JavaScript ES6+)
```javascript
/**
 * MQTTManager handles all MQTT WebSocket communication
 */
class MQTTManager {
    constructor(config) {
        this.config = config;
        this.client = null;
        this.connected = false;
    }
    
    /**
     * Connect to MQTT broker via WebSocket
     * @returns {Promise<void>}
     */
    async connect() {
        try {
            this.client = mqtt.connect(this.config.broker, {
                username: this.config.username,
                password: this.config.password,
                clientId: this.config.clientId
            });
            
            this.client.on('connect', () => {
                console.log('MQTT connected');
                this.connected = true;
                this.subscribeToTopics();
            });
            
        } catch (error) {
            console.error('MQTT connection failed:', error);
            throw error;
        }
    }
}
```

**Conventions:**
- ES6 classes for modular architecture
- Async/await for asynchronous operations
- JSDoc comments for public methods
- Promise-based error handling
- Semantic variable naming

---

## Future Enhancements & Roadmap

### Phase 1: Core Improvements (Short-term)
- [ ] **OTA Firmware Updates** - Over-the-air updates for STM32 and ESP32
- [ ] **TLS/SSL Encryption** - Secure MQTT communication with certificates
- [ ] **User Authentication** - Multi-user web dashboard with role-based access
- [ ] **Data Compression** - Reduce bandwidth usage with gzip compression
- [ ] **Expanded SD Buffering** - Support larger SD cards (32GB+) with wear leveling

### Phase 2: Feature Expansion (Mid-term)
- [ ] **Multi-Sensor Support** - BME280, BMP180, DHT22, MQ-series gas sensors
- [ ] **CoAP Protocol** - Alternative to MQTT for constrained networks
- [ ] **Modbus Integration** - Industrial sensor protocol support
- [ ] **Alert System** - Email/SMS notifications on threshold violations
- [ ] **Data Export** - CSV, JSON, Excel export with timestamp filtering

### Phase 3: Advanced Features (Long-term)
- [ ] **Machine Learning** - Anomaly detection and predictive maintenance
- [ ] **Mesh Networking** - ESP-NOW multi-node data collection
- [ ] **Mobile Applications** - Native iOS and Android apps
- [ ] **Advanced Analytics** - Statistical analysis, trend prediction, data correlation
- [ ] **Edge Computing** - Local data processing on ESP32 before transmission

### Phase 4: Enterprise Features
- [ ] **Multi-Tenant System** - Support multiple sites/locations
- [ ] **RESTful API** - Third-party integration and automation
- [ ] **Grafana Integration** - Professional dashboard and alerting
- [ ] **Database Backend** - PostgreSQL/InfluxDB for large-scale deployments
- [ ] **Kubernetes Deployment** - Container orchestration for scalability

---

## Contributing

Contributions are welcome! Please follow these guidelines:

1. **Fork the repository** and create a feature branch
2. **Follow code style** guidelines for respective components
3. **Add unit tests** for new functionality
4. **Update documentation** (README, component docs, diagrams)
5. **Test thoroughly** on actual hardware before submitting PR
6. **Submit pull request** with detailed description of changes

### Development Setup
```bash
# Clone repository
git clone https://github.com/yourusername/DATALOGGER-Dev2.0.git
cd DATALOGGER-Dev2.0

# Install pre-commit hooks (optional)
pip install pre-commit
pre-commit install

# Build all components
cd firmware/STM32 && make all
cd ../ESP32 && idf.py build
cd ../../broker && docker-compose up -d
cd ../web && python -m http.server 8000
```

---

## License

This project is licensed under the **MIT License** - see [LICENSE.md](LICENSE.md) for complete details.

**Key Points:**
- ✅ Free for personal and commercial use
- ✅ Modify and distribute freely
- ✅ No warranty provided
- ⚠️ Attribution required (keep copyright notice)

---

## Support & Contact

### Getting Help
- **Issues**: Report bugs via [GitHub Issues](https://github.com/yourusername/DATALOGGER-Dev2.0/issues)
- **Discussions**: Ask questions in [GitHub Discussions](https://github.com/yourusername/DATALOGGER-Dev2.0/discussions)
- **Documentation**: Check component-specific README files first

### Reporting Bugs
Please include:
- Hardware configuration (STM32/ESP32 board models)
- Software versions (ESP-IDF, STM32CubeIDE, browser)
- Steps to reproduce the issue
- Serial monitor output / logs
- Expected vs. actual behavior

---

## Acknowledgments

### Open-Source Dependencies
- **[ESP-IDF](https://github.com/espressif/esp-idf)** - Espressif IoT Development Framework (Apache 2.0)
- **[STM32 HAL](https://www.st.com/en/embedded-software/stm32cube-mcu-mpu-packages.html)** - STMicroelectronics Hardware Abstraction Layer (BSD-3-Clause)
- **[Eclipse Mosquitto](https://mosquitto.org/)** - Open-source MQTT broker (EPL/EDL)
- **[MQTT.js](https://github.com/mqttjs/MQTT.js)** - JavaScript MQTT client library (MIT)
- **[Chart.js](https://www.chartjs.org/)** - JavaScript charting library (MIT)
- **[Firebase](https://firebase.google.com/)** - Google cloud platform and real-time database
- **[cJSON](https://github.com/DaveGamble/cJSON)** - Lightweight JSON parser (MIT)

### Hardware Manufacturers
- **STMicroelectronics** - STM32 microcontrollers
- **Espressif Systems** - ESP32 WiFi SoC
- **Sensirion** - SHT3X temperature/humidity sensors
- **Maxim Integrated** - DS3231 precision RTC

---

## Version History

### v2.0.0 (Current) - Complete System Rewrite
- ✨ Added SD card offline buffering (204,800 records)
- ✨ Added ILI9225 LCD local display (176×220px)
- ✨ Implemented component health monitoring
- ✨ Redesigned web dashboard (7 pages, single-page app)
- ✨ Added Firebase cloud database integration
- ✨ Complete documentation with 120+ Mermaid diagrams
- 🔧 Improved MQTT topic structure (6 topics)
- 🔧 Enhanced error handling and recovery
- 🔧 Optimized power consumption
- 📚 Comprehensive README with wiring diagrams

### v1.0.0 - Initial Release
- Basic STM32 sensor acquisition
- ESP32 WiFi gateway
- MQTT communication
- Simple web dashboard
- Periodic and single read modes

---

**Built with ❤️ for IoT environmental monitoring**

*Last Updated: 2024-12-31*
