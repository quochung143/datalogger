# SYSTEM - UML & Architecture Diagrams (Complete System Architecture)

Comprehensive UML and architecture diagrams showing the complete DATALOGGER system structure including all 4 subsystems, their interactions, deployment, and data flows.

---

## 1. Complete System Architecture (C4 Level 1 - Context)

```mermaid
graph TB
    subgraph External["External Systems"]
        User[("👤 User<br/>(Dashboard Access)")]
        Internet[("🌐 Internet<br/>(NTP Time)")]
        Cloud["☁️ Firebase<br/>(Cloud Storage)"]
    end
    
    subgraph DataloggerSystem["DATALOGGER System"]
        WebApp["🖥️ Web Dashboard<br/>(Browser Application)"]
        MQTTBroker["📡 MQTT Broker<br/>(Mosquitto)"]
        Gateway["📱 ESP32 Gateway<br/>(WiFi Bridge)"]
        Firmware["🎛️ STM32 Firmware<br/>(Data Acquisition)"]
    end
    
    subgraph Physical["Physical World"]
        Sensor["🌡️ SHT3X Sensor<br/>(Temperature/Humidity)"]
        Relay["⚡ Relay Module<br/>(Device Control)"]
        Device["🔌 Connected Device"]
    end
    
    User -->|HTTPS/WSS| WebApp
    WebApp -->|MQTT over WebSocket| MQTTBroker
    WebApp -->|Store/Query| Cloud
    WebApp -->|Get Time| Internet
    
    Gateway -->|MQTT TCP| MQTTBroker
    Gateway -->|UART 115200| Firmware
    Gateway -->|GPIO Control| Relay
    Relay -->|Power| Device
    
    Firmware -->|I2C Read| Sensor
    Sensor -->|Temp & Hum| Firmware
    
    style User fill:#90EE90
    style WebApp fill:#87CEEB
    style MQTTBroker fill:#FFD700
    style Gateway fill:#FFA07A
    style Firmware fill:#DDA0DD
    style Sensor fill:#F0E68C
    style Cloud fill:#B0C4DE
```

---

## 2. System Component Diagram

