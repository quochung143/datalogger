# SYSTEM - Flow Diagrams (Complete System)

Comprehensive flowcharts describing the complete DATALOGGER system operation including all 4 subsystems: STM32 Firmware, ESP32 Gateway, MQTT Broker, and Web Dashboard.

---

## 1. Complete System Initialization Flow

```mermaid
flowchart TD
    Start([System Power On]) --> STM32_Init[STM32 Initialization]
    
    STM32_Init --> STM32_HAL[Initialize HAL & Clock<br/>72MHz System Clock]
    STM32_HAL --> STM32_Periph[Initialize Peripherals<br/>I2C1, SPI1, UART1, TIM2]
    STM32_Periph --> STM32_Drivers[Initialize Drivers<br/>SHT3X, DS3231, SD Card<br/>ILI9225 Display]
    STM32_Drivers --> STM32_Buffers[Setup Buffers<br/>Ring Buffer 256B<br/>SD Buffer Init]
    STM32_Buffers --> STM32_Ready{STM32<br/>Ready?}
    
    STM32_Ready -->|No| STM32_Error[Display Error<br/>on LCD]
    STM32_Ready -->|Yes| STM32_Display[Show Startup<br/>Message on Display]
    
    STM32_Display --> ESP32_Init[ESP32 Initialization]
    
    ESP32_Init --> ESP32_FreeRTOS[Initialize FreeRTOS<br/>Create Tasks]
    ESP32_FreeRTOS --> ESP32_Periph[Initialize Peripherals<br/>UART2, GPIO4 Relay]
    ESP32_Periph --> ESP32_WiFi[WiFi Manager Init<br/>Connect to AP]
    
    ESP32_WiFi --> WiFi_Connect{WiFi<br/>Connected?}
    WiFi_Connect -->|No| WiFi_Retry[Retry Connection<br/>Max 10 attempts]
    WiFi_Retry --> WiFi_Connect
    WiFi_Connect -->|Yes| MQTT_Init[MQTT Handler Init]
    
    MQTT_Init --> MQTT_Connect[Connect to Broker<br/>ws://broker:8083]
    MQTT_Connect --> MQTT_Auth{Authentication<br/>Success?}
    MQTT_Auth -->|No| MQTT_Error[Log Error<br/>Retry in 5s]
    MQTT_Error --> MQTT_Connect
    MQTT_Auth -->|Yes| MQTT_Sub[Subscribe to Topics<br/>datalogger/command<br/>datalogger/relay<br/>datalogger/time/sync]
    
    MQTT_Sub --> ESP32_Notify[Send MQTT CONNECTED<br/>to STM32 via UART]
    ESP32_Notify --> STM32_Update[STM32 Updates Status<br/>WiFi Connected]
    STM32_Update --> Display_Update[Update LCD Display<br/>Show WiFi Status]
    
    Display_Update --> Web_Init[Web Dashboard Load]
    Web_Init --> Web_Connect[Connect MQTT WebSocket<br/>ws://broker:8083/mqtt]
    Web_Connect --> Firebase_Init[Initialize Firebase<br/>Realtime Database]
    Firebase_Init --> System_Ready([System Fully<br/>Operational])
    
    style Start fill:#90EE90
    style System_Ready fill:#90EE90
    style STM32_Error fill:#FFB6C6
    style MQTT_Error fill:#FFB6C6
    style WiFi_Connect fill:#FFE4B5
    style MQTT_Auth fill:#FFE4B5
    style STM32_Ready fill:#FFE4B5
```

---

## 2. Complete Data Acquisition & Transmission Flow

