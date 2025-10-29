# ESP32 Firmware - UML Diagrams

This document contains all UML diagrams for the ESP32 firmware architecture, including class diagrams, component diagrams, and structural relationships.

## ESP32 Firmware Class Diagram

```mermaid
classDiagram
    %% Main Application Class
    class ESP32_Main {
        +wifi_manager_t g_wifi_manager
        +stm32_uart_t g_stm32_uart
        +mqtt_handler_t g_mqtt_handler
        +relay_control_t g_relay
        +button_handler_t g_button_handler
        +json_sensor_parser_t g_json_parser
        +bool g_device_on
        +bool g_periodic_active
        +uint8_t g_interval_index
        +app_main() void
        +initialize_components() esp_err_t
        +update_and_publish_state() void
        +main_loop_task() void
    }
    
    %% WiFi Manager Class
    class WiFiManager {
        -wifi_state_t current_state
        -wifi_config_t wifi_config
        -uint8_t retry_count
        -uint32_t max_retries
        -uint32_t retry_delay_ms
        -bool auto_reconnect
        +WiFi_Init(wifi_config_t*) bool
        +WiFi_Connect() bool
        +WiFi_Disconnect() bool
        +WiFi_GetState() wifi_state_t
        +WiFi_IsConnected() bool
        +WiFi_GetRSSI() int8_t
        +WiFi_GetIPInfo(tcpip_adapter_ip_info_t*) bool
        -on_wifi_event(wifi_event_t*) void
        -retry_connection() void
    }
    
    %% MQTT Handler Class
    class MQTTHandler {
        -esp_mqtt_client_handle_t client
        -mqtt_config_t config
        -bool connected
        -int retry_count
        -uint32_t reconnect_delay_ms
        -mqtt_event_callback_t event_callback
        +MQTT_Handler_Init(mqtt_config_t*) bool
        +MQTT_Handler_Start() bool
        +MQTT_Handler_Stop() bool
        +MQTT_Handler_Subscribe(char*, int) bool
        +MQTT_Handler_Publish(char*, char*, int, int) bool
        +MQTT_Handler_IsConnected() bool
        -mqtt_event_handler(esp_mqtt_event_handle_t) void
        -handle_connected_event() void
        -handle_disconnected_event() void
        -handle_data_event(esp_mqtt_event_handle_t) void
    }
    
    %% UART Handler Class
    class STM32_UART {
        -int uart_num
        -uart_config_t config
        -QueueHandle_t uart_queue
        -ring_buffer_handle_t rx_buffer
        -stm32_data_callback_t callback
        -char rx_line_buffer[256]
        +STM32_UART_Init(uart_config_t*) bool
        +STM32_UART_SendCommand(char*) bool
        +STM32_UART_StartTask() bool
        +STM32_UART_SetCallback(stm32_data_callback_t) void
        -uart_rx_task(void*) void
        -process_received_line(char*) void
        -assemble_line_from_buffer() bool
    }
    
    %% Relay Control Class
    class RelayControl {
        -int gpio_num
        -bool state
        -bool initialized
        -relay_state_callback_t callback
        +Relay_Init(int) bool
        +Relay_SetState(bool) bool
        +Relay_GetState() bool
        +Relay_Toggle() bool
        +Relay_SetCallback(relay_state_callback_t) void
        -update_hardware_state(bool) void
        -trigger_state_callback(bool) void
    }
    
    %% JSON Parser Class
    class JSONSensorParser {
        -sensor_data_callback_t single_callback
        -sensor_data_callback_t periodic_callback
        -cJSON* json_root
        +JSON_Parser_Init() bool
        +JSON_Parser_ProcessLine(char*) bool
        +JSON_Parser_SetSingleCallback(sensor_data_callback_t) void
        +JSON_Parser_SetPeriodicCallback(sensor_data_callback_t) void
        +JSON_Parser_IsValid(char*) bool
        +JSON_Parser_IsSensorFailed(float, float) bool
        -extract_sensor_data(cJSON*) sensor_data_t
        -validate_json_structure(cJSON*) bool
    }
    
    %% Button Handler Class
    class ButtonHandler {
        -gpio_num_t relay_button_gpio
        -gpio_num_t single_button_gpio
        -gpio_num_t periodic_button_gpio
        -gpio_num_t interval_button_gpio
        -button_press_callback_t relay_callback
        -button_press_callback_t single_callback
        -button_press_callback_t periodic_callback
        -button_press_callback_t interval_callback
        -uint32_t debounce_time_ms
        -uint32_t last_press_time[4]
        +Button_Init(button_config_t*) bool
        +Button_StartTask() bool
        +Button_SetCallbacks(button_callbacks_t*) void
        -button_task(void*) void
        -handle_button_press(int) void
        -is_debounce_valid(int) bool
    }
    
    %% JSON Utils Class
    class JSONUtils {
        +JSON_Utils_CreateSystemState(bool, bool, time_t) char*
        +JSON_Utils_CreateSensorData(char*, time_t, float, float) char*
        +JSON_Utils_ParseCommand(char*) command_t
        +JSON_Utils_ValidateFormat(char*) bool
        +JSON_Utils_FreeString(char*) void
        -format_timestamp(time_t) char*
        -escape_json_string(char*) char*
    }
    
    %% COAP Handler Class
    class COAPHandler {
        -coap_context_t* context
        -coap_address_t server_addr
        -coap_session_t* session
        -bool initialized
        +COAP_Handler_Init(char*, uint16_t) bool
        +COAP_Handler_SendData(char*, char*) bool
        +COAP_Handler_Start() bool
        +COAP_Handler_Stop() bool
        -coap_response_handler(coap_session_t*, coap_pdu_t*) void
        -create_coap_pdu(char*, char*) coap_pdu_t*
    }
    
    %% Data Structures
    class SensorData {
        +char mode[16]
        +time_t timestamp
        +float temperature
        +float humidity
        +bool valid
    }
    
    class SystemState {
        +bool device_on
        +bool periodic_active
        +uint8_t interval_index
        +time_t last_update
        +wifi_state_t wifi_state
        +bool mqtt_connected
    }
    
    class WiFiState {
        <<enumeration>>
        WIFI_STATE_DISCONNECTED
        WIFI_STATE_CONNECTING
        WIFI_STATE_CONNECTED
        WIFI_STATE_FAILED
    }
    
    class MQTTState {
        <<enumeration>>
        MQTT_DISCONNECTED
        MQTT_CONNECTING
        MQTT_CONNECTED
        MQTT_ERROR
    }
    
    class ButtonType {
        <<enumeration>>
        BUTTON_RELAY
        BUTTON_SINGLE
        BUTTON_PERIODIC
        BUTTON_INTERVAL
    }
    
    class CommandType {
        <<enumeration>>
        CMD_SINGLE
        CMD_PERIODIC_ON
        CMD_PERIODIC_OFF
        CMD_SET_TIME
        CMD_SET_INTERVAL
        CMD_UNKNOWN
    }
    
    %% Relationships
    ESP32_Main *-- WiFiManager : manages
    ESP32_Main *-- MQTTHandler : manages
    ESP32_Main *-- STM32_UART : manages
    ESP32_Main *-- RelayControl : manages
    ESP32_Main *-- JSONSensorParser : manages
    ESP32_Main *-- ButtonHandler : manages
    ESP32_Main ..> JSONUtils : uses
    ESP32_Main ..> COAPHandler : optionally uses
    ESP32_Main *-- SystemState : maintains
    
    WiFiManager ..> WiFiState : uses
    MQTTHandler ..> MQTTState : uses
    JSONSensorParser *-- SensorData : creates
    ButtonHandler ..> ButtonType : handles
    JSONUtils ..> CommandType : parses
    
    WiFiManager --> MQTTHandler : triggers
    MQTTHandler --> JSONSensorParser : receives data for
    STM32_UART --> JSONSensorParser : forwards data to
    JSONSensorParser --> MQTTHandler : sends data via
    ButtonHandler --> RelayControl : controls
    ButtonHandler --> STM32_UART : sends commands via
    RelayControl --> SystemState : updates
    
    %% Callback Relationships
    WiFiManager ..> ESP32_Main : wifi_event_callback
    MQTTHandler ..> ESP32_Main : mqtt_event_callback
    STM32_UART ..> JSONSensorParser : data_received_callback
    JSONSensorParser ..> MQTTHandler : sensor_data_callback
    RelayControl ..> ESP32_Main : state_change_callback
    ButtonHandler ..> ESP32_Main : button_press_callback
```

