# DS3231 RTC Library (ds3231)

## Overview

The DS3231 RTC Library provides a complete driver for the DS3231 high-precision real-time clock module. It supports time/date read/write operations, temperature reading, alarm configuration, and control register management via I2C interface.

## Files

- **ds3231.c**: DS3231 driver implementation
- **ds3231.h**: DS3231 API declarations and data structures

## Hardware Configuration

### DS3231 Specifications

- **Interface**: I2C
- **I2C Address**: 0x68 (7-bit address, 0xD0 for write, 0xD1 for read)
- **Supply Voltage**: 2.3V - 5.5V
- **Timekeeping Accuracy**: ±2ppm (0-40°C)
- **Temperature Sensor**: Built-in, -40°C to +85°C
- **Battery Backup**: Supports coin cell for timekeeping during power loss

### Register Map

```
Address  Register              Description
-------  -------------------   ------------------------------------
0x00     Seconds              BCD format (00-59)
0x01     Minutes              BCD format (00-59)
0x02     Hours                BCD format (00-23 or 01-12 + AM/PM)
0x03     Day of Week          1-7 (Monday=1)
0x04     Date                 BCD format (01-31)
0x05     Month/Century        BCD format (01-12), Century bit
0x06     Year                 BCD format (00-99)
0x07     Alarm 1 Seconds      BCD format
0x08     Alarm 1 Minutes      BCD format
0x09     Alarm 1 Hours        BCD format
0x0A     Alarm 1 Day/Date     BCD format
0x0B     Alarm 2 Minutes      BCD format
0x0C     Alarm 2 Hours        BCD format
0x0D     Alarm 2 Day/Date     BCD format
0x0E     Control              Configuration register
0x0F     Control/Status       Status flags
0x10     Aging Offset         Crystal aging trim
0x11-12  Temperature          11-bit temperature value
```

## Data Structures

### Time Structure

```c
typedef struct {
    uint8_t second;   // 0-59
    uint8_t minute;   // 0-59
    uint8_t hour;     // 0-23 (24-hour format)
} DS3231_Time;
```

### Date Structure

```c
typedef struct {
    uint8_t dayOfWeek;  // 1-7 (Monday=1, Sunday=7)
    uint8_t date;       // 1-31
    uint8_t month;      // 1-12
    uint16_t year;      // 2000-2099 (stored as 00-99 + 2000)
} DS3231_Date;
```

### Alarm Time Structure

```c
typedef struct {
    uint8_t second;   // 0-59 (Alarm 1 only)
    uint8_t minute;   // 0-59
    uint8_t hour;     // 0-23
    uint8_t date;     // 1-31 or day of week 1-7
} DS3231_AlarmTime;
```

## Core Functions

### Initialization

```c
void DS3231_Init(void);
```

Initializes the DS3231 module.

**Operations**:
- Verifies I2C communication
- Clears status flags
- Enables oscillator if disabled

**Usage Example**:
```c
int main(void)
{
    HAL_Init();
    MX_I2C1_Init();
    
    DS3231_Init();  // Initialize RTC
    
    // RTC is ready
}
```

### Time Operations

#### Read Current Time

```c
uint8_t DS3231_ReadTime(DS3231_Time *time);
```

Reads the current time from the RTC.

**Parameters**:
- `time`: Pointer to DS3231_Time structure to store result

**Returns**:
- 0: Success
- Non-zero: I2C communication error

**Usage Example**:
```c
DS3231_Time currentTime;
if (DS3231_ReadTime(&currentTime) == 0)
{
    printf("Current time: %02d:%02d:%02d\n",
           currentTime.hour,
           currentTime.minute,
           currentTime.second);
}
else
{
    printf("Error reading time\n");
}
```

**I2C Transaction**:
- Read 3 bytes starting at register 0x00
- Convert BCD to binary

#### Set Time

```c
uint8_t DS3231_SetTime(uint8_t hour, uint8_t minute, uint8_t second);
```

Sets the RTC time.

**Parameters**:
- `hour`: Hour value (0-23)
- `minute`: Minute value (0-59)
- `second`: Second value (0-59)

**Returns**:
- 0: Success
- Non-zero: I2C communication error or invalid parameters

**Usage Example**:
```c
// Set time to 14:35:00
if (DS3231_SetTime(14, 35, 0) == 0)
{
    printf("Time set successfully\n");
}
else
{
    printf("Failed to set time\n");
}
```

