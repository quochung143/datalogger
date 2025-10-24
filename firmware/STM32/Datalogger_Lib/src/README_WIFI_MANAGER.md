# MQTT Manager Library (wifi_manager)

## Overview

The MQTT Manager Library (formerly named "WiFi Manager") provides MQTT connection state tracking for the Datalogger system. It monitors the connection status between the STM32 microcontroller and the MQTT broker via the ESP32 module, allowing the system to conditionally transmit data only when the MQTT connection is active.

**Note**: Despite the filename "wifi_manager", this library tracks MQTT connection state, not WiFi connectivity. The ESP32 handles WiFi management independently.

## Files

- **wifi_manager.c**: MQTT state management implementation
- **wifi_manager.h**: MQTT state API and state definitions

## MQTT Connection States

### State Enumeration

```c
typedef enum {
    MQTT_STATE_DISCONNECTED = 0,
    MQTT_STATE_CONNECTED = 1
} MQTT_State_t;
```

**State Descriptions**:

| State | Value | Meaning | Data Transmission |
|-------|-------|---------|-------------------|
| `MQTT_STATE_DISCONNECTED` | 0 | ESP32 not connected to MQTT broker | Blocked - data buffered locally |
| `MQTT_STATE_CONNECTED` | 1 | ESP32 connected to MQTT broker | Allowed - data sent to cloud |

## API Functions

### Get MQTT Connection State

```c
MQTT_State_t mqtt_manager_get_state(void);
```

Returns the current MQTT connection state.

**Returns**:
- `MQTT_STATE_DISCONNECTED`: MQTT broker unreachable or ESP32 offline
- `MQTT_STATE_CONNECTED`: MQTT broker connected, ready for data transmission

**Usage Example**:
```c
#include "wifi_manager.h"
#include "sensor_json_output.h"
#include "sd_card_manager.h"

void SendMeasurement(void)
{
    float temp, hum;
    SHT3X_ReadData(&g_sht3x, &temp, &hum);
    
    if (mqtt_manager_get_state() == MQTT_STATE_CONNECTED)
    {
        // MQTT available - send to cloud
        sensor_json_output_send(0, temp, hum, "PERIODIC");
    }
    else
    {
        // MQTT unavailable - buffer to SD card
        SDCardManager_WriteData(DS3231_GetUnixTime(), temp, hum, "PERIODIC");
    }
}
```

## State Management

### State Variable

```c
static MQTT_State_t mqtt_state = MQTT_STATE_DISCONNECTED;
```

**Default State**: Disconnected (safe default, prevents data loss to unavailable broker).

### State Update Mechanism

**Current Implementation**: State is updated internally by the library based on communication with the ESP32.

**Typical Update Pattern**:
```c
void UpdateMQTTState(void)
{
    // Check ESP32 status via UART
    if (ESP32_IsConnected())
    {
        mqtt_state = MQTT_STATE_CONNECTED;
    }
    else
    {
        mqtt_state = MQTT_STATE_DISCONNECTED;
    }
}
```

**Alternative**: ESP32 could send status messages via UART:

```
ESP32 → STM32: "MQTT CONNECTED\r\n"
STM32 updates: mqtt_state = MQTT_STATE_CONNECTED

ESP32 → STM32: "MQTT DISCONNECTED\r\n"
STM32 updates: mqtt_state = MQTT_STATE_DISCONNECTED
```

## Integration with System Components

### Data Transmission Flow

**Connected State**:
```
Sensor Measurement
       ↓
 Check MQTT State
       ↓
  [CONNECTED] → Format JSON → Send via UART to ESP32 → ESP32 publishes to broker
```

**Disconnected State**:
```
Sensor Measurement
       ↓
 Check MQTT State
       ↓
  [DISCONNECTED] → Write to SD card buffer → Retry later when connected
```

### Main Loop Integration

