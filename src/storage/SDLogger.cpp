/* *****************************************************************
    *                        INFORMATION                          *
   ***************************************************************** 
   
    This file implements SD card data logging for the AeroSense
    project. It provides primary storage for flight data with
    CSV format export and flight session management.
   
*/

/* *****************************************************************
    *                     FILE CONFIGURATION                      *
   ***************************************************************** */

/* ---------------------- NECESSARY HEADERS ---------------------- */

// Include the header for the SD logger
#include "SDLogger.hpp"

// Include Bluetooth for data transmission
#include "../protocols/Bluetooth.hpp"

// Time and string functions
#include <time.h>

/* ---------------------- GLOBAL VARIABLES ---------------------- */

// Current SD configuration
t_sdConfig sd_config;

// Current flight session
t_flightInfo current_flight;

// SD card initialization flag
bool sd_initialized = false;

// Current flight file handle
File current_flight_file;

/* ------------------ PRIVATE FUNCTIONS PROTOTYPES ----------------- */

// Loads SD configuration from config file
// @return: 1 if successful, 0 otherwise
int loadSDConfig();

// Saves SD configuration to config file
// @return: 1 if successful, 0 otherwise
int saveSDConfig();

// Creates directory structure on SD card
// @return: 1 if successful, 0 otherwise
int createSDDirectories();

// Generates flight filename
// @param flight_number: Flight number
// @param filename: Buffer to store filename
void generateFlightFilename(uint16_t flight_number, char* filename);

// Updates flight index file
// @param flight_info: Flight information to add
// @return: 1 if successful, 0 otherwise
int updateFlightIndex(t_flightInfo* flight_info);

// Converts data record to CSV line
// @param record: Data record to convert
// @param csv_line: Buffer to store CSV line
void recordToCSV(t_dataRecord* record, char* csv_line);

// Checks SD card health and availability
// @return: Current SD card state
uint8_t checkSDCardHealth();

/* *****************************************************************
    *                        INIT FUNCTION                        *
   ***************************************************************** */

// Initializes the SD card logging system
int initSDLogger()
{
    // Initialize SPI for SD card
    SPI.begin(SD_SCK_PIN, SD_MISO_PIN, SD_MOSI_PIN, SD_CS_PIN);
    
    // Initialize SD card
    if (!SD.begin(SD_CS_PIN))
    {
        Serial.println("SD Card initialization failed");
        sd_config.sd_state = SD_ERROR;
        sd_initialized = false;
        return 0;
    }

    // Check card type
    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE)
    {
        Serial.println("No SD card attached");
        sd_config.sd_state = SD_CARD_REMOVED;
        sd_initialized = false;
        return 0;
    }

    // Print card information
    Serial.print("SD Card Type: ");
    if (cardType == CARD_MMC) Serial.println("MMC");
    else if (cardType == CARD_SD) Serial.println("SDSC");
    else if (cardType == CARD_SDHC) Serial.println("SDHC");
    else Serial.println("UNKNOWN");

    // Get card size
    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);

    // Create directory structure
    if (!createSDDirectories())
    {
        Serial.println("Failed to create SD directories");
        sd_config.sd_state = SD_ERROR;
        return 0;
    }

    // Load or create configuration
    if (!loadSDConfig())
    {
        // Create default configuration
        sd_config.current_flight = 0;
        sd_config.total_flights = 0;
        sd_config.records_in_flight = 0;
        sd_config.total_records_sd = 0;
        sd_config.card_size_mb = cardSize;
        sd_config.used_space_mb = 0;
        
        if (!saveSDConfig())
        {
            Serial.println("Failed to save default SD configuration");
            sd_config.sd_state = SD_ERROR;
            return 0;
        }
    }

    sd_config.sd_state = SD_READY;
    sd_initialized = true;

    Serial.printf("SD Logger initialized: %d flights, %d total records\n", 
                  sd_config.total_flights, sd_config.total_records_sd);

    return 1;
}

/* *****************************************************************
    *                    FLIGHT SESSION FUNCTIONS                 *
   ***************************************************************** */

