/* *****************************************************************
    *                    HEADER CONFIGURATION                     *
   ***************************************************************** */

// Ensure the header is included only once
#ifndef SDLOGGER_hpp
#define SDLOGGER_hpp

/* --------------------- NECESSARY LIBRARIES --------------------- */

// Standard integer types
#include <stdint.h>

// SD card libraries for ESP32
#include "FS.h"
#include "SD.h"
#include "SPI.h"

// Standard integer types
#include <stdint.h>

// Include sensor data structures
#include "../sensors/BME680.hpp"
#include "../sensors/MH-Z19B.hpp"
#include "../sensors/MQ-4.hpp"
#include "../sensors/MQ-7.hpp"
#include "../sensors/MQ-131.hpp"
#include "../sensors/MQ-137.hpp"
#include "../sensors/Pixhawk.hpp"

/* -------------------- MACROS AND CONSTANTS -------------------- */

// SD Card SPI pins for ESP32
#define SD_CS_PIN 5
#define SD_MOSI_PIN 23
#define SD_MISO_PIN 19
#define SD_SCK_PIN 18

// SD card file paths and naming
#define SD_DATA_DIR "/AeroSense"
#define SD_FLIGHT_PREFIX "/flight_"
#define SD_CONFIG_FILE "/aerosense_config.txt"
#define SD_INDEX_FILE "/flight_index.txt"

// File formats
#define CSV_HEADER "Timestamp,RecordID,Temp,Humidity,Pressure,VOC,CO2,CH4,CO,O3,NO2,NH3,CO_MQ137,Latitude,Longitude,Altitude,Satellites,GPS_Fix\n"

// SD card states
#define SD_NOT_INITIALIZED 0
#define SD_READY 1
#define SD_ERROR 2
#define SD_CARD_REMOVED 3

/* ---------------------- DATA STRUCTURES ---------------------- */

// Combined data record for all sensors
typedef struct
{
    uint32_t timestamp;         // Timestamp in seconds
    uint16_t record_id;         // Sequential record ID
    
    // Sensor data
    t_dataBME680 bme680;        // BME680 environmental data
    t_dataMHZ19B mhz19b;        // MH-Z19B CO2 data
    t_dataMQ4 mq4;              // MQ-4 methane data
    t_dataMQ7 mq7;              // MQ-7 carbon monoxide data
    t_dataMQ131 mq131;          // MQ-131 ozone and NO2 data
    t_dataMQ137 mq137;          // MQ-137 ammonia and CO data
    t_dataPixhawk pixhawk;      // Pixhawk GPS and altitude data
    
} t_dataRecord;

// SD card configuration and status
typedef struct
{
    uint8_t sd_state;           // Current SD card state
    uint16_t current_flight;    // Current flight number
    uint32_t total_flights;     // Total flights recorded
    uint32_t records_in_flight; // Records in current flight
    uint32_t total_records_sd;  // Total records on SD card
    uint64_t card_size_mb;      // SD card size in MB
    uint64_t used_space_mb;     // Used space in MB
    
} t_sdConfig;

// Flight session information
typedef struct
{
    uint16_t flight_number;     // Flight session number
    uint32_t start_timestamp;   // Flight start time
    uint32_t end_timestamp;     // Flight end time (0 if ongoing)
    uint32_t record_count;      // Number of records in flight
    char filename[32];          // Flight data filename
    
} t_flightInfo;

/* ---------------- PUBLIC FUNCTIONS PROTOTYPES ---------------- */

// Initializes the SD card logging system
// @return: 1 if successful, 0 otherwise
int initSDLogger();

// Checks if SD card is available and ready
// @return: 1 if ready, 0 otherwise
int isSDCardReady();

// Starts a new flight session on SD card
// @return: Flight number if successful, 0 otherwise
uint16_t startSDFlightSession();

// Ends current flight session
// @return: 1 if successful, 0 otherwise
int endSDFlightSession();

// Logs data record to SD card (CSV format)
// @param record: Data record to log
// @return: 1 if successful, 0 otherwise
int logRecordToSD(t_dataRecord* record);

// Gets SD card status and statistics
// @param config: Pointer to store SD configuration
void getSDStatus(t_sdConfig* config);

// Lists all flights on SD card
// @param flights: Array to store flight information
// @param max_flights: Maximum number of flights to list
// @return: Number of flights found
uint16_t listSDFlights(t_flightInfo* flights, uint16_t max_flights);

// Downloads flight data from SD card via Bluetooth
// @param flight_number: Flight number to download
// @return: 1 if successful, 0 otherwise
int downloadFlightFromSD(uint16_t flight_number);

// Deletes a specific flight from SD card
// @param flight_number: Flight number to delete
// @return: 1 if successful, 0 otherwise
int deleteSDFlight(uint16_t flight_number);

// Formats SD card (emergency function)
// @return: 1 if successful, 0 otherwise
int formatSDCard();

// Gets detailed SD card information
// @param total_size: Pointer to store total size in bytes
// @param used_size: Pointer to store used size in bytes
// @param free_size: Pointer to store free size in bytes
// @return: 1 if successful, 0 otherwise
int getSDCardInfo(uint64_t* total_size, uint64_t* used_size, uint64_t* free_size);

// Verifies SD card integrity
// @return: 1 if card is healthy, 0 if errors detected
int verifySDCard();

#endif // SDLOGGER_HPP
