/* *****************************************************************
    *                    HEADER CONFIGURATION                     *
   ***************************************************************** */

// Ensure the header is included only once
#ifndef DATALOGGER_hpp
#define DATALOGGER_hpp

/* --------------------- NECESSARY LIBRARIES --------------------- */

// Standard integer types
#include <stdint.h>

// File system for ESP32 flash storage
#include "FS.h"
#include "SPIFFS.h"

// Include all sensor data structures
#include "../sensors/BME680.hpp"
#include "../sensors/MH-Z19B.hpp"
#include "../sensors/MQ-4.hpp"
#include "../sensors/MQ-7.hpp"
#include "../sensors/MQ-131.hpp"
#include "../sensors/MQ-137.hpp"
#include "../sensors/Pixhawk.hpp"

/* -------------------- MACROS AND CONSTANTS -------------------- */

// Maximum number of records in flash storage (ring buffer)
#define MAX_FLASH_RECORDS 500

// Data file paths
#define DATA_FILE "/sensor_data.bin"
#define INDEX_FILE "/data_index.bin"
#define CONFIG_FILE "/data_config.bin"

// Data logging states
#define LOGGING_STOPPED 0
#define LOGGING_ACTIVE 1
#define LOGGING_DOWNLOAD_MODE 2

/* ---------------------- DATA STRUCTURES ---------------------- */

// Complete sensor data record structure
typedef struct
{
    // Record metadata
    uint32_t timestamp;         // Unix timestamp (seconds since epoch)
    uint16_t record_id;         // Sequential record ID
    uint8_t data_valid;         // Data validity flags bitfield
    
    // Environmental sensors
    t_dataBME680 bme680;        // Temperature, humidity, pressure, VOC
    t_dataMHZ19B mhz19b;        // CO2
    t_dataMQ4 mq4;              // CH4 (methane)
    t_dataMQ7 mq7;              // CO (carbon monoxide)
    t_dataMQ131 mq131;          // O3 (ozone), NO2
    t_dataMQ137 mq137;          // NH3 (ammonia), CO
    
    // Navigation data
    t_dataPixhawk pixhawk;      // GPS coordinates, altitude
    
    // Data integrity
    uint16_t checksum;          // Simple checksum for data validation
    
} t_dataRecord;

// Data logger configuration and status
typedef struct
{
    uint16_t total_records;     // Total number of records stored
    uint16_t current_index;     // Current write position (ring buffer)
    uint16_t oldest_index;      // Position of oldest record
    uint32_t session_start;     // Session start timestamp
    uint8_t logging_state;      // Current logging state
    uint32_t total_logged;      // Total records logged (lifetime)
    
} t_loggerConfig;

// Download status structure
typedef struct
{
    uint16_t total_records;     // Total records available
    uint16_t downloaded;        // Records downloaded so far
    uint8_t download_active;    // Download session active
    uint32_t session_start;     // Download session start time
    
} t_downloadStatus;

/* ---------------- PUBLIC FUNCTIONS PROTOTYPES ---------------- */

// Initializes the data logging system
// @return: 1 if successful, 0 otherwise
int initDataLogger();

// Starts data logging session
// @return: 1 if successful, 0 otherwise
int startLogging();

// Stops data logging session  
// @return: 1 if successful, 0 otherwise
int stopLogging();

// Logs a complete sensor data record
// @param bme680: BME680 sensor data
// @param mhz19b: MH-Z19B sensor data
// @param mq4: MQ-4 sensor data
// @param mq7: MQ-7 sensor data
// @param mq131: MQ-131 sensor data
// @param mq137: MQ-137 sensor data
// @param pixhawk: Pixhawk GPS/navigation data
// @return: 1 if successful, 0 otherwise
int logSensorData(t_dataBME680* bme680, t_dataMHZ19B* mhz19b, t_dataMQ4* mq4, 
                  t_dataMQ7* mq7, t_dataMQ131* mq131, t_dataMQ137* mq137, 
                  t_dataPixhawk* pixhawk);

// Gets current logging status
// @param config: Pointer to store current configuration
void getLoggingStatus(t_loggerConfig* config);

// Prepares for data download
// @return: Number of records available for download
uint16_t prepareDownload();

// Downloads a specific record by index
// @param index: Record index to download
// @param record: Pointer to store the record data
// @return: 1 if successful, 0 if index invalid
int downloadRecord(uint16_t index, t_dataRecord* record);

// Gets download progress status
// @param status: Pointer to store download status
void getDownloadStatus(t_downloadStatus* status);

// Clears all stored data (after successful download)
// @return: 1 if successful, 0 otherwise
int clearStoredData();

// Formats the internal storage (emergency function)
// @return: 1 if successful, 0 otherwise
int formatStorage();

// Gets storage usage statistics
// @param used_bytes: Pointer to store used bytes
// @param total_bytes: Pointer to store total bytes
// @return: 1 if successful, 0 otherwise
int getStorageStats(uint32_t* used_bytes, uint32_t* total_bytes);

#endif // DATALOGGER_HPP