**Validation**:
- Hour: 0-23
- Minute: 0-59
- Second: 0-59
- Invalid values return error

### Date Operations

#### Read Current Date

```c
uint8_t DS3231_ReadDate(DS3231_Date *date);
```

Reads the current date from the RTC.

**Parameters**:
- `date`: Pointer to DS3231_Date structure to store result

**Returns**:
- 0: Success
- Non-zero: I2C communication error

**Usage Example**:
```c
DS3231_Date currentDate;
if (DS3231_ReadDate(&currentDate) == 0)
{
    printf("Current date: %04d-%02d-%02d (Day %d)\n",
           currentDate.year,
           currentDate.month,
           currentDate.date,
           currentDate.dayOfWeek);
}
```

**I2C Transaction**:
- Read 4 bytes starting at register 0x03
- Convert BCD to binary
- Add 2000 to year value

#### Set Date

```c
uint8_t DS3231_SetDate(uint8_t dayOfWeek, uint8_t date, uint8_t month, uint16_t year);
```

Sets the RTC date.

**Parameters**:
- `dayOfWeek`: Day of week (1-7, Monday=1)
- `date`: Day of month (1-31)
- `month`: Month (1-12)
- `year`: Year (2000-2099)

**Returns**:
- 0: Success
- Non-zero: I2C communication error or invalid parameters

**Usage Example**:
```c
// Set date to Monday, March 15, 2024
if (DS3231_SetDate(1, 15, 3, 2024) == 0)
{
    printf("Date set successfully\n");
}
```

**Validation**:
- dayOfWeek: 1-7
- date: 1-31
- month: 1-12
- year: 2000-2099

### Temperature Reading

#### Read Temperature

```c
float DS3231_ReadTemperature(void);
```

Reads the internal temperature sensor.

**Returns**: Temperature in Celsius (-40.0 to +85.0)

**Usage Example**:
```c
float rtcTemp = DS3231_ReadTemperature();
printf("RTC Temperature: %.2f C\n", rtcTemp);
```

**Resolution**: 0.25°C (11-bit value)

**Conversion Formula**:
```
Temperature (°C) = (MSB << 2 | LSB >> 6) * 0.25
```

**I2C Transaction**:
- Read 2 bytes starting at register 0x11
- Combine bytes and convert to float

**Update Rate**: Temperature is updated every 64 seconds automatically.

### Alarm Functions

#### Set Alarm 1

```c
uint8_t DS3231_SetAlarm1(DS3231_AlarmTime *alarmTime, uint8_t alarmMode);
```

Configures Alarm 1 (with seconds precision).

**Parameters**:
- `alarmTime`: Pointer to alarm time structure
- `alarmMode`: Alarm matching mode (0-15)

**Alarm Modes**:
```
Mode  Match Condition
----  -------------------------------------------------------
0     Alarm once per second
1     Alarm when seconds match
2     Alarm when minutes and seconds match
3     Alarm when hours, minutes, and seconds match
4     Alarm when date, hours, minutes, and seconds match
5     Alarm when day of week, hours, minutes, seconds match
```

**Usage Example**:
```c
// Alarm at 06:30:00 every day
DS3231_AlarmTime alarm1;
alarm1.hour = 6;
alarm1.minute = 30;
alarm1.second = 0;
alarm1.date = 1;  // Don't care

DS3231_SetAlarm1(&alarm1, 3);  // Match hours, minutes, seconds
DS3231_EnableAlarm1(1);
```

#### Set Alarm 2

```c
uint8_t DS3231_SetAlarm2(DS3231_AlarmTime *alarmTime, uint8_t alarmMode);
```

Configures Alarm 2 (minute precision, no seconds).

**Parameters**:
- `alarmTime`: Pointer to alarm time structure (second field ignored)
- `alarmMode`: Alarm matching mode (0-7)

**Alarm Modes**:
```
Mode  Match Condition
----  -------------------------------------------------------
0     Alarm once per minute (at 00 seconds)
1     Alarm when minutes match
2     Alarm when hours and minutes match
3     Alarm when date, hours, and minutes match
4     Alarm when day of week, hours, minutes match
```

**Usage Example**:
```c
// Alarm at 12:00 every day
DS3231_AlarmTime alarm2;
alarm2.hour = 12;
alarm2.minute = 0;

DS3231_SetAlarm2(&alarm2, 2);  // Match hours and minutes
DS3231_EnableAlarm2(1);
```