```mermaid
flowchart TD
    Start([Data Acquisition<br/>Trigger]) --> Mode{Operation<br/>Mode?}
    
    Mode -->|Single Read| Single_Start[Single Read Command<br/>from Web Dashboard]
    Mode -->|Periodic| Periodic_Start[Periodic Timer<br/>Expires TIM2]
    
    Single_Start --> SHT3X_Single[SHT3X Single-Shot<br/>Measurement]
    Periodic_Start --> SHT3X_Fetch[SHT3X Fetch Data<br/>from Periodic Mode]
    
    SHT3X_Single --> Sensor_Read{Sensor<br/>Read OK?}
    SHT3X_Fetch --> Sensor_Read
    
    Sensor_Read -->|Failed| Error_Handle[Set Error Values<br/>Temp=0.0, Hum=0.0]
    Sensor_Read -->|Success| Get_Time[Read DS3231 RTC<br/>Unix Timestamp]
    
    Error_Handle --> Update_Manager
    Get_Time --> Update_Manager[Update Data Manager<br/>Store Temp, Hum, Time]
    
    Update_Manager --> JSON_Format[Format JSON Output<br/>sensor_json_output.c]
    JSON_Format --> WiFi_Check{ESP32<br/>Connected?}
    
    WiFi_Check -->|Yes| UART_Send[Send JSON to ESP32<br/>UART 115200bps]
    WiFi_Check -->|No| SD_Buffer[Buffer to SD Card<br/>204,800 records capacity]
    
    UART_Send --> ESP32_Receive[ESP32 UART Handler<br/>Receives Data]
    ESP32_Receive --> JSON_Parse[Parse JSON<br/>json_sensor_parser.c]
    JSON_Parse --> MQTT_Pub[Publish to MQTT<br/>Topic: datalogger/data/single<br/>or datalogger/data/periodic]
    
    MQTT_Pub --> Broker_Route[MQTT Broker<br/>Routes Message]
    Broker_Route --> Web_Receive[Web Dashboard<br/>Receives via WebSocket]
    Web_Receive --> Update_UI[Update UI Elements<br/>Dashboard, Charts]
    Update_UI --> Firebase_Store[Store to Firebase<br/>Realtime Database]
    
    SD_Buffer --> SD_Write{SD Write<br/>Success?}
    SD_Write -->|No| SD_Error[Log SD Error<br/>Data Lost]
    SD_Write -->|Yes| SD_Index[Update Write Index<br/>Increment Counter]
    
    Firebase_Store --> Display_LCD[Update ILI9225 LCD<br/>Show Latest Data]
    SD_Index --> Display_LCD
    SD_Error --> Display_LCD
    
    Display_LCD --> End([Data Cycle<br/>Complete])
    
    style Start fill:#90EE90
    style End fill:#90EE90
    style Sensor_Read fill:#FFE4B5
    style WiFi_Check fill:#FFE4B5
    style SD_Write fill:#FFE4B5
    style Error_Handle fill:#FFB6C6
    style SD_Error fill:#FFB6C6
```

---

## 3. Command Processing Flow (Web to STM32)