## Component Interaction UML Diagram

```mermaid
classDiagram
    %% Component Interface Definitions
    class IWiFiManager {
        <<interface>>
        +connect() bool
        +disconnect() bool
        +getState() wifi_state_t
        +isConnected() bool
    }
    
    class IMQTTHandler {
        <<interface>>
        +start() bool
        +stop() bool
        +publish(topic, data) bool
        +subscribe(topic) bool
        +isConnected() bool
    }
    
    class IUARTHandler {
        <<interface>>
        +sendCommand(cmd) bool
        +setCallback(callback) void
        +startReceiving() bool
    }
    
    class IRelayControl {
        <<interface>>
        +setState(state) bool
        +getState() bool
        +toggle() bool
    }
    
    class IButtonHandler {
        <<interface>>
        +initialize() bool
        +setCallbacks(callbacks) void
        +startMonitoring() bool
    }
    
    class IJSONParser {
        <<interface>>
        +processLine(data) bool
        +setCallbacks(callbacks) void
        +isValid(data) bool
    }
    
    %% Concrete Implementations
    WiFiManager --|> IWiFiManager : implements
    MQTTHandler --|> IMQTTHandler : implements
    STM32_UART --|> IUARTHandler : implements
    RelayControl --|> IRelayControl : implements
    ButtonHandler --|> IButtonHandler : implements
    JSONSensorParser --|> IJSONParser : implements
    
    %% Component Dependencies
    ESP32_Main --> IWiFiManager : depends on
    ESP32_Main --> IMQTTHandler : depends on
    ESP32_Main --> IUARTHandler : depends on
    ESP32_Main --> IRelayControl : depends on
    ESP32_Main --> IButtonHandler : depends on
    ESP32_Main --> IJSONParser : depends on
    
    %% Inter-component Dependencies
    MQTTHandler --> IWiFiManager : requires
    JSONSensorParser --> IMQTTHandler : uses
    ButtonHandler --> IRelayControl : controls
    ButtonHandler --> IUARTHandler : commands via
```

