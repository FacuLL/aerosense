/* *****************************************************************
    *                        INFORMATION                          *
   ***************************************************************** 

    This file handles Bluetooth communication for the AeroSense system.
    It initializes the Bluetooth module, manages commands, and sends data.

*/


/* *****************************************************************
    *                     FILE CONFIGURATION                      *
   ***************************************************************** */

/* ---------------------- NECESSARY HEADERS ---------------------- */

// Required for Bluetooth communication
#include "Bluetooth.hpp"

// Include data logger for logging commands
#include "../storage/DataLogger.hpp"

/* ---------------------- GLOBAL VARIABLES ---------------------- */

// Bluetooth serial object for communication
BluetoothSerial SerialBT;


/* *****************************************************************
    *                      INIT COMMUNICATION                     *
   ***************************************************************** */

// Initializes the Bluetooth communication module
// Returns: 1 if initialization is successful, 0 otherwise
int initCommBT()
{
    /* -------------------- INITIALIZATION -------------------- */

    // Start Bluetooth communication with the name "AeroSense"
    if (!SerialBT.begin("AeroSense"))
    {
        return 0;
    }

    // Indicate successful startup
    SerialBT.print("STARTED");

    return 1;
}


/* *****************************************************************
    *                    HANDLE BLUETOOTH DATA                    *
   ***************************************************************** */

// Handles incoming Bluetooth commands to enable or disable measurements
// Parameters:
// - xEnableMeasuring: Pointer to a variable that controls measurement state
void handleBT(uint8_t *xEnableMeasuring)
{
    /* -------------------- LOCAL VARIABLES -------------------- */

    // String to hold incoming command
    static String command = "";
    
    /* --------------------- DATA HANDLING ------------------------ */

    // Check if data is available from Bluetooth
    if (!SerialBT.available())
    {
        return;
    }

    // Read incoming data
    while (SerialBT.available())
    {
        char inChar = (char)SerialBT.read();
        
        // Check for command terminator
        if (inChar == '\n' || inChar == '\r')
        {
            if (command.length() > 0)
            {
                // Process the complete command
                processBluetoothCommand(command, xEnableMeasuring);
                command = ""; // Clear command buffer
            }
        }
        else
        {
            command += inChar; // Build command string
        }
    }
}

// Process complete Bluetooth commands
void processBluetoothCommand(String command, uint8_t *xEnableMeasuring)
{
    command.trim(); // Remove whitespace
    command.toUpperCase(); // Convert to uppercase for consistency
    
    // Legacy single character commands
    if (command == "1")
    {
        *xEnableMeasuring = 1;
        startLogging();
        SerialBT.print("START MEASURING & LOGGING\n");
    }
    else if (command == "0")
    {
        *xEnableMeasuring = 0;
        stopLogging();
        SerialBT.print("STOP MEASURING & LOGGING\n");
    }
    // Data logging commands
    else if (command == "LOG_START")
    {
        if (startLogging())
        {
            SerialBT.print("LOGGING STARTED\n");
        }
        else
        {
            SerialBT.print("LOGGING START FAILED\n");
        }
    }
    else if (command == "LOG_STOP")
    {
        if (stopLogging())
        {
            SerialBT.print("LOGGING STOPPED\n");
        }
        else
        {
            SerialBT.print("LOGGING STOP FAILED\n");
        }
    }
    else if (command == "LOG_STATUS")
    {
        sendLoggingStatus();
    }
    else if (command.startsWith("DOWNLOAD"))
    {
        handleDownloadCommands(command);
    }
    else if (command == "LOG_CLEAR")
    {
        if (clearStoredData())
        {
            SerialBT.print("DATA CLEARED\n");
        }
        else
        {
            SerialBT.print("CLEAR FAILED\n");
        }
    }
    else if (command == "STORAGE_INFO")
    {
        uint32_t used, total;
        if (getStorageStats(&used, &total))
        {
            SerialBT.printf("STORAGE: %d/%d bytes used\n", used, total);
        }
    }
}


