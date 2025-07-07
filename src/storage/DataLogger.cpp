/* *****************************************************************
    *                        INFORMATION                          *
   ***************************************************************** 
   
    This file implements a data logging system for the AeroSense
    project. It uses ESP32 internal flash storage (SPIFFS) as a
    ring buffer to store sensor readings with timestamps. The system
    supports offline data collection and batch download via Bluetooth.
   
*/

/* *****************************************************************
    *                     FILE CONFIGURATION                      *
   ***************************************************************** */

/* ---------------------- NECESSARY HEADERS ---------------------- */

// Include the header for the data logger
#include "DataLogger.hpp"

// Time functions
#include <time.h>

/* ---------------------- GLOBAL VARIABLES ---------------------- */

// Current logger configuration
t_loggerConfig logger_config;

// Download status
t_downloadStatus download_status;

// Current session record counter
uint16_t session_record_count = 0;

/* ------------------ PRIVATE FUNCTIONS PROTOTYPES ----------------- */

// Calculates checksum for a data record
// @param record: Pointer to the data record
// @return: Calculated checksum
uint16_t calculateRecordChecksum(t_dataRecord* record);

// Loads configuration from flash
// @return: 1 if successful, 0 otherwise
int loadConfig();

// Saves configuration to flash
// @return: 1 if successful, 0 otherwise
int saveConfig();

// Gets current Unix timestamp
// @return: Current timestamp in seconds
uint32_t getCurrentTimestamp();

// Writes a record to flash at specific position
// @param record: Record to write
// @param position: Position in ring buffer
// @return: 1 if successful, 0 otherwise
int writeRecordAtPosition(t_dataRecord* record, uint16_t position);

// Reads a record from flash at specific position
// @param record: Pointer to store the record
// @param position: Position in ring buffer
// @return: 1 if successful, 0 otherwise
int readRecordAtPosition(t_dataRecord* record, uint16_t position);

/* *****************************************************************
    *                        INIT FUNCTION                        *
   ***************************************************************** */

// Initializes the data logging system
int initDataLogger()
{
    // Initialize SPIFFS file system
    if (!SPIFFS.begin(true))
    {
        Serial.println("SPIFFS initialization failed");
        return 0;
    }

    // Load existing configuration or create new one
    if (!loadConfig())
    {
        // Create default configuration
        logger_config.total_records = 0;
        logger_config.current_index = 0;
        logger_config.oldest_index = 0;
        logger_config.session_start = 0;
        logger_config.logging_state = LOGGING_STOPPED;
        logger_config.total_logged = 0;
        
        // Save default configuration
        if (!saveConfig())
        {
            Serial.println("Failed to save default configuration");
            return 0;
        }
    }

    // Initialize download status
    download_status.total_records = 0;
    download_status.downloaded = 0;
    download_status.download_active = 0;
    download_status.session_start = 0;

    session_record_count = 0;

    Serial.println("Data Logger initialized successfully");
    Serial.printf("Total records: %d, Current index: %d\n", 
                  logger_config.total_records, logger_config.current_index);

    return 1;
}

/* *****************************************************************
    *                      LOGGING FUNCTIONS                      *
   ***************************************************************** */

// Starts data logging session
int startLogging()
{
    if (logger_config.logging_state == LOGGING_ACTIVE)
    {
        Serial.println("Logging already active");
        return 1;
    }

    logger_config.logging_state = LOGGING_ACTIVE;
    logger_config.session_start = getCurrentTimestamp();
    session_record_count = 0;

    if (!saveConfig())
    {
        Serial.println("Failed to save logging state");
        return 0;
    }

    Serial.println("Data logging started");
    return 1;
}

// Stops data logging session
int stopLogging()
{
    if (logger_config.logging_state != LOGGING_ACTIVE)
    {
        Serial.println("Logging not active");
        return 1;
    }

    logger_config.logging_state = LOGGING_STOPPED;

    if (!saveConfig())
    {
        Serial.println("Failed to save logging state");
        return 0;
    }

    Serial.printf("Data logging stopped. Session recorded %d entries\n", session_record_count);
    return 1;
}