```mermaid
graph TB
    subgraph STM32_System["STM32 Subsystem"]
        STM32_Main["Main Application<br/>main.c"]
        
        subgraph Drivers["Hardware Drivers"]
            SHT3X_Drv["SHT3X Driver<br/>sht3x.c"]
            DS3231_Drv["DS3231 Driver<br/>ds3231.c"]
            SD_Drv["SD Card Driver<br/>sd_card.c"]
            LCD_Drv["ILI9225 Driver<br/>ili9225.c"]
        end
        
        subgraph Managers["System Managers"]
            Data_Mgr["Data Manager<br/>data_manager.c"]
            SD_Mgr["SD Manager<br/>sd_card_manager.c"]
            Display_Mgr["Display Manager<br/>display.c"]
        end
        
        subgraph Communication["Communication"]
            UART_Handler["UART Handler<br/>uart.c"]
            CMD_Parser["Command Parser<br/>cmd_parser.c"]
            Ring_Buf["Ring Buffer<br/>ring_buffer.c"]
        end
    end
    
    subgraph ESP32_System["ESP32 Subsystem"]
        ESP32_Main["Main Task<br/>main.c"]
        
        subgraph ESP32_Components["FreeRTOS Components"]
            WiFi_Mgr["WiFi Manager<br/>wifi_manager"]
            MQTT_Handler["MQTT Handler<br/>mqtt_handler"]
            UART_Bridge["UART Bridge<br/>stm32_uart"]
            Relay_Ctrl["Relay Control<br/>relay_control"]
            JSON_Parser["JSON Parser<br/>json_sensor_parser"]
        end
    end
    
    subgraph Broker_System["MQTT Broker"]
        Mosquitto["Mosquitto 2.x<br/>(Docker)"]
        Auth["Authentication<br/>(bcrypt)"]
        Persistence["Message<br/>Persistence"]
        WebSocket["WebSocket<br/>Protocol"]
    end
    
    subgraph Web_System["Web Dashboard"]
        Web_Main["WebApplication<br/>app.js"]
        
        subgraph Web_Managers["Managers"]
            MQTT_Mgr["MQTTManager"]
            Firebase_Mgr["FirebaseManager"]
            Chart_Mgr["ChartManager"]
            State_Mgr["StateManager"]
            UI_Ctrl["UIController"]
            Health_Mon["HealthMonitor"]
        end
    end
    
    STM32_Main --> Drivers
    STM32_Main --> Managers
    STM32_Main --> Communication
    
    Drivers --> Data_Mgr
    Data_Mgr --> SD_Mgr
    Data_Mgr --> Display_Mgr
    Data_Mgr --> UART_Handler
    
    UART_Handler --> Ring_Buf
    Ring_Buf --> CMD_Parser
    CMD_Parser --> STM32_Main
    
    ESP32_Main --> ESP32_Components
    WiFi_Mgr --> MQTT_Handler
    MQTT_Handler --> JSON_Parser
    JSON_Parser --> UART_Bridge
    UART_Bridge -.UART.-> UART_Handler
    
    MQTT_Handler -.MQTT.-> Mosquitto
    Auth --> Mosquitto
    Persistence --> Mosquitto
    WebSocket --> Mosquitto
    
    Web_Main --> Web_Managers
    MQTT_Mgr -.WebSocket.-> Mosquitto
    Firebase_Mgr -.HTTPS.-> Firebase_Mgr
    
    style STM32_System fill:#DDA0DD,stroke:#9370DB,stroke-width:3px
    style ESP32_System fill:#FFA07A,stroke:#FF6347,stroke-width:3px
    style Broker_System fill:#FFD700,stroke:#FFA500,stroke-width:3px
    style Web_System fill:#87CEEB,stroke:#4682B4,stroke-width:3px
```

---

## 3. Deployment Diagram

```mermaid
graph TB
    subgraph Hardware["Hardware Layer"]
        subgraph MCU_Board["STM32 Blue Pill Board"]
            STM32["STM32F103C8T6<br/>ARM Cortex-M3<br/>72MHz, 64KB Flash"]
            STM32_Periph["Peripherals<br/>I2C1, SPI1<br/>UART1, TIM2"]
        end
        
        subgraph ESP_Board["ESP32 Development Board"]
            ESP32["ESP32-WROOM-32<br/>Dual-core Xtensa<br/>240MHz, WiFi"]
            ESP32_Periph["Peripherals<br/>UART2, GPIO4<br/>WiFi Radio"]
        end
        
        subgraph Sensors_HW["Sensor Hardware"]
            SHT3X["SHT3X<br/>I2C 0x44"]
            DS3231["DS3231<br/>I2C 0x68<br/>+ CR2032"]
            SD_Card["MicroSD Card<br/>FAT32<br/>1GB+"]
            LCD_HW["ILI9225<br/>176×220<br/>SPI"]
            Relay_HW["5V Relay<br/>GPIO4<br/>NO/NC"]
        end
    end
    
    subgraph Network["Network Layer"]
        Router["WiFi Router<br/>2.4GHz<br/>SSID: xxx"]
        
        subgraph Server["Server (Docker Host)"]
            Docker["Docker Engine"]
            Broker_Container["mosquitto-broker<br/>Container<br/>Port 1883, 8083"]
            Volumes["Docker Volumes<br/>- config<br/>- data<br/>- logs"]
        end
    end
    
    subgraph Client["Client Layer"]
        Browser["Web Browser<br/>(Chrome/Firefox)<br/>User Device"]
        Firebase_Cloud["Firebase RTDB<br/>(Google Cloud)<br/>Realtime Database"]
    end
    
    STM32_Periph -->|I2C| SHT3X
    STM32_Periph -->|I2C| DS3231
    STM32_Periph -->|SPI| SD_Card
    STM32_Periph -->|SPI| LCD_HW
    STM32_Periph -->|UART<br/>115200bps| ESP32_Periph
    
    ESP32_Periph -->|GPIO| Relay_HW
    ESP32_Periph -->|WiFi<br/>802.11n| Router
    
    Router -->|Ethernet/WiFi| Docker
    Docker --> Broker_Container
    Broker_Container --> Volumes
    
    Browser -->|WebSocket<br/>ws://host:8083| Broker_Container
    Browser -->|HTTPS<br/>Firebase SDK| Firebase_Cloud
    
    style Hardware fill:#FFE4E1
    style Network fill:#E0FFFF
    style Client fill:#F0FFF0
    style STM32 fill:#DDA0DD
    style ESP32 fill:#FFA07A
    style Broker_Container fill:#FFD700
```