```c
int main(void)
{
    // Initialization
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART1_UART_Init();
    
    SHT3X_Init(&g_sht3x, &hi2c1, 0x44);
    SDCardManager_Init();
    UART_Init(&huart1);
    
    // Main loop
    while (1)
    {
        if (Flag_Periodic)  // Timer-triggered measurement
        {
            Flag_Periodic = 0;
            
            // Measure sensor data
            float temp, hum;
            SHT3X_ReadData(&g_sht3x, &temp, &hum);
            uint32_t timestamp = DS3231_GetUnixTime();
            
            // Check MQTT state before transmission
            if (mqtt_manager_get_state() == MQTT_STATE_CONNECTED)
            {
                // Send to cloud
                sensor_json_output_send(timestamp, temp, hum, "PERIODIC");
            }
            else
            {
                // Buffer locally
                SDCardManager_WriteData(timestamp, temp, hum, "PERIODIC");
            }
        }
        
        // Process UART commands
        UART_Handle();
        if (Flag_UART)
        {
            Flag_UART = 0;
            COMMAND_EXECUTE((char *)buff);
        }
    }
}
```

## MQTT Connection Monitoring

### Heartbeat Mechanism

ESP32 can periodically send heartbeat messages to indicate MQTT connection status:

**ESP32 Code** (send every 10 seconds):
```c
void mqtt_heartbeat_task(void)
{
    while (1)
    {
        if (mqtt_client_is_connected())
        {
            uart_write_bytes(UART_NUM_1, "MQTT CONNECTED\r\n", 16);
        }
        else
        {
            uart_write_bytes(UART_NUM_1, "MQTT DISCONNECTED\r\n", 19);
        }
        
        vTaskDelay(10000 / portTICK_PERIOD_MS);  // 10 seconds
    }
}
```

**STM32 Code** (process heartbeat in command handler):
```c
void MQTT_STATUS_PARSER(int argc, char **argv)
{
    if (argc == 2)
    {
        if (strcmp(argv[1], "CONNECTED") == 0)
        {
            mqtt_manager_set_state(MQTT_STATE_CONNECTED);
            printf("MQTT broker connected\r\n");
        }
        else if (strcmp(argv[1], "DISCONNECTED") == 0)
        {
            mqtt_manager_set_state(MQTT_STATE_DISCONNECTED);
            printf("MQTT broker disconnected\r\n");
        }
    }
}
```

### Timeout-Based Disconnect Detection

If heartbeat not received within timeout, assume disconnected:

```c
static uint32_t last_heartbeat_time = 0;

void CheckMQTTTimeout(void)
{
    if (mqtt_manager_get_state() == MQTT_STATE_CONNECTED)
    {
        if ((HAL_GetTick() - last_heartbeat_time) > 30000)  // 30 seconds timeout
        {
            mqtt_manager_set_state(MQTT_STATE_DISCONNECTED);
            printf("MQTT heartbeat timeout, assuming disconnected\r\n");
        }
    }
}

void UpdateHeartbeat(void)
{
    last_heartbeat_time = HAL_GetTick();
}
```

## Data Synchronization Strategy

### Offline Buffering

When MQTT is disconnected, data is buffered to SD card:

```c
void PeriodicMeasurement(void)
{
    float temp, hum;
    SHT3X_ReadData(&g_sht3x, &temp, &hum);
    uint32_t timestamp = DS3231_GetUnixTime();
    
    if (mqtt_manager_get_state() == MQTT_STATE_CONNECTED)
    {
        // Online mode - send immediately
        sensor_json_output_send(timestamp, temp, hum, "PERIODIC");
    }
    else
    {
        // Offline mode - buffer to SD card
        SDCardManager_WriteData(timestamp, temp, hum, "PERIODIC");
    }
}
```

### Reconnection and Synchronization

When MQTT reconnects, flush buffered data:

