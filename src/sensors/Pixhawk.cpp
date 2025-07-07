/* *****************************************************************
    *                        INFORMATION                          *
   ***************************************************************** 
   
    This file handles the initialization and data retrieval 
    from the Pixhawk autopilot. The system communicates via UART 
    using a simplified MAVLink protocol parser to extract GPS 
    coordinates and altitude information.
   
*/

/* *****************************************************************
    *                     FILE CONFIGURATION                      *
   ***************************************************************** */

/* ---------------------- NECESSARY HEADERS ---------------------- */

// Includes the header for the Pixhawk interface
#include "Pixhawk.hpp"

/* ---------------------- GLOBAL VARIABLES ---------------------- */

// HardwareSerial object for UART communication with Pixhawk
HardwareSerial uartPixhawk(2);

// Buffer for incoming MAVLink messages
uint8_t mavlink_buffer[300];
uint16_t buffer_index = 0;

// Latest GPS data
t_dataPixhawk latest_gps_data;

/* ------------------ PRIVATE FUNCTIONS PROTOTYPES ----------------- */

// Parses a complete MAVLink message
// @param buffer: Buffer containing the message
// @param length: Length of the message
// @return: 1 if GPS data was updated, 0 otherwise
int parseMAVLinkMessage(uint8_t* buffer, uint16_t length);

// Extracts GPS data from GPS_RAW_INT message
// @param payload: Message payload
void parseGPSRawInt(uint8_t* payload);

// Extracts position data from GLOBAL_POSITION_INT message  
// @param payload: Message payload
void parseGlobalPositionInt(uint8_t* payload);

// Converts int32_t to float for coordinates
// @param value: Integer value scaled by 1e7
// @return: Float value in degrees
double int32ToCoordinate(int32_t value);

// Calculates MAVLink checksum
// @param data: Data to calculate checksum for
// @param length: Length of data
// @return: Calculated checksum
uint8_t calculateChecksum(uint8_t* data, uint16_t length);

/* *****************************************************************
    *                        INIT FUNCTION                        *
   ***************************************************************** */

// Initializes the Pixhawk communication
// @return: 1 if successful, 0 otherwise
int initPixhawk()
{
    // Begin UART communication with specified baud rate and pins
    uartPixhawk.begin(PIXHAWK_BAUD, SERIAL_8N1, PIXHAWK_RX, PIXHAWK_TX);
    
    // Initialize GPS data structure
    latest_gps_data.latitude = 0.0;
    latest_gps_data.longitude = 0.0;
    latest_gps_data.altitude = 0.0;
    latest_gps_data.relative_altitude = 0.0;
    latest_gps_data.satellites_visible = 0;
    latest_gps_data.fix_type = 0;
    latest_gps_data.hdop = 65535;
    latest_gps_data.data_valid = 0;
    
    buffer_index = 0;
    
    return 1;
}

/* *****************************************************************
    *                      GET DATA FUNCTION                      *
   ***************************************************************** */

// Retrieves GPS and altitude data from the Pixhawk
// @param newData: Pointer to structure where data will be stored
void getDataPixhawk(t_dataPixhawk *newData)
{
    // Process any incoming MAVLink messages
    processMAVLinkMessages();
    
    // Copy the latest GPS data
    *newData = latest_gps_data;
}

/* *****************************************************************
    *                    MAVLINK PROCESSING                       *
   ***************************************************************** */

// Processes incoming MAVLink messages
// @return: 1 if valid GPS message received, 0 otherwise
int processMAVLinkMessages()
{
    int gps_updated = 0;
    
    // Read available bytes from UART
    while (uartPixhawk.available())
    {
        uint8_t byte = uartPixhawk.read();
        
        // Look for MAVLink start byte (0xFE for MAVLink v1.0)
        if (buffer_index == 0 && byte != 0xFE)
        {
            continue; // Skip until we find start byte
        }
        
        // Store byte in buffer
        if (buffer_index < sizeof(mavlink_buffer))
        {
            mavlink_buffer[buffer_index++] = byte;
        }
        
        // Check if we have enough bytes for header
        if (buffer_index >= 6)
        {
            uint8_t payload_length = mavlink_buffer[1];
            uint16_t expected_length = payload_length + 8; // Header + payload + checksum
            
            // Check if we have complete message
            if (buffer_index >= expected_length)
            {
                // Verify checksum
                uint8_t received_checksum = mavlink_buffer[expected_length - 1];
                uint8_t calculated_checksum = calculateChecksum(mavlink_buffer + 1, expected_length - 2);
                
                if (received_checksum == calculated_checksum)
                {
                    // Parse the message
                    if (parseMAVLinkMessage(mavlink_buffer, expected_length))
                    {
                        gps_updated = 1;
                    }
                }
                
                // Reset buffer for next message
                buffer_index = 0;
            }
        }
        
        // Prevent buffer overflow
        if (buffer_index >= sizeof(mavlink_buffer))
        {
            buffer_index = 0;
        }
    }
    
    return gps_updated;
}