```mermaid
flowchart TD
    Start([User Action<br/>on Web Dashboard]) --> UI_Event[Button Click or<br/>Form Submit]
    
    UI_Event --> Command_Type{Command<br/>Type?}
    
    Command_Type -->|Read Single| Cmd_Single[Build JSON Command<br/>command: READ_SINGLE]
    Command_Type -->|Start Periodic| Cmd_StartPer[Build JSON Command<br/>command: START_PERIODIC<br/>interval: 5000]
    Command_Type -->|Stop Periodic| Cmd_StopPer[Build JSON Command<br/>command: STOP_PERIODIC]
    Command_Type -->|Set Time| Cmd_Time[Build JSON Command<br/>command: SET_TIME<br/>timestamp: unix_time]
    Command_Type -->|Relay Control| Cmd_Relay[Build JSON Command<br/>state: ON/OFF]
    
    Cmd_Single --> MQTT_Publish
    Cmd_StartPer --> MQTT_Publish
    Cmd_StopPer --> MQTT_Publish
    Cmd_Time --> MQTT_Publish
    Cmd_Relay --> Relay_Topic[Publish to<br/>datalogger/relay]
    
    MQTT_Publish[Publish to MQTT<br/>Topic: datalogger/command<br/>QoS: 1]
    
    Relay_Topic --> ESP32_Relay[ESP32 Relay Handler<br/>Receives Command]
    MQTT_Publish --> Broker_Receive[MQTT Broker<br/>Receives Message]
    
    Broker_Receive --> ESP32_Sub[ESP32 Subscribed<br/>Receives Command]
    ESP32_Sub --> Parse_CMD[Parse JSON Command<br/>json_utils.c]
    
    Parse_CMD --> Format_UART[Format UART Command<br/>Text-based Protocol]
    Format_UART --> UART_TX[Send to STM32<br/>UART TX 115200bps]
    
    UART_TX --> STM32_RX[STM32 UART RX<br/>Interrupt Handler]
    STM32_RX --> Ring_Buffer[Store in Ring Buffer<br/>256 bytes capacity]
    Ring_Buffer --> Parse_Line{Complete Line<br/>Received?}
    
    Parse_Line -->|No| Wait_More[Wait for More Data]
    Wait_More --> STM32_RX
    Parse_Line -->|Yes| CMD_Parser[Command Parser<br/>cmd_parser.c]
    
    CMD_Parser --> CMD_Execute[Command Execute<br/>command_execute.c]
    CMD_Execute --> CMD_Func[Execute Command Function<br/>cmd_func.c]
    
    CMD_Func --> Action_Type{Action<br/>Type?}
    
    Action_Type -->|Single| Execute_Single[Trigger Single<br/>Measurement]
    Action_Type -->|Start Periodic| Execute_Periodic[Start TIM2 Timer<br/>Set Interval]
    Action_Type -->|Stop Periodic| Execute_Stop[Stop TIM2 Timer<br/>Disable Periodic]
    Action_Type -->|Set Time| Execute_Time[Update DS3231 RTC<br/>Set Date/Time]
    
    ESP32_Relay --> GPIO_Control[Control GPIO4<br/>Set HIGH/LOW]
    GPIO_Control --> Relay_Switch[Physical Relay<br/>Switches State]
    Relay_Switch --> State_Pub[Publish State<br/>datalogger/state]
    
    Execute_Single --> Response_Gen
    Execute_Periodic --> Response_Gen
    Execute_Stop --> Response_Gen
    Execute_Time --> Response_Gen
    State_Pub --> Response_Gen
    
    Response_Gen[Generate Response<br/>or Trigger Action] --> End([Command<br/>Executed])
    
    style Start fill:#90EE90
    style End fill:#90EE90
    style Command_Type fill:#FFE4B5
    style Parse_Line fill:#FFE4B5
    style Action_Type fill:#FFE4B5
```

---

## 4. Offline Buffering & Sync Flow