```c
void OnMQTTReconnected(void)
{
    mqtt_manager_set_state(MQTT_STATE_CONNECTED);
    
    printf("MQTT reconnected, syncing buffered data...\r\n");
    
    // Send all buffered data
    while (SDCardManager_GetRecordCount() > 0)
    {
        uint32_t ts;
        float temp, hum;
        char mode[16];
        
        if (SDCardManager_ReadData(&ts, &temp, &hum, mode) == 0)
        {
            sensor_json_output_send(ts, temp, hum, mode);
            SDCardManager_RemoveRecord();  // Remove from buffer after sending
            
            HAL_Delay(100);  // Rate limit to avoid overwhelming broker
        }
        else
        {
            break;  // Read error, stop syncing
        }
    }
    
    printf("Synchronization complete\r\n");
}
```

## Usage Examples

### Example 1: Conditional Data Transmission

```c
void SingleMeasurement(void)
{
    // Measure sensor
    float temp, hum;
    SHT3X_ReadData(&g_sht3x, &temp, &hum);
    
    // Check MQTT state
    if (mqtt_manager_get_state() == MQTT_STATE_CONNECTED)
    {
        // Transmit to cloud
        sensor_json_output_send(0, temp, hum, "SINGLE");
        printf("Data sent to MQTT broker\r\n");
    }
    else
    {
        // Cannot send, inform user
        printf("MQTT disconnected, data not transmitted\r\n");
    }
}
```

### Example 2: Status Display

```c
void DisplayMQTTStatus(void)
{
    MQTT_State_t state = mqtt_manager_get_state();
    
    if (state == MQTT_STATE_CONNECTED)
    {
        DISPLAY_DrawString(10, 100, "MQTT: ONLINE", FONT_11X18, COLOR_GREEN);
    }
    else
    {
        DISPLAY_DrawString(10, 100, "MQTT: OFFLINE", FONT_11X18, COLOR_RED);
    }
}
```

### Example 3: Mode-Dependent Behavior

```c
void ConfigureSamplingRate(void)
{
    if (mqtt_manager_get_state() == MQTT_STATE_CONNECTED)
    {
        // High-frequency sampling when connected
        SetPeriodicInterval(5);  // 5 seconds
    }
    else
    {
        // Low-frequency sampling to save SD card space when offline
        SetPeriodicInterval(60);  // 60 seconds
    }
}
```

## Performance Characteristics

### Function Overhead

**mqtt_manager_get_state()**:
- Returns static variable
- Execution time: ~10 clock cycles (~0.1 µs @ 72 MHz)

**Negligible overhead**, safe to call frequently.

### State Update Latency

**Heartbeat-Based**:
- Update interval: 10 seconds (configurable)
- Maximum detection delay: 10 seconds

**Command-Based**:
- Update latency: Command reception time (~1 ms)
- Immediate response to ESP32 state changes

## Best Practices

### 1. Always Check State Before Transmission

```c
// GOOD: Check before sending
if (mqtt_manager_get_state() == MQTT_STATE_CONNECTED)
{
    sensor_json_output_send(ts, temp, hum, mode);
}

// BAD: Send without checking (data loss risk)
sensor_json_output_send(ts, temp, hum, mode);  // May fail silently
```

### 2. Buffer Data When Disconnected

```c
// Ensure no data loss
if (mqtt_manager_get_state() == MQTT_STATE_CONNECTED)
{
    sensor_json_output_send(ts, temp, hum, mode);
}
else
{
    SDCardManager_WriteData(ts, temp, hum, mode);  // Buffer for later
}
```

### 3. Handle Reconnection Events

```c
static MQTT_State_t previous_state = MQTT_STATE_DISCONNECTED;

void MonitorMQTTState(void)
{
    MQTT_State_t current_state = mqtt_manager_get_state();
    
    if (current_state == MQTT_STATE_CONNECTED && 
        previous_state == MQTT_STATE_DISCONNECTED)
    {
        // Reconnection detected
        printf("MQTT reconnected\r\n");
        SyncBufferedData();
    }
    
    previous_state = current_state;
}
```