## State Machine UML Diagram

```mermaid
stateDiagram-v2
    [*] --> SystemInit
    
    state SystemInit {
        [*] --> InitNVS
        InitNVS --> InitNetif
        InitNetif --> InitComponents
        InitComponents --> InitWiFi
        InitWiFi --> InitMQTT
        InitMQTT --> InitUART
        InitUART --> InitRelay
        InitRelay --> InitButtons
        InitButtons --> InitJSON
        InitJSON --> SystemReady
        SystemReady --> [*]
    }
    
    SystemInit --> OperationalState : All components initialized
    
    state OperationalState {
        [*] --> WiFiManagement
        [*] --> MQTTManagement
        [*] --> UARTCommunication
        [*] --> ButtonMonitoring
        [*] --> RelayControl
        
        state WiFiManagement {
            [*] --> WiFi_Disconnected
            WiFi_Disconnected --> WiFi_Connecting : connect()
            WiFi_Connecting --> WiFi_Connected : success
            WiFi_Connecting --> WiFi_Failed : max_retries
            WiFi_Failed --> WiFi_Connecting : retry_timer
            WiFi_Connected --> WiFi_Disconnected : connection_lost
        }
        
        state MQTTManagement {
            [*] --> MQTT_Disconnected
            MQTT_Disconnected --> MQTT_Connecting : WiFi connected + 4s stable
            MQTT_Connecting --> MQTT_Connected : broker_ack
            MQTT_Connecting --> MQTT_Error : connection_failed
            MQTT_Error --> MQTT_Connecting : exponential_backoff
            MQTT_Connected --> MQTT_Disconnected : WiFi_lost || broker_disconnect
        }
        
        state UARTCommunication {
            [*] --> UART_Idle
            UART_Idle --> UART_Receiving : data_available
            UART_Receiving --> UART_Processing : line_complete
            UART_Processing --> UART_Idle : processing_done
            UART_Idle --> UART_Transmitting : send_command
            UART_Transmitting --> UART_Idle : transmission_complete
        }
        
        state ButtonMonitoring {
            [*] --> Button_Idle
            Button_Idle --> Button_Pressed : gpio_interrupt
            Button_Pressed --> Button_Debouncing : start_debounce
            Button_Debouncing --> Button_Released : debounce_complete
            Button_Released --> Button_Processing : validate_press
            Button_Processing --> Button_Idle : action_complete
            Button_Debouncing --> Button_Idle : invalid_press
        }
        
        state RelayControl {
            [*] --> Relay_Off
            Relay_Off --> Relay_On : set_state(true)
            Relay_On --> Relay_Off : set_state(false)
            Relay_Off --> Relay_On : toggle()
            Relay_On --> Relay_Off : toggle()
        }
    }
    
    OperationalState --> ErrorRecovery : critical_error
    ErrorRecovery --> OperationalState : recovery_complete
    ErrorRecovery --> SystemRestart : unrecoverable_error
    SystemRestart --> [*]
```