#### Enable/Disable Alarms

```c
void DS3231_EnableAlarm1(uint8_t enable);
void DS3231_EnableAlarm2(uint8_t enable);
```

**Parameters**:
- `enable`: 1 to enable, 0 to disable

**Usage Example**:
```c
DS3231_EnableAlarm1(1);  // Enable Alarm 1
DS3231_EnableAlarm2(0);  // Disable Alarm 2
```

#### Clear Alarm Flags

```c
void DS3231_ClearAlarm1Flag(void);
void DS3231_ClearAlarm2Flag(void);
```

Clears the alarm interrupt flag.

**Important**: Must be called after alarm triggers to allow next alarm.

**Usage Example**:
```c
// In alarm interrupt handler
void DS3231_AlarmISR(void)
{
    // Check which alarm triggered
    uint8_t status = DS3231_ReadStatus();
    
    if (status & 0x01)  // Alarm 1 flag
    {
        DS3231_ClearAlarm1Flag();
        // Handle Alarm 1 event
    }
    
    if (status & 0x02)  // Alarm 2 flag
    {
        DS3231_ClearAlarm2Flag();
        // Handle Alarm 2 event
    }
}
```

### Control Register Functions

#### Enable/Disable Square Wave Output

```c
void DS3231_EnableSquareWave(uint8_t enable);
```

Enables or disables the 32kHz output pin.

**Parameters**:
- `enable`: 1 to enable, 0 to disable

#### Set Square Wave Frequency

```c
void DS3231_SetSquareWaveFrequency(uint8_t frequency);
```

Sets the SQW/INT pin output frequency.

**Parameters**:
- `frequency`: 0=1Hz, 1=1.024kHz, 2=4.096kHz, 3=8.192kHz

**Usage Example**:
```c
DS3231_SetSquareWaveFrequency(0);  // 1Hz output
DS3231_EnableSquareWave(1);        // Enable output
```

## BCD Conversion

The DS3231 stores values in BCD (Binary-Coded Decimal) format.

### BCD to Binary

```c
static uint8_t BCD_To_Decimal(uint8_t bcd)
{
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}
```

**Example**: 
- BCD 0x23 → Binary 23
- BCD 0x59 → Binary 59

### Binary to BCD

```c
static uint8_t Decimal_To_BCD(uint8_t decimal)
{
    return ((decimal / 10) << 4) | (decimal % 10);
}
```

**Example**:
- Binary 23 → BCD 0x23
- Binary 59 → BCD 0x59

## Usage Examples

### Complete Time/Date Read

```c
void ReadAndDisplayDateTime(void)
{
    DS3231_Time time;
    DS3231_Date date;
    
    if (DS3231_ReadTime(&time) == 0 && DS3231_ReadDate(&date) == 0)
    {
        printf("Date: %04d-%02d-%02d\n",
               date.year, date.month, date.date);
        printf("Time: %02d:%02d:%02d\n",
               time.hour, time.minute, time.second);
        printf("Day of Week: %d\n", date.dayOfWeek);
    }
    else
    {
        printf("Error reading RTC\n");
    }
}
```

### Set Complete Date and Time

```c
void SetDateTimeExample(void)
{
    // Set date: Monday, March 15, 2024
    DS3231_SetDate(1, 15, 3, 2024);
    
    // Set time: 14:35:00
    DS3231_SetTime(14, 35, 0);
    
    printf("RTC initialized to 2024-03-15 14:35:00\n");
}
```

### Daily Alarm Example

```c
void SetupDailyAlarm(void)
{
    // Wake up at 06:00:00 every day
    DS3231_AlarmTime wakeUpTime;
    wakeUpTime.hour = 6;
    wakeUpTime.minute = 0;
    wakeUpTime.second = 0;
    
    // Set Alarm 1 to match hours, minutes, seconds
    DS3231_SetAlarm1(&wakeUpTime, 3);
    DS3231_EnableAlarm1(1);
    
    printf("Daily alarm set for 06:00:00\n");
}
```

### Temperature Monitoring

```c
void MonitorRTCTemperature(void)
{
    while (1)
    {
        float temp = DS3231_ReadTemperature();
        printf("RTC Temp: %.2f C\n", temp);
        
        HAL_Delay(60000);  // Read every minute
    }
}
```

## Integration with Command Parser

### SET TIME Command