### 4. Implement Timeout Protection

```c
// Detect stale connections
static uint32_t last_mqtt_activity = 0;

void UpdateMQTTActivity(void)
{
    last_mqtt_activity = HAL_GetTick();
}

void CheckMQTTStale(void)
{
    if (mqtt_manager_get_state() == MQTT_STATE_CONNECTED)
    {
        if ((HAL_GetTick() - last_mqtt_activity) > 60000)  // 60 seconds
        {
            printf("WARNING: No MQTT activity for 60 seconds\r\n");
            mqtt_manager_set_state(MQTT_STATE_DISCONNECTED);
        }
    }
}
```

## Error Handling

### Connection Loss Detection

```c
void DetectConnectionLoss(void)
{
    static uint32_t last_heartbeat = 0;
    
    if (mqtt_manager_get_state() == MQTT_STATE_CONNECTED)
    {
        if ((HAL_GetTick() - last_heartbeat) > 30000)
        {
            // No heartbeat for 30 seconds
            mqtt_manager_set_state(MQTT_STATE_DISCONNECTED);
            printf("MQTT connection lost\r\n");
        }
    }
}

void OnHeartbeatReceived(void)
{
    last_heartbeat = HAL_GetTick();
}
```

### State Change Logging

```c
void LogMQTTStateChange(MQTT_State_t new_state)
{
    static MQTT_State_t old_state = MQTT_STATE_DISCONNECTED;
    
    if (new_state != old_state)
    {
        uint32_t timestamp = DS3231_GetUnixTime();
        
        if (new_state == MQTT_STATE_CONNECTED)
        {
            printf("[%lu] MQTT CONNECTED\r\n", timestamp);
        }
        else
        {
            printf("[%lu] MQTT DISCONNECTED\r\n", timestamp);
        }
        
        old_state = new_state;
    }
}
```

## MQTT Topics and ESP32 Interaction

### STM32 → ESP32 → MQTT Broker

**Single Mode Data Transmission**:
```
STM32: sensor_json_output_send() → UART → ESP32 receives JSON
ESP32: mqtt_client_publish("datalogger/sensor/data", json_string)
MQTT Broker: Forwards to topic "datalogger/sensor/data"
```

**ESP32 → STM32 State Updates**:
```
MQTT Broker: Publishes command to "datalogger/cmd"
ESP32: Subscribes to "datalogger/cmd", receives command
ESP32: Forwards command via UART → STM32
STM32: COMMAND_EXECUTE() processes command
```

### Topic Structure

| Topic | Direction | Purpose | Example Payload |
|-------|-----------|---------|-----------------|
| `datalogger/sensor/data` | STM32 → Broker | Sensor data | `{"timestamp":1710000000,"temperature":25.50,...}` |
| `datalogger/cmd` | Broker → STM32 | Commands | `SINGLE`, `PERIODIC ON` |
| `datalogger/status` | STM32 → Broker | System status | `{"mqtt":"connected","sd_buffer":1234}` |
| `datalogger/esp32/status` | ESP32 → Broker | ESP32 heartbeat | `{"rssi":-45,"uptime":3600}` |

## Memory Footprint

### RAM Usage

- State variable: 1 byte (`MQTT_State_t`)
- **Total**: 1 byte

### Flash Usage

- Function code: ~50 bytes

**Extremely lightweight library.**

## Debugging

### Print State Periodically

```c
void DebugPrintMQTTState(void)
{
    MQTT_State_t state = mqtt_manager_get_state();
    
    printf("MQTT State: %s\r\n", 
           (state == MQTT_STATE_CONNECTED) ? "CONNECTED" : "DISCONNECTED");
}
```

### State Change Callback