## Package Structure UML Diagram

```mermaid
classDiagram
    %% Package definitions using namespaces
    namespace WiFiPackage {
        class WiFiManager
        class WiFiConfig
        class WiFiState
    }
    
    namespace MQTTPackage {
        class MQTTHandler
        class MQTTConfig
        class MQTTState
    }
    
    namespace UARTPackage {
        class STM32_UART
        class UARTConfig
        class RingBuffer
    }
    
    namespace RelayPackage {
        class RelayControl
        class RelayConfig
        class RelayState
    }
    
    namespace ButtonPackage {
        class ButtonHandler
        class ButtonConfig
        class ButtonState
    }
    
    namespace JSONPackage {
        class JSONSensorParser
        class JSONUtils
        class SensorData
    }
    
    namespace COAPPackage {
        class COAPHandler
        class COAPConfig
    }
    
    namespace CorePackage {
        class ESP32_Main
        class SystemState
        class SystemConfig
    }
    
    %% Package Dependencies
    CorePackage --> WiFiPackage : uses
    CorePackage --> MQTTPackage : uses
    CorePackage --> UARTPackage : uses
    CorePackage --> RelayPackage : uses
    CorePackage --> ButtonPackage : uses
    CorePackage --> JSONPackage : uses
    CorePackage --> COAPPackage : optionally uses
    
    MQTTPackage --> WiFiPackage : depends on
    JSONPackage --> MQTTPackage : uses
    ButtonPackage --> RelayPackage : controls
    ButtonPackage --> UARTPackage : commands via
```

## Object Interaction UML Diagram