// Logs a complete sensor data record
int logSensorData(t_dataBME680* bme680, t_dataMHZ19B* mhz19b, t_dataMQ4* mq4, 
                  t_dataMQ7* mq7, t_dataMQ131* mq131, t_dataMQ137* mq137, 
                  t_dataPixhawk* pixhawk)
{
    // Check if logging is active
    if (logger_config.logging_state != LOGGING_ACTIVE)
    {
        return 0; // Not logging, return silently
    }

    // Create new data record
    t_dataRecord record;
    
    // Fill metadata
    record.timestamp = getCurrentTimestamp();
    record.record_id = logger_config.total_logged + 1;
    record.data_valid = 0xFF; // Assume all data valid initially
    
    // Copy sensor data
    record.bme680 = *bme680;
    record.mhz19b = *mhz19b;
    record.mq4 = *mq4;
    record.mq7 = *mq7;
    record.mq131 = *mq131;
    record.mq137 = *mq137;
    record.pixhawk = *pixhawk;
    
    // Calculate checksum
    record.checksum = calculateRecordChecksum(&record);
    
    // Write record to current position
    if (!writeRecordAtPosition(&record, logger_config.current_index))
    {
        Serial.println("Failed to write data record");
        return 0;
    }

    // Update ring buffer indices
    logger_config.current_index = (logger_config.current_index + 1) % MAX_FLASH_RECORDS;
    
    // Update record count
    if (logger_config.total_records < MAX_FLASH_RECORDS)
    {
        logger_config.total_records++;
    }
    else
    {
        // Ring buffer is full, update oldest index
        logger_config.oldest_index = (logger_config.oldest_index + 1) % MAX_FLASH_RECORDS;
    }
    
    logger_config.total_logged++;
    session_record_count++;

    // Save configuration every 10 records to reduce wear
    if (session_record_count % 10 == 0)
    {
        saveConfig();
    }

    return 1;
}

/* *****************************************************************
    *                     DOWNLOAD FUNCTIONS                      *
   ***************************************************************** */

// Prepares for data download
uint16_t prepareDownload()
{
    download_status.total_records = logger_config.total_records;
    download_status.downloaded = 0;
    download_status.download_active = 1;
    download_status.session_start = getCurrentTimestamp();

    logger_config.logging_state = LOGGING_DOWNLOAD_MODE;
    saveConfig();

    Serial.printf("Prepared %d records for download\n", download_status.total_records);
    return download_status.total_records;
}

// Downloads a specific record by index
int downloadRecord(uint16_t index, t_dataRecord* record)
{
    if (!download_status.download_active)
    {
        Serial.println("Download session not active");
        return 0;
    }

    if (index >= logger_config.total_records)
    {
        Serial.printf("Invalid record index: %d\n", index);
        return 0;
    }

    // Calculate actual position in ring buffer
    uint16_t actual_position = (logger_config.oldest_index + index) % MAX_FLASH_RECORDS;
    
    if (!readRecordAtPosition(record, actual_position))
    {
        Serial.printf("Failed to read record at position %d\n", actual_position);
        return 0;
    }

    // Verify checksum
    uint16_t calculated_checksum = calculateRecordChecksum(record);
    if (calculated_checksum != record->checksum)
    {
        Serial.printf("Checksum mismatch for record %d\n", index);
        return 0;
    }

    download_status.downloaded++;
    return 1;
}

// Gets download progress status
void getDownloadStatus(t_downloadStatus* status)
{
    *status = download_status;
}

// Clears all stored data (after successful download)
int clearStoredData()
{
    // Reset configuration
    logger_config.total_records = 0;
    logger_config.current_index = 0;
    logger_config.oldest_index = 0;
    logger_config.logging_state = LOGGING_STOPPED;

    // Reset download status
    download_status.total_records = 0;
    download_status.downloaded = 0;
    download_status.download_active = 0;

    session_record_count = 0;

    // Save configuration
    if (!saveConfig())
    {
        Serial.println("Failed to save cleared configuration");
        return 0;
    }

    Serial.println("All stored data cleared");
    return 1;
}

/* *****************************************************************
    *                     STATUS FUNCTIONS                        *
   ***************************************************************** */