```mermaid
flowchart TD
    Start([Data Ready for<br/>Transmission]) --> WiFi_Status{ESP32 WiFi<br/>Connected?}
    
    WiFi_Status -->|Yes| Direct_Send[Send Data via<br/>UART to ESP32]
    WiFi_Status -->|No| Buffer_Path[Offline Mode<br/>Activated]
    
    Direct_Send --> MQTT_Transmit[Transmit via MQTT<br/>to Cloud]
    MQTT_Transmit --> End_Direct([Normal Flow<br/>Complete])
    
    Buffer_Path --> SD_Manager[SD Card Manager<br/>sd_card_manager.c]
    SD_Manager --> Check_Space{SD Buffer<br/>Has Space?}
    
    Check_Space -->|No| Buffer_Full[Buffer Full<br/>204,800 records limit]
    Buffer_Full --> Overflow_Handle[Circular Overflow<br/>Overwrite Oldest]
    Check_Space -->|Yes| Write_Record[Write Data Record<br/>512 bytes per block]
    
    Overflow_Handle --> Write_Record
    Write_Record --> Record_Format[Format Record:<br/>Timestamp, Temp, Hum<br/>Mode, Sequence #]
    Record_Format --> SPI_Write[SPI Write to SD<br/>PA4-PA7 pins]
    
    SPI_Write --> Write_Success{Write<br/>Success?}
    Write_Success -->|No| Write_Error[Log Error<br/>Retry 3 times]
    Write_Error --> Write_Retry{Retry<br/>Count < 3?}
    Write_Retry -->|Yes| SPI_Write
    Write_Retry -->|No| Data_Lost[Data Lost<br/>Display Warning]
    
    Write_Success -->|Yes| Update_Index[Update Write Index<br/>Increment Counter]
    Update_Index --> Display_SD[Update LCD Display<br/>Show SD: WRITING]
    Display_SD --> Monitor_WiFi[Monitor WiFi Status<br/>Check Reconnection]
    
    Monitor_WiFi --> WiFi_Restored{WiFi<br/>Restored?}
    WiFi_Restored -->|No| Continue_Buffer[Continue Buffering<br/>New Data]
    Continue_Buffer --> Monitor_WiFi
    
    WiFi_Restored -->|Yes| MQTT_Reconnect[MQTT Reconnects<br/>to Broker]
    MQTT_Reconnect --> Notify_STM32[ESP32 Sends<br/>MQTT CONNECTED]
    Notify_STM32 --> STM32_Detect[STM32 Detects<br/>Reconnection]
    
    STM32_Detect --> Check_Buffered{Buffered Data<br/>Available?}
    Check_Buffered -->|No| Resume_Normal[Resume Normal<br/>Operation]
    Check_Buffered -->|Yes| Read_Oldest[Read Oldest Record<br/>from SD Card]
    
    Read_Oldest --> Parse_Record[Parse Record Data<br/>Extract Fields]
    Parse_Record --> Send_Buffered[Send via UART<br/>to ESP32]
    Send_Buffered --> ESP32_Fwd[ESP32 Forwards<br/>to MQTT]
    
    ESP32_Fwd --> Wait_ACK{Wait for<br/>ACK?}
    Wait_ACK -->|Timeout| Retry_Send[Retry Send<br/>Max 3 attempts]
    Retry_Send --> Wait_ACK
    Wait_ACK -->|Success| Remove_Record[Remove Record<br/>Decrement Counter]
    
    Remove_Record --> More_Data{More Buffered<br/>Data?}
    More_Data -->|Yes| Read_Oldest
    More_Data -->|No| Sync_Complete[Sync Complete<br/>Clear SD Buffer Flag]
    
    Sync_Complete --> Resume_Normal
    Resume_Normal --> End_Sync([Return to<br/>Normal Mode])
    
    style Start fill:#90EE90
    style End_Direct fill:#90EE90
    style End_Sync fill:#90EE90
    style WiFi_Status fill:#FFE4B5
    style Check_Space fill:#FFE4B5
    style Write_Success fill:#FFE4B5
    style WiFi_Restored fill:#FFE4B5
    style Check_Buffered fill:#FFE4B5
    style More_Data fill:#FFE4B5
    style Buffer_Full fill:#FFB6C6
    style Data_Lost fill:#FFB6C6
```

---

## 5. Error Detection & Recovery Flow