```mermaid
classDiagram
    %% Runtime Object Instances
    class main_app["g_main_app : ESP32_Main"] {
        +wifi_manager : WiFiManager*
        +mqtt_handler : MQTTHandler*
        +stm32_uart : STM32_UART*
        +relay : RelayControl*
        +button_handler : ButtonHandler*
        +json_parser : JSONSensorParser*
    }
    
    class wifi_mgr["g_wifi_manager : WiFiManager"] {
        +current_state = WIFI_STATE_CONNECTED
        +retry_count = 0
        +config.ssid = "Redmi Note 9 Pro"
    }
    
    class mqtt_hdl["g_mqtt_handler : MQTTHandler"] {
        +client : esp_mqtt_client_handle_t
        +connected = true
        +retry_count = 0
        +config.uri = "mqtt://192.168.1.39:1883"
    }
    
    class uart_hdl["g_stm32_uart : STM32_UART"] {
        +uart_num = UART_NUM_1
        +rx_buffer : ring_buffer_handle_t
        +callback = on_stm32_data_received
    }
    
    class relay_ctrl["g_relay : RelayControl"] {
        +gpio_num = GPIO_NUM_4
        +state = true
        +callback = on_relay_state_changed
    }
    
    class btn_hdl["g_button_handler : ButtonHandler"] {
        +relay_button_gpio = GPIO_NUM_5
        +single_button_gpio = GPIO_NUM_16
        +periodic_button_gpio = GPIO_NUM_17
        +interval_button_gpio = GPIO_NUM_4
    }
    
    class json_parser["g_json_parser : JSONSensorParser"] {
        +single_callback = on_single_sensor_data
        +periodic_callback = on_periodic_sensor_data
    }
    
    class sensor_data1["sensor_data : SensorData"] {
        +mode = "SINGLE"
        +timestamp = 1729699200
        +temperature = 25.6
        +humidity = 60.2
    }
    
    class sensor_data2["sensor_data : SensorData"] {
        +mode = "PERIODIC"
        +timestamp = 1729699260
        +temperature = 25.8
        +humidity = 60.5
    }
    
    class system_state["g_system_state : SystemState"] {
        +device_on = true
        +periodic_active = false
        +interval_index = 0
        +wifi_state = WIFI_STATE_CONNECTED
        +mqtt_connected = true
    }
    
    %% Object Relationships
    main_app *-- wifi_mgr
    main_app *-- mqtt_hdl
    main_app *-- uart_hdl
    main_app *-- relay_ctrl
    main_app *-- btn_hdl
    main_app *-- json_parser
    main_app *-- system_state
    
    json_parser ..> sensor_data1 : creates
    json_parser ..> sensor_data2 : creates
    
    mqtt_hdl --> wifi_mgr : requires connected
    uart_hdl --> json_parser : forwards data to
    json_parser --> mqtt_hdl : publishes via
    btn_hdl --> relay_ctrl : controls
    btn_hdl --> uart_hdl : sends commands via
    relay_ctrl --> system_state : updates
```

## Error Handling UML Diagram

```mermaid
classDiagram
    %% Base Error Classes
    class ESP32Error {
        <<abstract>>
        #error_code : esp_err_t
        #error_message : char*
        #timestamp : time_t
        +getErrorCode() esp_err_t
        +getErrorMessage() char*
        +getTimestamp() time_t
        +toString() char*
    }
    
    class WiFiError {
        -wifi_state : wifi_state_t
        -retry_count : uint8_t
        +WiFiError(wifi_state_t, uint8_t)
        +getWiFiState() wifi_state_t
        +getRetryCount() uint8_t
        +isRecoverable() bool
    }
    
    class MQTTError {
        -mqtt_error_type : mqtt_error_type_t
        -broker_response : int
        +MQTTError(mqtt_error_type_t, int)
        +getErrorType() mqtt_error_type_t
        +getBrokerResponse() int
        +calculateBackoff() uint32_t
    }
    
    class UARTError {
        -uart_error_type : uart_error_type_t
        -buffer_overflow : bool
        +UARTError(uart_error_type_t, bool)
        +getErrorType() uart_error_type_t
        +isBufferOverflow() bool
        +requiresReinit() bool
    }
    
    class JSONError {
        -parse_error : cJSON_bool
        -invalid_data : char*
        +JSONError(cJSON_bool, char*)
        +getParseError() cJSON_bool
        +getInvalidData() char*
        +isRecoverable() bool
    }
    
    class RelayError {
        -gpio_error : esp_err_t
        -hardware_fault : bool
        +RelayError(esp_err_t, bool)
        +getGPIOError() esp_err_t
        +isHardwareFault() bool
        +canRetry() bool
    }
    
    %% Error Handler Classes
    class ErrorHandler {
        <<abstract>>
        +handleError(ESP32Error*) bool
        +logError(ESP32Error*) void
        +notifyError(ESP32Error*) void
    }
    
    class WiFiErrorHandler {
        -max_retries : uint8_t
        -retry_delay_ms : uint32_t
        +handleError(WiFiError*) bool
        +scheduleRetry(WiFiError*) void
        +resetConnection() bool
    }
    
    class MQTTErrorHandler {
        -max_backoff_ms : uint32_t
        -base_delay_ms : uint32_t
        +handleError(MQTTError*) bool
        +calculateExponentialBackoff(uint8_t) uint32_t
        +resetRetryCount() void
    }
    
    class UARTErrorHandler {
        +handleError(UARTError*) bool
        +clearBuffer() void
        +reinitializeUART() bool
        +restoreConfiguration() bool
    }
    
    class JSONErrorHandler {
        +handleError(JSONError*) bool
        +discardInvalidData() void
        +logParseDetails() void
        +continueProcessing() bool
    }
    
    class RelayErrorHandler {
        +handleError(RelayError*) bool
        +reinitializeGPIO() bool
        +testHardware() bool
        +disableRelay() void
    }
    
    %% Error Manager
    class ErrorManager {
        -error_handlers : ErrorHandler*[]
        -error_log : ESP32Error*[]
        -max_log_size : uint16_t
        +registerHandler(ErrorHandler*) void
        +handleError(ESP32Error*) bool
        +getErrorLog() ESP32Error*[]
        +clearErrorLog() void
        +getErrorStatistics() error_stats_t
    }
    
    %% Inheritance Relationships
    ESP32Error <|-- WiFiError
    ESP32Error <|-- MQTTError
    ESP32Error <|-- UARTError
    ESP32Error <|-- JSONError
    ESP32Error <|-- RelayError
    
    ErrorHandler <|-- WiFiErrorHandler
    ErrorHandler <|-- MQTTErrorHandler
    ErrorHandler <|-- UARTErrorHandler
    ErrorHandler <|-- JSONErrorHandler
    ErrorHandler <|-- RelayErrorHandler
    
    %% Composition Relationships
    ErrorManager *-- ErrorHandler : manages
    ErrorManager *-- ESP32Error : logs
    
    %% Usage Relationships
    WiFiManager ..> WiFiError : throws
    MQTTHandler ..> MQTTError : throws
    STM32_UART ..> UARTError : throws
    JSONSensorParser ..> JSONError : throws
    RelayControl ..> RelayError : throws
    
    WiFiManager ..> WiFiErrorHandler : uses
    MQTTHandler ..> MQTTErrorHandler : uses
    STM32_UART ..> UARTErrorHandler : uses
    JSONSensorParser ..> JSONErrorHandler : uses
    RelayControl ..> RelayErrorHandler : uses
    
    ESP32_Main *-- ErrorManager : contains
```