```c
void SET_TIME_PARSER(uint8_t argc, char **argv)
{
    if (argc != 4)
    {
        PRINT_CLI("Usage: SET TIME HH MM SS\r\n");
        return;
    }
    
    uint8_t hour = atoi(argv[2]);
    uint8_t minute = atoi(argv[3]);
    uint8_t second = atoi(argv[4]);
    
    if (DS3231_SetTime(hour, minute, second) == 0)
    {
        PRINT_CLI("Time set successfully\r\n");
    }
    else
    {
        PRINT_CLI("Error: Invalid time\r\n");
    }
}
```

### SET DATE Command

```c
void SET_DATE_PARSER(uint8_t argc, char **argv)
{
    if (argc != 5)
    {
        PRINT_CLI("Usage: SET DATE YYYY MM DD\r\n");
        return;
    }
    
    uint16_t year = atoi(argv[2]);
    uint8_t month = atoi(argv[3]);
    uint8_t date = atoi(argv[4]);
    uint8_t dayOfWeek = 1;  // Default to Monday
    
    if (DS3231_SetDate(dayOfWeek, date, month, year) == 0)
    {
        PRINT_CLI("Date set successfully\r\n");
    }
    else
    {
        PRINT_CLI("Error: Invalid date\r\n");
    }
}
```

## Error Handling

### I2C Communication Errors

All read/write functions return error codes:

```c
uint8_t result = DS3231_ReadTime(&time);
if (result != 0)
{
    switch (result)
    {
        case HAL_TIMEOUT:
            printf("I2C timeout\n");
            break;
        case HAL_ERROR:
            printf("I2C error\n");
            break;
        default:
            printf("Unknown error\n");
    }
}
```

### Invalid Parameter Handling

Functions validate input parameters:

```c
uint8_t DS3231_SetTime(uint8_t hour, uint8_t minute, uint8_t second)
{
    // Validate parameters
    if (hour > 23 || minute > 59 || second > 59)
    {
        return 1;  // Invalid parameter
    }
    
    // Proceed with I2C write
    // ...
}
```

## Performance Characteristics

### I2C Transaction Times

- Single byte read: ~100 microseconds
- Time read (3 bytes): ~200 microseconds
- Date read (4 bytes): ~250 microseconds
- Time write (3 bytes): ~250 microseconds
- Temperature read (2 bytes): ~150 microseconds

### Total Operation Times

- `DS3231_ReadTime()`: 200-300 µs
- `DS3231_ReadDate()`: 250-350 µs
- `DS3231_SetTime()`: 250-400 µs
- `DS3231_ReadTemperature()`: 150-250 µs

## Power Consumption

### Active Mode

- Timekeeping: 200 µA typical
- Temperature conversion: +200 µA during conversion

### Battery Backup Mode

- Timekeeping only: 3 µA typical
- Battery life: 3-5 years with CR2032 coin cell

## Best Practices

### 1. Check Return Values

```c
// Always check for I2C errors
if (DS3231_ReadTime(&time) != 0)
{
    // Handle error
    return;
}
```

### 2. Initialize Once

```c
// Call DS3231_Init() once during system startup
void SystemInit(void)
{
    HAL_Init();
    MX_I2C1_Init();
    DS3231_Init();  // Initialize RTC once
}
```

### 3. Clear Alarm Flags

```c
// Always clear alarm flag after handling
void AlarmHandler(void)
{
    DS3231_ClearAlarm1Flag();  // Must clear flag
    // Process alarm event
}
```

### 4. Validate User Input

```c
// Validate before setting time
if (hour <= 23 && minute <= 59 && second <= 59)
{
    DS3231_SetTime(hour, minute, second);
}
```

## Dependencies

### Required

- **STM32 HAL I2C**: Hardware abstraction layer for I2C
- **I2C1 peripheral**: Configured in STM32CubeMX

### Used By

- **display.c**: Reads time/date for LCD display
- **cmd_parser.c**: SET TIME and SET DATE commands
- **sensor_json_output.c**: Timestamp for sensor data

## Summary

The DS3231 RTC Library provides:
- Complete real-time clock functionality
- High-precision timekeeping (±2ppm)
- Temperature monitoring
- Dual alarms with flexible configuration
- Battery backup support
- Simple I2C interface
- BCD conversion handled internally
- Robust error handling

This driver enables accurate timekeeping and scheduling capabilities for the Datalogger system with minimal CPU overhead.