---

## 4. Network Topology Diagram

```mermaid
graph TB
    subgraph Internet_Zone["Internet"]
        Firebase["Firebase<br/>Realtime Database<br/>firebaseio.com"]
        NTP["NTP Servers<br/>pool.ntp.org"]
    end
    
    subgraph LAN["Local Area Network (192.168.1.0/24)"]
        Router["WiFi Router<br/>192.168.1.1<br/>Gateway + DHCP"]
        
        subgraph Server_Subnet["Server Subnet"]
            DockerHost["Docker Host<br/>192.168.1.100"]
            
            subgraph Docker_Network["Docker Bridge (172.17.0.0/16)"]
                Broker["MQTT Broker<br/>172.17.0.2<br/>Port 1883, 8083"]
            end
        end
        
        subgraph IoT_Subnet["IoT Device Subnet"]
            ESP32_Dev["ESP32 Gateway<br/>192.168.1.150<br/>WiFi Client"]
        end
        
        subgraph Client_Subnet["Client Subnet"]
            Laptop["User Laptop<br/>192.168.1.50"]
            Phone["Mobile Phone<br/>192.168.1.51"]
        end
        
        subgraph Embedded["Embedded (No IP)"]
            STM32_Dev["STM32 Board<br/>(UART Only)"]
        end
    end
    
    Router -->|Port Forward<br/>8883:1883<br/>8083:8083| DockerHost
    DockerHost -->|Docker NAT| Broker
    
    ESP32_Dev -->|MQTT TCP<br/>192.168.1.100:1883| Broker
    ESP32_Dev -.UART 115200.-> STM32_Dev
    
    Laptop -->|WebSocket<br/>ws://192.168.1.100:8083| Broker
    Phone -->|WebSocket<br/>ws://192.168.1.100:8083| Broker
    
    Laptop -->|HTTPS| Firebase
    Phone -->|HTTPS| Firebase
    
    Laptop -->|NTP| NTP
    
    Router -->|Internet| Firebase
    Router -->|Internet| NTP
    
    style Internet_Zone fill:#FFE4B5
    style LAN fill:#E0FFFF
    style Docker_Network fill:#F0F8FF
    style Broker fill:#FFD700
    style ESP32_Dev fill:#FFA07A
    style STM32_Dev fill:#DDA0DD
```

---

## 5. Data Flow Architecture