## Memory Management UML Diagram

```mermaid
classDiagram
    %% Memory Management Classes
    class MemoryManager {
        <<singleton>>
        -instance : MemoryManager*
        -heap_stats : multi_heap_info_t
        -task_stats : TaskStatus_t[]
        +getInstance() MemoryManager*
        +allocateMemory(size_t) void*
        +freeMemory(void*) void
        +getHeapInfo() heap_info_t
        +getTaskMemoryUsage() task_memory_t[]
        +logMemoryUsage() void
        +checkMemoryLeaks() bool
    }
    
    class RingBuffer {
        -buffer : uint8_t*
        -size : size_t
        -head : size_t
        -tail : size_t
        -mutex : SemaphoreHandle_t
        +create(size_t) RingBuffer*
        +destroy() void
        +write(uint8_t*, size_t) size_t
        +read(uint8_t*, size_t) size_t
        +availableSpace() size_t
        +availableData() size_t
        +reset() void
    }
    
    class QueueManager {
        -uart_queue : QueueHandle_t
        -button_queue : QueueHandle_t
        -wifi_event_queue : QueueHandle_t
        -mqtt_event_queue : QueueHandle_t
        +createQueues() bool
        +destroyQueues() void
        +getUARTQueue() QueueHandle_t
        +getButtonQueue() QueueHandle_t
        +getWiFiEventQueue() QueueHandle_t
        +getMQTTEventQueue() QueueHandle_t
    }
    
    class TaskManager {
        -uart_task_handle : TaskHandle_t
        -button_task_handle : TaskHandle_t
        -main_task_handle : TaskHandle_t
        -wifi_task_handle : TaskHandle_t
        +createTasks() bool
        +deleteTasks() void
        +suspendTask(TaskHandle_t) void
        +resumeTask(TaskHandle_t) void
        +getTaskInfo(TaskHandle_t) task_info_t
        +listAllTasks() task_list_t
    }
    
    class StringBuffer {
        -buffer : char*
        -capacity : size_t
        -length : size_t
        -auto_resize : bool
        +create(size_t) StringBuffer*
        +destroy() void
        +append(char*) bool
        +clear() void
        +toString() char*
        +getLength() size_t
        +getCapacity() size_t
        +resize(size_t) bool
    }
    
    class ConfigManager {
        -nvs_handle : nvs_handle_t
        -wifi_config : wifi_config_t
        -mqtt_config : mqtt_config_t
        -system_config : system_config_t
        +loadConfiguration() bool
        +saveConfiguration() bool
        +getWiFiConfig() wifi_config_t*
        +getMQTTConfig() mqtt_config_t*
        +getSystemConfig() system_config_t*
        +resetToDefaults() void
    }
    
    %% Memory Pool Classes
    class MemoryPool {
        <<abstract>>
        #pool_start : void*
        #pool_size : size_t
        #block_size : size_t
        #free_blocks : uint16_t
        #total_blocks : uint16_t
        +allocate() void*
        +deallocate(void*) void
        +getUsage() pool_usage_t
        +defragment() void
    }
    
    class JSONMemoryPool {
        -json_objects : cJSON*[]
        -max_objects : uint16_t
        +allocateJSON() cJSON*
        +deallocateJSON(cJSON*) void
        +getAvailableObjects() uint16_t
        +preAllocateObjects(uint16_t) void
    }
    
    class BufferMemoryPool {
        -buffer_pool : uint8_t*[]
        -buffer_size : size_t
        -pool_size : uint16_t
        +allocateBuffer() uint8_t*
        +deallocateBuffer(uint8_t*) void
        +getBufferSize() size_t
        +getAvailableBuffers() uint16_t
    }
    
    %% Relationships
    MemoryPool <|-- JSONMemoryPool
    MemoryPool <|-- BufferMemoryPool
    
    ESP32_Main --> MemoryManager : uses
    ESP32_Main --> QueueManager : uses
    ESP32_Main --> TaskManager : uses
    ESP32_Main --> ConfigManager : uses
    
    STM32_UART --> RingBuffer : uses
    JSONSensorParser --> JSONMemoryPool : uses
    STM32_UART --> BufferMemoryPool : uses
    
    MemoryManager --> MemoryPool : manages
    MemoryManager --> RingBuffer : monitors
    MemoryManager --> StringBuffer : tracks
    
    QueueManager --> TaskManager : coordinates with
    TaskManager --> MemoryManager : reports to
```