```c
typedef void (*mqtt_state_callback_t)(MQTT_State_t new_state);

static mqtt_state_callback_t state_change_callback = NULL;

void mqtt_manager_register_callback(mqtt_state_callback_t callback)
{
    state_change_callback = callback;
}

void mqtt_manager_set_state(MQTT_State_t new_state)
{
    if (mqtt_state != new_state)
    {
        mqtt_state = new_state;
        
        if (state_change_callback != NULL)
        {
            state_change_callback(new_state);  // Notify application
        }
    }
}
```

**Usage**:
```c
void OnMQTTStateChanged(MQTT_State_t new_state)
{
    printf("MQTT state changed to: %s\r\n", 
           (new_state == MQTT_STATE_CONNECTED) ? "CONNECTED" : "DISCONNECTED");
    
    if (new_state == MQTT_STATE_CONNECTED)
    {
        SyncBufferedData();
    }
}

int main(void)
{
    // ...
    mqtt_manager_register_callback(OnMQTTStateChanged);
    // ...
}
```

## Dependencies

### Required

- None (standalone library)

### Used By

- **main.c**: Main loop checks state for conditional data transmission
- **sensor_json_output.c**: May check state before sending JSON

### Related

- **uart.c**: Receives MQTT status messages from ESP32
- **sd_card_manager.c**: Buffers data when MQTT unavailable

## Limitations

### No Automatic Reconnection

This library only tracks state. Actual reconnection is handled by the ESP32.

**STM32 cannot reconnect MQTT** - it relies on ESP32 to manage WiFi and MQTT connections.

### No QoS Management

State is binary (connected/disconnected). Quality of Service (QoS) levels and message delivery guarantees are handled by the ESP32 MQTT client.

### No Direct Broker Communication

STM32 does not communicate with the MQTT broker directly. All MQTT operations are proxied through the ESP32 via UART.

## Communication Architecture

```
┌───────────┐         UART          ┌───────────┐         WiFi/MQTT          ┌─────────────┐
│   STM32   │ ←──────────────────→  │   ESP32   │ ←────────────────────────→ │ MQTT Broker │
│ (Sensors) │  Commands/Data/Status  │  (WiFi)   │   Pub/Sub on Topics       │   (Cloud)   │
└───────────┘                        └───────────┘                            └─────────────┘
      ↑                                      ↑
      │                                      │
mqtt_manager_get_state()           mqtt_client_is_connected()
```

**Responsibilities**:
- **STM32**: Read sensors, check MQTT state, buffer data if offline
- **ESP32**: Manage WiFi, connect to MQTT broker, forward commands/data
- **MQTT Broker**: Distribute messages to web dashboard and other clients

## Future Enhancements

### 1. Enhanced State Information

```c
typedef struct {
    MQTT_State_t state;
    uint32_t last_connected_time;
    uint32_t reconnect_count;
    int8_t rssi;  // WiFi signal strength from ESP32
} MQTT_Status_t;

MQTT_Status_t mqtt_manager_get_status(void);
```

### 2. Automatic Sync on Reconnect

```c
void mqtt_manager_enable_auto_sync(bool enable);
```

### 3. Connection Quality Metrics

```c
typedef struct {
    uint32_t uptime;
    uint32_t disconnect_events;
    uint32_t messages_sent;
    uint32_t messages_failed;
} MQTT_Statistics_t;

MQTT_Statistics_t mqtt_manager_get_statistics(void);
```

## Summary

The MQTT Manager Library provides:
- Simple binary state tracking (CONNECTED/DISCONNECTED)
- Lightweight API (single function, 1-byte state)
- Enables conditional data transmission based on MQTT availability
- Supports offline buffering and synchronization strategies
- Minimal overhead (~0.1 µs per state check)
- Integration with SD card buffering for reliable data logging

This library allows the STM32 Datalogger to intelligently handle network connectivity changes, ensuring no data loss while minimizing unnecessary transmission attempts when the MQTT broker is unavailable.