// Starts a new flight session on SD card
uint16_t startSDFlightSession()
{
    if (!isSDCardReady())
    {
        Serial.println("SD card not ready for flight session");
        return 0;
    }

    // Close previous flight if open
    if (current_flight_file)
    {
        current_flight_file.close();
    }

    // Generate new flight number
    sd_config.current_flight = sd_config.total_flights + 1;
    
    // Create flight info
    current_flight.flight_number = sd_config.current_flight;
    current_flight.start_timestamp = millis() / 1000;
    current_flight.end_timestamp = 0;
    current_flight.record_count = 0;
    
    // Generate filename
    generateFlightFilename(current_flight.flight_number, current_flight.filename);
    
    // Create flight file with CSV header
    current_flight_file = SD.open(current_flight.filename, FILE_WRITE);
    if (!current_flight_file)
    {
        Serial.printf("Failed to create flight file: %s\n", current_flight.filename);
        return 0;
    }
    
    // Write CSV header
    current_flight_file.print(CSV_HEADER);
    current_flight_file.flush();
    
    // Update configuration
    sd_config.total_flights++;
    sd_config.records_in_flight = 0;
    
    Serial.printf("Started SD flight session %d: %s\n", 
                  current_flight.flight_number, current_flight.filename);

    return current_flight.flight_number;
}

// Ends current flight session
int endSDFlightSession()
{
    if (!current_flight_file)
    {
        Serial.println("No active flight session to end");
        return 1; // Not an error if no session active
    }

    // Update flight end time
    current_flight.end_timestamp = millis() / 1000;
    current_flight.record_count = sd_config.records_in_flight;
    
    // Close flight file
    current_flight_file.close();
    
    // Update flight index
    updateFlightIndex(&current_flight);
    
    // Save configuration
    saveSDConfig();
    
    Serial.printf("Ended SD flight session %d: %d records\n", 
                  current_flight.flight_number, current_flight.record_count);

    return 1;
}

/* *****************************************************************
    *                      LOGGING FUNCTIONS                      *
   ***************************************************************** */

// Checks if SD card is available and ready
int isSDCardReady()
{
    if (!sd_initialized)
    {
        return 0;
    }
    
    // Check SD card health periodically
    sd_config.sd_state = checkSDCardHealth();
    
    return (sd_config.sd_state == SD_READY);
}

// Logs data record to SD card (CSV format)
int logRecordToSD(t_dataRecord* record)
{
    if (!isSDCardReady() || !current_flight_file)
    {
        return 0; // SD not ready or no active flight
    }

    // Convert record to CSV
    char csv_line[512];
    recordToCSV(record, csv_line);
    
    // Write to file
    current_flight_file.print(csv_line);
    current_flight_file.flush(); // Ensure data is written
    
    // Update counters
    sd_config.records_in_flight++;
    sd_config.total_records_sd++;
    
    // Save configuration every 50 records to reduce wear
    if (sd_config.records_in_flight % 50 == 0)
    {
        saveSDConfig();
    }

    return 1;
}

/* *****************************************************************
    *                     STATUS AND INFO FUNCTIONS               *
   ***************************************************************** */

// Gets SD card status and statistics
void getSDStatus(t_sdConfig* config)
{
    // Update usage statistics
    if (isSDCardReady())
    {
        uint64_t total, used;
        if (getSDCardInfo(&total, &used, nullptr))
        {
            sd_config.card_size_mb = total / (1024 * 1024);
            sd_config.used_space_mb = used / (1024 * 1024);
        }
    }
    
    *config = sd_config;
}

// Gets detailed SD card information
int getSDCardInfo(uint64_t* total_size, uint64_t* used_size, uint64_t* free_size)
{
    if (!isSDCardReady())
    {
        return 0;
    }

    *total_size = SD.totalBytes();
    *used_size = SD.usedBytes();
    
    if (free_size)
    {
        *free_size = *total_size - *used_size;
    }

    return 1;
}

// Lists all flights on SD card
uint16_t listSDFlights(t_flightInfo* flights, uint16_t max_flights)
{
    if (!isSDCardReady())
    {
        return 0;
    }

    File index_file = SD.open(SD_INDEX_FILE, FILE_READ);
    if (!index_file)
    {
        return 0;
    }

    uint16_t count = 0;
    while (index_file.available() && count < max_flights)
    {
        String line = index_file.readStringUntil('\n');
        line.trim();
        
        if (line.length() > 0)
        {
            // Parse flight info line: flight_number,start_time,end_time,record_count,filename
            int comma1 = line.indexOf(',');
            int comma2 = line.indexOf(',', comma1 + 1);
            int comma3 = line.indexOf(',', comma2 + 1);
            int comma4 = line.indexOf(',', comma3 + 1);
            
            if (comma1 > 0 && comma2 > comma1 && comma3 > comma2 && comma4 > comma3)
            {
                flights[count].flight_number = line.substring(0, comma1).toInt();
                flights[count].start_timestamp = line.substring(comma1 + 1, comma2).toInt();
                flights[count].end_timestamp = line.substring(comma2 + 1, comma3).toInt();
                flights[count].record_count = line.substring(comma3 + 1, comma4).toInt();
                line.substring(comma4 + 1).toCharArray(flights[count].filename, 32);
                count++;
            }
        }
    }

    index_file.close();
    return count;
}

