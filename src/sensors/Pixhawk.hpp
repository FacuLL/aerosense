/* *****************************************************************
    *                    HEADER CONFIGURATION                     *
   ***************************************************************** */

// Ensure the header is included only once
#ifndef PIXHAWK_hpp
#define PIXHAWK_hpp

/* --------------------- NECESSARY LIBRARIES --------------------- */

// Standard integer types for fixed-width integer definitions
#include <stdint.h>

// Provides the HardwareSerial class for UART communication
#include <HardwareSerial.h>

/* -------------------- MACROS AND CONSTANTS -------------------- */

// UART baud rate for Pixhawk communication (standard MAVLink rate)
#define PIXHAWK_BAUD 57600

// Pin definitions for RX and TX (Serial2 on ESP32)
#define PIXHAWK_RX 16
#define PIXHAWK_TX 17

// MAVLink message IDs we're interested in
#define MAVLINK_MSG_ID_GPS_RAW_INT 24
#define MAVLINK_MSG_ID_GLOBAL_POSITION_INT 33

/* ---------------------- DATA STRUCTURES ---------------------- */

// Structure to hold GPS and altitude data from Pixhawk
typedef struct
{
    // GPS latitude in degrees (scaled by 1e7)
    double latitude;

    // GPS longitude in degrees (scaled by 1e7)  
    double longitude;

    // Altitude above sea level in meters
    float altitude;

    // Relative altitude above home in meters
    float relative_altitude;

    // Number of visible satellites
    uint8_t satellites_visible;

    // GPS fix type (0=no fix, 1=no fix, 2=2D, 3=3D, 4=DGPS, 5=RTK)
    uint8_t fix_type;

    // GPS horizontal dilution of precision
    uint16_t hdop;

    // Data validity flag
    uint8_t data_valid;

} t_dataPixhawk;

/* ---------------- PUBLIC FUNCTIONS PROTOTYPES ---------------- */

// Initializes the Pixhawk communication
// @return: 1 if successful, 0 otherwise
int initPixhawk();

// Retrieves GPS and altitude data from the Pixhawk
// @param newData: Pointer to structure where data will be stored
void getDataPixhawk(t_dataPixhawk *newData);

// Processes incoming MAVLink messages
// @return: 1 if valid GPS message received, 0 otherwise
int processMAVLinkMessages();

#endif // PIXHAWK_HPP