```mermaid
flowchart TD
    Start([System Running]) --> Monitor[Continuous Monitoring<br/>All Components]
    
    Monitor --> Check_Sensors{Sensor<br/>Status?}
    Check_Sensors -->|OK| Check_Comm
    Check_Sensors -->|Error| Sensor_Err[SHT3X Read Failed<br/>I2C Error]
    
    Sensor_Err --> Sensor_Retry[Retry Read<br/>3 attempts]
    Sensor_Retry --> Sensor_Retry_OK{Retry<br/>Success?}
    Sensor_Retry_OK -->|No| Sensor_Failed[Mark Sensor Failed<br/>Send 0.0 values]
    Sensor_Retry_OK -->|Yes| Sensor_Recovered[Sensor Recovered<br/>Clear Error Flag]
    
    Sensor_Failed --> Display_Err1[Display Error<br/>on LCD: SENSOR ERR]
    Sensor_Recovered --> Monitor
    
    Check_Comm{Communication<br/>Status?}
    Check_Comm -->|OK| Check_Storage
    Check_Comm -->|Error| Comm_Type{Error<br/>Type?}
    
    Comm_Type -->|UART Timeout| UART_Err[UART Communication<br/>Lost]
    Comm_Type -->|WiFi Down| WiFi_Err[WiFi Disconnected<br/>Event]
    Comm_Type -->|MQTT Error| MQTT_Err[MQTT Connection<br/>Lost]
    
    UART_Err --> UART_Reset[Reset UART<br/>Peripheral]
    UART_Reset --> UART_Reinit[Reinitialize<br/>UART Handler]
    UART_Reinit --> Monitor
    
    WiFi_Err --> WiFi_Reconnect[Start WiFi<br/>Reconnection]
    WiFi_Reconnect --> WiFi_Attempt{Reconnect<br/>Success?}
    WiFi_Attempt -->|No| WiFi_Backoff[Exponential Backoff<br/>Wait 5s, 10s, 30s]
    WiFi_Backoff --> WiFi_Reconnect
    WiFi_Attempt -->|Yes| WiFi_Restored[WiFi Restored<br/>Notify STM32]
    WiFi_Restored --> MQTT_Reconnect
    
    MQTT_Err --> MQTT_Reconnect[MQTT Reconnect<br/>Attempt]
    MQTT_Reconnect --> MQTT_Auth{Auth<br/>Success?}
    MQTT_Auth -->|No| MQTT_Retry[Retry in 5s<br/>Check Credentials]
    MQTT_Retry --> MQTT_Reconnect
    MQTT_Auth -->|Yes| MQTT_Sub[Resubscribe<br/>to Topics]
    MQTT_Sub --> MQTT_Restored[MQTT Restored<br/>Resume Operation]
    MQTT_Restored --> Monitor
    
    Check_Storage{Storage<br/>Status?}
    Check_Storage -->|OK| Check_Display
    Check_Storage -->|Error| SD_Error[SD Card Error<br/>Write Failed]
    
    SD_Error --> SD_Reinit[Reinitialize<br/>SD Card]
    SD_Reinit --> SD_Retry{Reinit<br/>Success?}
    SD_Retry -->|No| SD_Failed[SD Card Failed<br/>Disable Buffering]
    SD_Retry -->|Yes| SD_OK[SD Card OK<br/>Resume Buffering]
    
    SD_Failed --> Display_Err2[Display Error<br/>LCD: SD CARD ERR]
    SD_OK --> Monitor
    
    Check_Display{Display<br/>Status?}
    Check_Display -->|OK| Check_RTC
    Check_Display -->|Error| Display_Error[ILI9225 SPI Error<br/>Communication Lost]
    
    Display_Error --> Display_Reinit[Reinitialize<br/>Display]
    Display_Reinit --> Display_Test[Test Display<br/>Draw Pattern]
    Display_Test --> Display_OK{Test<br/>Success?}
    Display_OK -->|No| Display_Disabled[Disable Display<br/>Continue Operation]
    Display_OK -->|Yes| Display_Recovered[Display Recovered]
    Display_Recovered --> Monitor
    Display_Disabled --> Monitor
    
    Check_RTC{RTC<br/>Status?}
    Check_RTC -->|OK| All_OK
    Check_RTC -->|Error| RTC_Error[DS3231 I2C Error<br/>Time Invalid]
    
    RTC_Error --> RTC_Retry[Retry Read<br/>3 attempts]
    RTC_Retry --> RTC_OK{Read<br/>Success?}
    RTC_OK -->|No| RTC_Failed[RTC Failed<br/>Use System Ticks]
    RTC_OK -->|Yes| RTC_Restored[RTC Restored]
    RTC_Failed --> Display_Err3[Display Error<br/>LCD: RTC ERR]
    RTC_Restored --> Monitor
    
    All_OK[All Systems<br/>Operational] --> Health_Report[Send Health Report<br/>datalogger/state]
    Health_Report --> Monitor
    
    Display_Err1 --> Log_Error
    Display_Err2 --> Log_Error
    Display_Err3 --> Log_Error
    
    Log_Error[Log Error to<br/>System Log] --> Notify_Web[Notify Web Dashboard<br/>Component Status]
    Notify_Web --> Monitor
    
    style Start fill:#90EE90
    style All_OK fill:#90EE90
    style Monitor fill:#87CEEB
    style Check_Sensors fill:#FFE4B5
    style Check_Comm fill:#FFE4B5
    style Check_Storage fill:#FFE4B5
    style Check_Display fill:#FFE4B5
    style Check_RTC fill:#FFE4B5
    style Sensor_Failed fill:#FFB6C6
    style SD_Failed fill:#FFB6C6
    style Display_Disabled fill:#FFB6C6
    style RTC_Failed fill:#FFB6C6
    style Display_Err1 fill:#FFB6C6
    style Display_Err2 fill:#FFB6C6
    style Display_Err3 fill:#FFB6C6
```

