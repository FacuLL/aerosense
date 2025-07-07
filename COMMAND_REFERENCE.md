# AeroSense Command Reference

## Overview

The AeroSense system now uses intuitive, human-readable commands for all operations. This makes the system much more user-friendly and professional.

## Primary Commands

### Measurement Control
```
START                - Start measuring and logging (recommended)
STOP                 - Stop measuring and logging (recommended)
BEGIN                - Alternative to START
END                  - Alternative to STOP
START_MEASURING      - Full command name
STOP_MEASURING       - Full command name
```

### System Information
```
STATUS               - Show SD card and system status
INFO                 - Alternative to STATUS  
STORAGE_INFO         - Detailed SD card storage information
HELP                 - Show all available commands
?                    - Alternative to HELP
COMMANDS             - Alternative to HELP
```

### Flight Management
```
START_FLIGHT         - Start new flight session (manual)
NEW_FLIGHT           - Alternative to START_FLIGHT
END_FLIGHT           - End current flight session (manual)
CLOSE_FLIGHT         - Alternative to END_FLIGHT
LIST_FLIGHTS         - List all recorded flights
FLIGHTS              - Alternative to LIST_FLIGHTS
```

### Data Retrieval
```
DOWNLOAD_FLIGHT:N    - Download flight number N data
SD_DOWNLOAD_FLIGHT:N - Full SD command version
SD_LIST_FLIGHTS      - Detailed flight list with metadata
```

### Maintenance Commands
```
SD_VERIFY            - Verify SD card integrity
SD_DELETE_FLIGHT:N   - Delete flight number N
CLEAR_DATA           - Format SD card (WARNING: deletes all data!)
FORMAT_SD            - Alternative to CLEAR_DATA
SD_INFO              - Detailed SD card technical information
```

### Legacy Commands (Backward Compatibility)
```
1                    - Start measuring (old style)
0                    - Stop measuring (old style)
LOG_START            - Legacy logging command
LOG_STOP             - Legacy logging command
LOG_STATUS           - Legacy status command
```

## Command Examples

### Basic Usage
```
> START
MEASURING STARTED - SD Flight 1

> STATUS  
SD_STATUS: State=1 Flights=1 Records=45 Size=16384MB Used=12MB

> STOP
MEASURING STOPPED - SD FLIGHT ENDED

> LIST_FLIGHTS
SD_FLIGHTS: 1 total
FLIGHT:1,1641024000,1641025800,45,/AeroSense/flight_0001.csv
```

### Flight Management
```
> NEW_FLIGHT
SD FLIGHT STARTED: 2

> END_FLIGHT  
SD FLIGHT STOPPED

> DOWNLOAD_FLIGHT:1
SD_FLIGHT_DATA_START: 1
Timestamp,RecordID,Temp,Humidity,Pressure,VOC,CO2,CH4,CO,O3,NO2,NH3,CO_MQ137,Latitude,Longitude,Altitude,Satellites,GPS_Fix
1641024000,1,25,60,1013,100,400,2,1,50,20,10,5,40.123456,-74.123456,100.50,8,3
...
SD_FLIGHT_DATA_END: 1
```

### Error Handling
```
> INVALID_COMMAND
UNKNOWN COMMAND: INVALID_COMMAND
Send 'HELP' for available commands

> HELP
=== AeroSense Commands ===
MEASUREMENT:
  START / BEGIN        - Start measuring and logging
  STOP / END           - Stop measuring and logging
...
```

## Command Response Format

All commands provide clear, informative responses:
- **Success**: Descriptive confirmation messages
- **Errors**: Clear error descriptions with suggestions
- **Status**: Structured data with key information
- **Help**: Complete command reference

## Best Practices

1. **Use descriptive commands**: Prefer `START` over `1`
2. **Check status regularly**: Use `STATUS` to monitor system
3. **Get help when needed**: Use `HELP` for command reference
4. **Verify before clearing**: Use `SD_VERIFY` before `CLEAR_DATA`
5. **Download before deleting**: Always backup flights before deletion

## Migration from Legacy Commands

| Old Command | New Command | Notes |
|-------------|-------------|-------|
| `1` | `START` | More intuitive |
| `0` | `STOP` | More intuitive |
| `LOG_STATUS` | `STATUS` | Shorter and clearer |
| `STORAGE_INFO` | `STATUS` or `SD_INFO` | Multiple options |

The system maintains backward compatibility, so old commands still work while you transition to the new command set.