/* *****************************************************************
    *                   FLIGHT MANAGEMENT FUNCTIONS               *
   ***************************************************************** */

// Downloads flight data from SD card via Bluetooth
int downloadFlightFromSD(uint16_t flight_number)
{
    if (!isSDCardReady())
    {
        Serial.println("SD card not ready for download");
        return 0;
    }

    // Generate flight filename
    char filename[64];
    generateFlightFilename(flight_number, filename);
    
    // Open flight file
    File flight_file = SD.open(filename, FILE_READ);
    if (!flight_file)
    {
        Serial.printf("Flight file %s not found\n", filename);
        return 0;
    }

    Serial.printf("Downloading flight %d (%s)...\n", flight_number, filename);
    
    // Send flight data via Bluetooth
    extern BluetoothSerial SerialBT;
    SerialBT.printf("SD_FLIGHT_DATA_START: %d\n", flight_number);
    
    // Read and transmit file contents
    while (flight_file.available())
    {
        String line = flight_file.readStringUntil('\n');
        SerialBT.print(line);
        if (!line.endsWith("\n"))
        {
            SerialBT.print("\n");
        }
    }
    
    flight_file.close();
    SerialBT.printf("SD_FLIGHT_DATA_END: %d\n", flight_number);
    
    Serial.printf("Flight %d download completed\n", flight_number);
    return 1;
}

// Deletes a specific flight from SD card
int deleteSDFlight(uint16_t flight_number)
{
    if (!isSDCardReady())
    {
        return 0;
    }

    // Generate flight filename
    char filename[64];
    generateFlightFilename(flight_number, filename);
    
    // Delete flight file
    if (!SD.remove(filename))
    {
        Serial.printf("Failed to delete flight file: %s\n", filename);
        return 0;
    }

    Serial.printf("Deleted flight %d: %s\n", flight_number, filename);
    
    // Note: For simplicity, we don't rebuild the index file here
    // A full implementation would remove the flight from the index
    
    return 1;
}

// Formats SD card (emergency function)
int formatSDCard()
{
    if (!isSDCardReady())
    {
        return 0;
    }

    Serial.println("WARNING: Formatting SD card - all data will be lost!");
    
    // Close any open files
    if (current_flight_file)
    {
        current_flight_file.close();
    }

    // Note: SD.format() is not available in all ESP32 SD libraries
    // This is a simplified implementation that removes key directories
    
    // Remove all flight files
    File root = SD.open(SD_DATA_DIR);
    if (root)
    {
        File file = root.openNextFile();
        while (file)
        {
            if (!file.isDirectory())
            {
                SD.remove(file.name());
            }
            file = root.openNextFile();
        }
        root.close();
    }

    // Remove configuration files
    SD.remove(SD_CONFIG_FILE);
    SD.remove(SD_INDEX_FILE);
    
    // Reinitialize configuration
    sd_config.current_flight = 0;
    sd_config.total_flights = 0;
    sd_config.records_in_flight = 0;
    sd_config.total_records_sd = 0;
    
    saveSDConfig();
    
    Serial.println("SD card formatted");
    return 1;
}

// Verifies SD card integrity
int verifySDCard()
{
    if (!isSDCardReady())
    {
        return 0;
    }

    // Basic verification: check if we can read configuration
    t_sdConfig temp_config;
    if (!loadSDConfig())
    {
        Serial.println("SD card verification failed: Cannot read config");
        return 0;
    }

    // Verify that stated flights exist
    uint16_t existing_flights = 0;
    for (uint16_t i = 1; i <= sd_config.total_flights; i++)
    {
        char filename[64];
        generateFlightFilename(i, filename);
        if (SD.exists(filename))
        {
            existing_flights++;
        }
    }

    if (existing_flights != sd_config.total_flights)
    {
        Serial.printf("SD card integrity warning: %d flights expected, %d found\n", 
                      sd_config.total_flights, existing_flights);
    }

    Serial.printf("SD card verification completed: %d/%d flights verified\n", 
                  existing_flights, sd_config.total_flights);
    
    return 1;
}