---

## UML Diagram Conventions

### Class Stereotypes
- `<<interface>>` - Abstract interface definitions
- `<<enumeration>>` - Enumerated types
- `<<singleton>>` - Singleton pattern implementation
- `<<abstract>>` - Abstract base classes

### Relationship Types
- `-->` - Association (uses/depends on)
- `*--` - Composition (owns/contains)
- `o--` - Aggregation (has/references)
- `<|--` - Inheritance (extends/implements)
- `..>` - Dependency (uses temporarily)
- `-.->` - Realization (implements interface)

### Visibility Indicators
- `+` - Public members and methods
- `-` - Private members and methods
- `#` - Protected members and methods
- `~` - Package-private members

### Method Annotations
- `()` - Method with no parameters
- `(type param)` - Method with typed parameters
- `type` - Return type specification
- `void` - No return value

### Class Attributes
- Simple attributes: `+attribute_name : type`
- Constants: `+CONSTANT_NAME : type = value`
- Static members: `{static} +static_member : type`
- Array/Collection: `+items : type[]`

### Package Organization
- Core functionality in `CorePackage`
- Network components in `WiFiPackage` and `MQTTPackage`
- Communication in `UARTPackage`
- Hardware control in `RelayPackage` and `ButtonPackage`
- Data processing in `JSONPackage`
- Optional features in `COAPPackage`

### Design Patterns Implemented
- **Singleton**: MemoryManager, ErrorManager
- **Observer**: Event callbacks throughout system
- **Strategy**: Different error handling strategies
- **Factory**: Memory pool creation
- **State Machine**: WiFi and MQTT state management
- **Command**: Button press handling
- **Facade**: Component managers hide complexity