```mermaid
graph LR
    subgraph Sources["Data Sources"]
        Sensor["🌡️ SHT3X Sensor<br/>±0.2°C, ±2% RH"]
        RTC["⏰ DS3231 RTC<br/>Unix Timestamp"]
    end
    
    subgraph Acquisition["Data Acquisition"]
        STM32["STM32 Firmware<br/>- I2C Read<br/>- JSON Format<br/>- Validation"]
    end
    
    subgraph Buffering["Offline Buffering"]
        SD["SD Card<br/>204,800 records<br/>512B blocks"]
        Memory["RAM Buffer<br/>256B Ring<br/>UART RX"]
    end
    
    subgraph Transport["Data Transport"]
        UART["UART Link<br/>115200 bps<br/>8N1"]
        ESP32["ESP32 Gateway<br/>- Parse JSON<br/>- WiFi Bridge"]
    end
    
    subgraph Distribution["Data Distribution"]
        MQTT["MQTT Broker<br/>- Topic Routing<br/>- QoS 0/1<br/>- Persistence"]
    end
    
    subgraph Storage["Data Storage"]
        Firebase["Firebase RTDB<br/>- Historical<br/>- Queryable<br/>- Cloud Backup"]
    end
    
    subgraph Visualization["Data Visualization"]
        Chart["Chart.js<br/>Real-time Line<br/>50-point window"]
        Table["Data Table<br/>Live Updates<br/>Filtering"]
        Display["ILI9225 LCD<br/>Local Display<br/>176×220 px"]
    end
    
    Sensor -->|I2C 100kHz| STM32
    RTC -->|I2C 100kHz| STM32
    
    STM32 -->|Online| UART
    STM32 -->|Offline| SD
    SD -->|Sync| UART
    
    UART --> Memory
    Memory --> ESP32
    
    ESP32 -->|WiFi| MQTT
    MQTT -->|WebSocket| Chart
    MQTT -->|WebSocket| Table
    MQTT -.Store.-> Firebase
    
    STM32 -->|SPI| Display
    
    style Sources fill:#F0E68C
    style Acquisition fill:#DDA0DD
    style Buffering fill:#FFB6C1
    style Transport fill:#FFA07A
    style Distribution fill:#FFD700
    style Storage fill:#B0C4DE
    style Visualization fill:#87CEEB
```

---

## 6. STM32 State Machine Diagram

```mermaid
stateDiagram-v2
    [*] --> INIT: Power On
    
    INIT --> IDLE: Init Success
    INIT --> ERROR: Init Failed
    
    IDLE --> SINGLE_READ: Single Command
    IDLE --> PERIODIC_START: Periodic ON
    IDLE --> CONFIG: Set Time/Config
    
    SINGLE_READ --> MEASURING: Trigger Sensor
    MEASURING --> PROCESSING: Data Ready
    PROCESSING --> TRANSMITTING: Format JSON
    TRANSMITTING --> IDLE: TX Complete
    TRANSMITTING --> BUFFERING: WiFi Down
    BUFFERING --> IDLE: Buffered
    
    PERIODIC_START --> PERIODIC_ACTIVE: Timer Started
    PERIODIC_ACTIVE --> PERIODIC_MEASURE: Timer Interrupt
    PERIODIC_MEASURE --> PERIODIC_PROCESS: Data Ready
    PERIODIC_PROCESS --> PERIODIC_TX: Format JSON
    PERIODIC_TX --> PERIODIC_ACTIVE: TX Complete
    PERIODIC_TX --> PERIODIC_BUFFER: WiFi Down
    PERIODIC_BUFFER --> PERIODIC_ACTIVE: Buffered
    PERIODIC_ACTIVE --> IDLE: Stop Command
    
    CONFIG --> IDLE: Config Complete
    
    ERROR --> RECOVERY: Retry Init
    RECOVERY --> IDLE: Recovered
    RECOVERY --> ERROR: Failed
    
    BUFFERING --> SYNCING: WiFi Restored
    PERIODIC_BUFFER --> SYNCING: WiFi Restored
    SYNCING --> IDLE: Sync Complete
    
    note right of IDLE
        - Sensor Ready
        - Waiting Commands
        - Display Active
    end note
    
    note right of PERIODIC_ACTIVE
        - Timer Running
        - Continuous Measurement
        - Real-time TX
    end note
    
    note right of BUFFERING
        - SD Card Writing
        - Offline Mode
        - Data Preserved
    end note
```

---

## 7. ESP32 State Machine Diagram