// Gets current logging status
void getLoggingStatus(t_loggerConfig* config)
{
    *config = logger_config;
}

// Formats the internal storage (emergency function)
int formatStorage()
{
    if (!SPIFFS.format())
    {
        Serial.println("Failed to format SPIFFS");
        return 0;
    }

    // Reinitialize after format
    return initDataLogger();
}

// Gets storage usage statistics
int getStorageStats(uint32_t* used_bytes, uint32_t* total_bytes)
{
    *total_bytes = SPIFFS.totalBytes();
    *used_bytes = SPIFFS.usedBytes();
    return 1;
}

/* *****************************************************************
    *                    PRIVATE FUNCTIONS                        *
   ***************************************************************** */

// Calculates checksum for a data record
uint16_t calculateRecordChecksum(t_dataRecord* record)
{
    uint16_t checksum = 0;
    uint8_t* data = (uint8_t*)record;
    
    // Calculate checksum for all data except the checksum field itself
    for (size_t i = 0; i < sizeof(t_dataRecord) - sizeof(uint16_t); i++)
    {
        checksum += data[i];
    }
    
    return checksum;
}

// Loads configuration from flash
int loadConfig()
{
    File config_file = SPIFFS.open(CONFIG_FILE, "r");
    if (!config_file)
    {
        Serial.println("No existing configuration found");
        return 0;
    }

    size_t bytes_read = config_file.readBytes((char*)&logger_config, sizeof(t_loggerConfig));
    config_file.close();

    if (bytes_read != sizeof(t_loggerConfig))
    {
        Serial.println("Invalid configuration file size");
        return 0;
    }

    Serial.println("Configuration loaded successfully");
    return 1;
}

// Saves configuration to flash
int saveConfig()
{
    File config_file = SPIFFS.open(CONFIG_FILE, "w");
    if (!config_file)
    {
        Serial.println("Failed to open configuration file for writing");
        return 0;
    }

    size_t bytes_written = config_file.write((uint8_t*)&logger_config, sizeof(t_loggerConfig));
    config_file.close();

    if (bytes_written != sizeof(t_loggerConfig))
    {
        Serial.println("Failed to write complete configuration");
        return 0;
    }

    return 1;
}

// Gets current Unix timestamp
uint32_t getCurrentTimestamp()
{
    // For now, use millis() / 1000 as a simple timestamp
    // In a real implementation, you might want to use RTC or NTP
    return millis() / 1000;
}

// Writes a record to flash at specific position
int writeRecordAtPosition(t_dataRecord* record, uint16_t position)
{
    File data_file = SPIFFS.open(DATA_FILE, "r+");
    if (!data_file)
    {
        // File doesn't exist, create it
        data_file = SPIFFS.open(DATA_FILE, "w+");
        if (!data_file)
        {
            Serial.println("Failed to create data file");
            return 0;
        }
    }

    // Seek to position
    if (!data_file.seek(position * sizeof(t_dataRecord)))
    {
        Serial.printf("Failed to seek to position %d\n", position);
        data_file.close();
        return 0;
    }

    // Write record
    size_t bytes_written = data_file.write((uint8_t*)record, sizeof(t_dataRecord));
    data_file.close();

    if (bytes_written != sizeof(t_dataRecord))
    {
        Serial.printf("Failed to write complete record at position %d\n", position);
        return 0;
    }

    return 1;
}

// Reads a record from flash at specific position
int readRecordAtPosition(t_dataRecord* record, uint16_t position)
{
    File data_file = SPIFFS.open(DATA_FILE, "r");
    if (!data_file)
    {
        Serial.println("Failed to open data file for reading");
        return 0;
    }

    // Seek to position
    if (!data_file.seek(position * sizeof(t_dataRecord)))
    {
        Serial.printf("Failed to seek to position %d\n", position);
        data_file.close();
        return 0;
    }

    // Read record
    size_t bytes_read = data_file.readBytes((char*)record, sizeof(t_dataRecord));
    data_file.close();

    if (bytes_read != sizeof(t_dataRecord))
    {
        Serial.printf("Failed to read complete record at position %d\n", position);
        return 0;
    }

    return 1;
}