/* *****************************************************************
    *                   DATA LOGGING FUNCTIONS                    *
   ***************************************************************** */

// Process complete Bluetooth commands (declaration for internal use)
void processBluetoothCommand(String command, uint8_t *xEnableMeasuring);

// Sends logging status via Bluetooth
void sendLoggingStatus()
{
    t_loggerConfig config;
    getLoggingStatus(&config);
    
    SerialBT.printf("LOG_STATUS: State=%d Records=%d Index=%d Total=%d\n",
                    config.logging_state, config.total_records, 
                    config.current_index, config.total_logged);
}

// Handles data download commands
void handleDownloadCommands(String command)
{
    if (command == "DOWNLOAD_PREPARE")
    {
        uint16_t total_records = prepareDownload();
        SerialBT.printf("DOWNLOAD_READY: %d records\n", total_records);
    }
    else if (command.startsWith("DOWNLOAD_RECORD:"))
    {
        // Extract record index
        int colonIndex = command.indexOf(':');
        if (colonIndex != -1)
        {
            uint16_t record_index = command.substring(colonIndex + 1).toInt();
            
            t_dataRecord record;
            if (downloadRecord(record_index, &record))
            {
                // Send record data in CSV format for easy parsing
                SerialBT.printf("RECORD:%d,%d,%d,", record_index, record.timestamp, record.record_id);
                
                // BME680 data
                SerialBT.printf("%d,%d,%d,%d,", record.bme680.temp, record.bme680.humidity, 
                               record.bme680.pressure, record.bme680.vocIndex);
                
                // Gas sensor data
                SerialBT.printf("%d,%d,%d,%d,%d,%d,%d,", record.mhz19b.CO2, record.mq4.methane,
                               record.mq7.carbonMonoxyde, record.mq131.ozone, record.mq131.no2,
                               record.mq137.nh3, record.mq137.co);
                
                // GPS data
                if (record.pixhawk.data_valid)
                {
                    SerialBT.printf("%.6f,%.6f,%.2f,%d,%d\n", 
                                   record.pixhawk.latitude, record.pixhawk.longitude,
                                   record.pixhawk.altitude, record.pixhawk.satellites_visible,
                                   record.pixhawk.fix_type);
                }
                else
                {
                    SerialBT.printf("0,0,0,0,0\n");
                }
            }
            else
            {
                SerialBT.printf("DOWNLOAD_ERROR: Record %d not found\n", record_index);
            }
        }
    }
    else if (command == "DOWNLOAD_STATUS")
    {
        t_downloadStatus status;
        getDownloadStatus(&status);
        SerialBT.printf("DOWNLOAD_STATUS: %d/%d downloaded\n", 
                        status.downloaded, status.total_records);
    }
}


/* *****************************************************************
    *                      SEND DATA FUNCTION                     *
   ***************************************************************** */

// Sends data via Bluetooth
// Parameters:
// - nom: Name of the data
// - data: Data value
// - unidad: Unit of the data
// - CR: Flag to indicate whether to add a newline (1) or separator (0)
void sendData(String nom, uint16_t data, String unidad, uint8_t CR)
{
    /* ------------------- DATA TRANSMISSION ------------------- */

    // Create the buffer for formatting
    char buffer[60];

    // Format the data with no space between the name and the value, and fixed spacing after each reading
    if (unidad != "")
    {
        // Format with unit attached to the value directly
        sprintf(buffer, "%s%d%s", nom.c_str(), data, unidad.c_str());
    }
    
    else
    {
        // Format without unit
        sprintf(buffer, "%s%d", nom.c_str(), data);
    }

    // Send the formatted data
    SerialBT.print(buffer);

    // Add a few spaces to separate this data from the next one
    SerialBT.print("    ");

    // Add a newline if CR is set
    if (CR)
    {
        SerialBT.print("\n");
    }
}