```mermaid
stateDiagram-v2
    [*] --> BOOT: Power On
    
    BOOT --> WIFI_INIT: FreeRTOS Start
    WIFI_INIT --> WIFI_CONNECTING: Start WiFi
    
    WIFI_CONNECTING --> WIFI_CONNECTED: Association OK
    WIFI_CONNECTING --> WIFI_RETRY: Timeout
    WIFI_RETRY --> WIFI_CONNECTING: Backoff Delay
    WIFI_RETRY --> WIFI_FAILED: Max Retries
    
    WIFI_CONNECTED --> MQTT_INIT: Got IP Address
    MQTT_INIT --> MQTT_CONNECTING: Connect Broker
    
    MQTT_CONNECTING --> MQTT_CONNECTED: CONNACK
    MQTT_CONNECTING --> MQTT_RETRY: Timeout
    MQTT_RETRY --> MQTT_CONNECTING: Retry Delay
    
    MQTT_CONNECTED --> OPERATIONAL: Subscribed
    
    OPERATIONAL --> PROCESSING_CMD: Command Received
    OPERATIONAL --> PROCESSING_DATA: Data from STM32
    OPERATIONAL --> RELAY_CONTROL: Relay Command
    
    PROCESSING_CMD --> OPERATIONAL: Forwarded to STM32
    PROCESSING_DATA --> OPERATIONAL: Published to MQTT
    RELAY_CONTROL --> OPERATIONAL: GPIO Updated
    
    OPERATIONAL --> MQTT_DISCONNECTED: Connection Lost
    OPERATIONAL --> WIFI_DISCONNECTED: WiFi Lost
    
    MQTT_DISCONNECTED --> MQTT_CONNECTING: Auto Reconnect
    WIFI_DISCONNECTED --> WIFI_CONNECTING: Auto Reconnect
    
    WIFI_FAILED --> RECOVERY: Soft Reset
    RECOVERY --> WIFI_INIT: Reset Complete
    
    note right of OPERATIONAL
        - Bidirectional Bridge
        - UART ↔ MQTT
        - Relay Control
        - State Reporting
    end note
    
    note right of MQTT_CONNECTED
        - Subscribed Topics:
          * datalogger/command
          * datalogger/relay
          * datalogger/time/sync
    end note
```

---

## 8. Web Dashboard State Machine

```mermaid
stateDiagram-v2
    [*] --> LOADING: Page Load
    
    LOADING --> INIT_MQTT: HTML Parsed
    INIT_MQTT --> INIT_FIREBASE: MQTT Connecting
    
    INIT_FIREBASE --> CONNECTING: Services Init
    
    CONNECTING --> CONNECTED: All Connected
    CONNECTING --> PARTIAL: MQTT Only
    CONNECTING --> OFFLINE: All Failed
    
    CONNECTED --> ACTIVE: Receiving Data
    
    ACTIVE --> DISPLAYING: Update UI
    ACTIVE --> COMMANDING: User Action
    ACTIVE --> QUERYING: Data Request
    ACTIVE --> EXPORTING: Export Request
    
    DISPLAYING --> ACTIVE: Render Complete
    COMMANDING --> ACTIVE: Command Sent
    QUERYING --> ACTIVE: Query Complete
    EXPORTING --> ACTIVE: Export Done
    
    ACTIVE --> DEGRADED: MQTT Lost
    DEGRADED --> ACTIVE: MQTT Restored
    DEGRADED --> OFFLINE: Firebase Lost
    
    OFFLINE --> CONNECTING: Auto Reconnect
    PARTIAL --> CONNECTED: Firebase OK
    
    note right of ACTIVE
        - Real-time Charts
        - Live Data Table
        - Component Health
        - User Interactions
    end note
    
    note right of DEGRADED
        - MQTT Down
        - Firebase Active
        - Historical Data Only
        - No Real-time
    end note
```

---

*Continued in next section...*