---

## 6. Periodic Mode Operation Flow

```mermaid
flowchart TD
    Start([Periodic Mode<br/>START Command]) --> Validate_Interval{Interval<br/>Valid?}
    
    Validate_Interval -->|No| Invalid[Invalid Interval<br/>500ms - 60000ms]
    Validate_Interval -->|Yes| Config_TIM2[Configure TIM2<br/>Set Prescaler & Period]
    
    Invalid --> Send_Error[Send Error Response<br/>to Web Dashboard]
    Send_Error --> End_Error([Mode Not Started])
    
    Config_TIM2 --> Start_SHT3X[Start SHT3X<br/>Periodic Mode]
    Start_SHT3X --> SHT3X_Mode{Select<br/>Mode?}
    
    SHT3X_Mode -->|Interval >= 10s| Mode_05MPS[SHT3X_PERIODIC_05MPS<br/>0.5 measurements/sec]
    SHT3X_Mode -->|5s <= Interval < 10s| Mode_1MPS[SHT3X_PERIODIC_1MPS<br/>1 measurement/sec]
    SHT3X_Mode -->|2s <= Interval < 5s| Mode_2MPS[SHT3X_PERIODIC_2MPS<br/>2 measurements/sec]
    SHT3X_Mode -->|1s <= Interval < 2s| Mode_4MPS[SHT3X_PERIODIC_4MPS<br/>4 measurements/sec]
    SHT3X_Mode -->|Interval < 1s| Mode_10MPS[SHT3X_PERIODIC_10MPS<br/>10 measurements/sec]
    
    Mode_05MPS --> Init_Sensor
    Mode_1MPS --> Init_Sensor
    Mode_2MPS --> Init_Sensor
    Mode_4MPS --> Init_Sensor
    Mode_10MPS --> Init_Sensor
    
    Init_Sensor[Initialize Sensor<br/>Periodic Measurement] --> Sensor_Init_OK{Init<br/>Success?}
    
    Sensor_Init_OK -->|No| Init_Failed[Sensor Init Failed<br/>I2C Error]
    Init_Failed --> Retry_Init[Retry Initialization<br/>3 attempts]
    Retry_Init --> Sensor_Init_OK
    
    Sensor_Init_OK -->|Yes| Enable_Timer[Enable TIM2<br/>Start Interrupt]
    Enable_Timer --> Set_Flag[Set Periodic<br/>Active Flag]
    Set_Flag --> Update_Display[Update LCD Display<br/>Show PERIODIC: ON<br/>Interval: Xs]
    
    Update_Display --> Notify_Started[Send Status to Web<br/>datalogger/state]
    Notify_Started --> Wait_Timer([Wait for Timer<br/>Interrupt])
    
    Wait_Timer --> TIM2_ISR[TIM2 IRQ Handler<br/>Triggers]
    TIM2_ISR --> Set_Measure_Flag[Set Measurement<br/>Required Flag]
    Set_Measure_Flag --> Main_Loop[Return to Main Loop]
    
    Main_Loop --> Check_Flag{Measurement<br/>Flag Set?}
    Check_Flag -->|No| Other_Tasks[Process Other Tasks<br/>Commands, Display]
    Other_Tasks --> Check_Flag
    
    Check_Flag -->|Yes| Clear_Flag[Clear Measurement<br/>Flag]
    Clear_Flag --> Fetch_Data[SHT3X Fetch Data<br/>Read from Sensor Buffer]
    
    Fetch_Data --> Data_Valid{Data<br/>Valid?}
    Data_Valid -->|No| Error_Count[Increment Error<br/>Counter]
    Error_Count --> Error_Threshold{Errors<br/>> 5?}
    Error_Threshold -->|Yes| Stop_Periodic[Auto-Stop Periodic<br/>Sensor Problem]
    Error_Threshold -->|No| Use_Last[Use Last Valid<br/>Data]
    
    Data_Valid -->|Yes| Get_Timestamp[Read DS3231<br/>Unix Timestamp]
    Use_Last --> Get_Timestamp
    
    Get_Timestamp --> Update_Manager[Update Data Manager<br/>data_manager.c]
    Update_Manager --> Format_JSON[Format JSON Output<br/>Mode: PERIODIC]
    
    Format_JSON --> Check_WiFi{WiFi<br/>Connected?}
    Check_WiFi -->|Yes| Send_UART[Send to ESP32<br/>UART TX]
    Check_WiFi -->|No| Buffer_SD[Buffer to SD Card<br/>Offline Storage]
    
    Send_UART --> ESP32_Process[ESP32 Processes<br/>Publishes MQTT]
    ESP32_Process --> Web_Update[Web Dashboard<br/>Updates Chart]
    Web_Update --> Firebase_Log[Firebase Stores<br/>Historical Data]
    
    Buffer_SD --> SD_Write[Write to SD<br/>Increment Counter]
    SD_Write --> Display_Update
    Firebase_Log --> Display_Update
    
    Display_Update[Update LCD Display<br/>Temp, Hum, Time] --> Check_Stop{Stop Command<br/>Received?}
    
    Check_Stop -->|No| Wait_Timer
    Check_Stop -->|Yes| Stop_Timer[Disable TIM2<br/>Stop Interrupt]
    
    Stop_Periodic --> Stop_Timer
    Stop_Timer --> Stop_SHT3X[Stop SHT3X<br/>Periodic Mode]
    Stop_SHT3X --> Clear_Periodic_Flag[Clear Periodic<br/>Active Flag]
    Clear_Periodic_Flag --> Final_Display[Update Display<br/>Show PERIODIC: OFF]
    Final_Display --> Notify_Stopped[Send Status<br/>Mode Stopped]
    Notify_Stopped --> End_Stop([Periodic Mode<br/>Stopped])
    
    style Start fill:#90EE90
    style End_Error fill:#FFB6C6
    style End_Stop fill:#90EE90
    style Validate_Interval fill:#FFE4B5
    style SHT3X_Mode fill:#FFE4B5
    style Sensor_Init_OK fill:#FFE4B5
    style Data_Valid fill:#FFE4B5
    style Check_WiFi fill:#FFE4B5
    style Error_Threshold fill:#FFE4B5
    style Check_Stop fill:#FFE4B5
    style Check_Flag fill:#FFE4B5
    style Invalid fill:#FFB6C6
    style Init_Failed fill:#FFB6C6
    style Stop_Periodic fill:#FFB6C6
```

---

*Continued in next section...*