/* *****************************************************************
    *                    PRIVATE FUNCTIONS                        *
   ***************************************************************** */

// Parses a complete MAVLink message
int parseMAVLinkMessage(uint8_t* buffer, uint16_t length)
{
    if (length < 8) return 0;
    
    uint8_t message_id = buffer[5];
    uint8_t* payload = buffer + 6;
    
    switch (message_id)
    {
        case MAVLINK_MSG_ID_GPS_RAW_INT:
            parseGPSRawInt(payload);
            return 1;
            
        case MAVLINK_MSG_ID_GLOBAL_POSITION_INT:
            parseGlobalPositionInt(payload);
            return 1;
            
        default:
            return 0;
    }
}

// Extracts GPS data from GPS_RAW_INT message
void parseGPSRawInt(uint8_t* payload)
{
    // GPS_RAW_INT message structure (simplified)
    // uint64_t time_usec      - offset 0
    // uint8_t fix_type        - offset 8  
    // int32_t lat             - offset 9
    // int32_t lon             - offset 13
    // int32_t alt             - offset 17
    // uint16_t eph            - offset 21
    // uint16_t epv            - offset 23
    // uint16_t vel            - offset 25
    // uint16_t cog            - offset 27
    // uint8_t satellites_visible - offset 29
    
    latest_gps_data.fix_type = payload[8];
    
    // Extract coordinates (scaled by 1e7)
    int32_t lat_raw = *((int32_t*)(payload + 9));
    int32_t lon_raw = *((int32_t*)(payload + 13));
    int32_t alt_raw = *((int32_t*)(payload + 17));
    
    latest_gps_data.latitude = int32ToCoordinate(lat_raw);
    latest_gps_data.longitude = int32ToCoordinate(lon_raw);
    latest_gps_data.altitude = alt_raw / 1000.0f; // Convert mm to m
    
    latest_gps_data.hdop = *((uint16_t*)(payload + 21));
    latest_gps_data.satellites_visible = payload[29];
    
    // Mark data as valid if we have a 3D fix
    latest_gps_data.data_valid = (latest_gps_data.fix_type >= 3) ? 1 : 0;
}

// Extracts position data from GLOBAL_POSITION_INT message
void parseGlobalPositionInt(uint8_t* payload)
{
    // GLOBAL_POSITION_INT message structure (simplified)
    // uint32_t time_boot_ms   - offset 0
    // int32_t lat             - offset 4
    // int32_t lon             - offset 8
    // int32_t alt             - offset 12
    // int32_t relative_alt    - offset 16
    
    int32_t lat_raw = *((int32_t*)(payload + 4));
    int32_t lon_raw = *((int32_t*)(payload + 8));
    int32_t alt_raw = *((int32_t*)(payload + 12));
    int32_t rel_alt_raw = *((int32_t*)(payload + 16));
    
    latest_gps_data.latitude = int32ToCoordinate(lat_raw);
    latest_gps_data.longitude = int32ToCoordinate(lon_raw);
    latest_gps_data.altitude = alt_raw / 1000.0f; // Convert mm to m
    latest_gps_data.relative_altitude = rel_alt_raw / 1000.0f; // Convert mm to m
    
    latest_gps_data.data_valid = 1;
}

// Converts int32_t to float for coordinates
double int32ToCoordinate(int32_t value)
{
    return (double)value / 1e7;
}

// Calculates MAVLink checksum
uint8_t calculateChecksum(uint8_t* data, uint16_t length)
{
    uint8_t checksum = 0xFF;
    
    for (uint16_t i = 0; i < length; i++)
    {
        checksum ^= data[i];
    }
    
    return checksum;
}