/* *****************************************************************
    *                    PRIVATE FUNCTIONS                        *
   ***************************************************************** */

// Loads SD configuration from config file
int loadSDConfig()
{
    File config_file = SD.open(SD_CONFIG_FILE, FILE_READ);
    if (!config_file)
    {
        return 0;
    }

    // Read configuration values
    String line;
    while (config_file.available())
    {
        line = config_file.readStringUntil('\n');
        line.trim();
        
        if (line.startsWith("current_flight="))
        {
            sd_config.current_flight = line.substring(15).toInt();
        }
        else if (line.startsWith("total_flights="))
        {
            sd_config.total_flights = line.substring(14).toInt();
        }
        else if (line.startsWith("total_records="))
        {
            sd_config.total_records_sd = line.substring(14).toInt();
        }
    }

    config_file.close();
    return 1;
}

// Saves SD configuration to config file
int saveSDConfig()
{
    if (!isSDCardReady())
    {
        return 0;
    }

    File config_file = SD.open(SD_CONFIG_FILE, FILE_WRITE);
    if (!config_file)
    {
        return 0;
    }

    config_file.printf("current_flight=%d\n", sd_config.current_flight);
    config_file.printf("total_flights=%d\n", sd_config.total_flights);
    config_file.printf("total_records=%d\n", sd_config.total_records_sd);
    config_file.printf("card_size_mb=%llu\n", sd_config.card_size_mb);
    
    config_file.close();
    return 1;
}

// Creates directory structure on SD card
int createSDDirectories()
{
    // Create main AeroSense directory
    if (!SD.exists(SD_DATA_DIR))
    {
        if (!SD.mkdir(SD_DATA_DIR))
        {
            Serial.println("Failed to create AeroSense directory");
            return 0;
        }
    }

    return 1;
}

// Generates flight filename
void generateFlightFilename(uint16_t flight_number, char* filename)
{
    sprintf(filename, "%s%s%04d.csv", SD_DATA_DIR, SD_FLIGHT_PREFIX, flight_number);
}

// Updates flight index file
int updateFlightIndex(t_flightInfo* flight_info)
{
    File index_file = SD.open(SD_INDEX_FILE, FILE_APPEND);
    if (!index_file)
    {
        return 0;
    }

    index_file.printf("%d,%d,%d,%d,%s\n",
                      flight_info->flight_number,
                      flight_info->start_timestamp,
                      flight_info->end_timestamp,
                      flight_info->record_count,
                      flight_info->filename);

    index_file.close();
    return 1;
}

// Converts data record to CSV line
void recordToCSV(t_dataRecord* record, char* csv_line)
{
    sprintf(csv_line, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%.6f,%.6f,%.2f,%d,%d\n",
            record->timestamp,
            record->record_id,
            record->bme680.temp,
            record->bme680.humidity,
            record->bme680.pressure,
            record->bme680.vocIndex,
            record->mhz19b.CO2,
            record->mq4.methane,
            record->mq7.carbonMonoxyde,
            record->mq131.ozone,
            record->mq131.no2,
            record->mq137.nh3,
            record->mq137.co,
            record->pixhawk.data_valid ? record->pixhawk.latitude : 0.0,
            record->pixhawk.data_valid ? record->pixhawk.longitude : 0.0,
            record->pixhawk.data_valid ? record->pixhawk.altitude : 0.0,
            record->pixhawk.data_valid ? record->pixhawk.satellites_visible : 0,
            record->pixhawk.data_valid ? record->pixhawk.fix_type : 0);
}

// Checks SD card health and availability
uint8_t checkSDCardHealth()
{
    // Try to access the card
    if (SD.cardType() == CARD_NONE)
    {
        return SD_CARD_REMOVED;
    }

    // Try to create a test file
    File test_file = SD.open("/test_health.tmp", FILE_WRITE);
    if (!test_file)
    {
        return SD_ERROR;
    }

    test_file.print("test");
    test_file.close();
    
    // Delete test file
    SD.remove("/test_health.tmp");

    return SD_READY;
}
