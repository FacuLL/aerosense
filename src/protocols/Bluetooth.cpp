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

// Include SD card logger for logging commands
#include "../storage/SDLogger.hpp"

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

    // Indicate successful startup with helpful message
    SerialBT.print("AeroSense READY - Send 'HELP' for commands\n");

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
    
    // Main measurement commands
    if (command == "START" || command == "START_MEASURING" || command == "BEGIN")
    {
        *xEnableMeasuring = 1;
        uint16_t flight_num = startSDFlightSession();
        if (flight_num > 0)
        {
            SerialBT.printf("MEASURING STARTED - SD Flight %d\n", flight_num);
        }
        else
        {
            SerialBT.print("MEASURING STARTED - SD LOGGING FAILED\n");
        }
    }
    else if (command == "STOP" || command == "STOP_MEASURING" || command == "END")
    {
        *xEnableMeasuring = 0;
        endSDFlightSession();
        SerialBT.print("MEASURING STOPPED - SD FLIGHT ENDED\n");
    }
    // Legacy single character commands (for backward compatibility)
    else if (command == "1")
    {
        *xEnableMeasuring = 1;
        uint16_t flight_num = startSDFlightSession();
        if (flight_num > 0)
        {
            SerialBT.printf("START MEASURING & SD LOGGING - Flight %d\n", flight_num);
        }
        else
        {
            SerialBT.print("START MEASURING - SD LOGGING FAILED\n");
        }
    }
    else if (command == "0")
    {
        *xEnableMeasuring = 0;
        endSDFlightSession();
        SerialBT.print("STOP MEASURING & SD LOGGING\n");
    }
    // SD logging commands (simplified from flash commands)
    else if (command == "LOG_START" || command == "START_FLIGHT" || command == "NEW_FLIGHT")
    {
        uint16_t flight_num = startSDFlightSession();
        if (flight_num > 0)
        {
            SerialBT.printf("SD FLIGHT STARTED: %d\n", flight_num);
        }
        else
        {
            SerialBT.print("SD FLIGHT START FAILED\n");
        }
    }
    else if (command == "LOG_STOP" || command == "END_FLIGHT" || command == "CLOSE_FLIGHT")
    {
        if (endSDFlightSession())
        {
            SerialBT.print("SD FLIGHT STOPPED\n");
        }
        else
        {
            SerialBT.print("SD FLIGHT STOP FAILED\n");
        }
    }
    else if (command == "LOG_STATUS" || command == "STATUS" || command == "INFO")
    {
        sendSDStatus();
    }
    else if (command == "LIST_FLIGHTS" || command == "FLIGHTS")
    {
        handleSDCommands("SD_LIST_FLIGHTS");
    }
    else if (command.startsWith("DOWNLOAD_FLIGHT:"))
    {
        String sd_command = "SD_" + command;
        handleSDCommands(sd_command);
    }
    else if (command.startsWith("SD_"))
    {
        handleSDCommands(command);
    }
    else if (command == "HELP" || command == "COMMANDS" || command == "?")
    {
        sendHelpMessage();
    }
    else if (command == "CLEAR_DATA" || command == "FORMAT_SD")
    {
        if (formatSDCard())
        {
            SerialBT.print("SD CARD FORMATTED\n");
        }
        else
        {
            SerialBT.print("SD FORMAT FAILED\n");
        }
    }
    else if (command == "STORAGE_INFO" || command == "SD_INFO")
    {
        uint64_t total, used, free;
        if (getSDCardInfo(&total, &used, &free))
        {
            SerialBT.printf("SD STORAGE: Total=%lluMB Used=%lluMB Free=%lluMB\n",
                           total/(1024*1024), used/(1024*1024), free/(1024*1024));
        }
        else
        {
            SerialBT.print("SD CARD NOT AVAILABLE\n");
        }
    }
    else
    {
        // Unknown command
        SerialBT.printf("UNKNOWN COMMAND: %s\n", command.c_str());
        SerialBT.print("Send 'HELP' for available commands\n");
    }
}


/* *****************************************************************
    *                     SD CARD FUNCTIONS                       *
   ***************************************************************** */

// Process complete Bluetooth commands (declaration for internal use)
void processBluetoothCommand(String command, uint8_t *xEnableMeasuring);

// Sends help message with available commands
void sendHelpMessage()
{
    SerialBT.print("\n=== AeroSense Commands ===\n");
    SerialBT.print("MEASUREMENT:\n");
    SerialBT.print("  START / BEGIN        - Start measuring and logging\n");
    SerialBT.print("  STOP / END           - Stop measuring and logging\n");
    SerialBT.print("  STATUS               - Show system status\n");
    SerialBT.print("\nFLIGHT MANAGEMENT:\n");
    SerialBT.print("  START_FLIGHT         - Start new flight session\n");
    SerialBT.print("  END_FLIGHT           - End current flight session\n");
    SerialBT.print("  LIST_FLIGHTS         - List all recorded flights\n");
    SerialBT.print("\nDATA RETRIEVAL:\n");
    SerialBT.print("  SD_DOWNLOAD_FLIGHT:N - Download flight N data\n");
    SerialBT.print("  SD_LIST_FLIGHTS      - Detailed flight list\n");
    SerialBT.print("  STORAGE_INFO         - SD card storage info\n");
    SerialBT.print("\nMAINTENANCE:\n");
    SerialBT.print("  SD_VERIFY            - Verify SD card integrity\n");
    SerialBT.print("  SD_DELETE_FLIGHT:N   - Delete flight N\n");
    SerialBT.print("  CLEAR_DATA           - Format SD card (WARNING!)\n");
    SerialBT.print("\nLEGACY:\n");
    SerialBT.print("  1 / 0                - Start/Stop (backward compatibility)\n");
    SerialBT.print("  HELP / ?             - Show this message\n");
    SerialBT.print("========================\n\n");
}

// Handles SD card commands via Bluetooth
void handleSDCommands(String command)
{
    if (command == "SD_STATUS")
    {
        sendSDStatus();
    }
    else if (command == "SD_START_FLIGHT")
    {
        uint16_t flight_num = startSDFlightSession();
        if (flight_num > 0)
        {
            SerialBT.printf("SD_FLIGHT_STARTED: %d\n", flight_num);
        }
        else
        {
            SerialBT.print("SD_FLIGHT_START_FAILED\n");
        }
    }
    else if (command == "SD_END_FLIGHT")
    {
        if (endSDFlightSession())
        {
            SerialBT.print("SD_FLIGHT_ENDED\n");
        }
        else
        {
            SerialBT.print("SD_FLIGHT_END_FAILED\n");
        }
    }
    else if (command == "SD_LIST_FLIGHTS")
    {
        t_flightInfo flights[10]; // List up to 10 flights
        uint16_t count = listSDFlights(flights, 10);
        
        SerialBT.printf("SD_FLIGHTS: %d total\n", count);
        for (uint16_t i = 0; i < count; i++)
        {
            SerialBT.printf("FLIGHT:%d,%d,%d,%d,%s\n",
                           flights[i].flight_number,
                           flights[i].start_timestamp,
                           flights[i].end_timestamp,
                           flights[i].record_count,
                           flights[i].filename);
        }
    }
    else if (command.startsWith("SD_DOWNLOAD_FLIGHT:"))
    {
        // Extract flight number
        int colonIndex = command.indexOf(':');
        if (colonIndex != -1)
        {
            uint16_t flight_number = command.substring(colonIndex + 1).toInt();
            if (downloadFlightFromSD(flight_number))
            {
                SerialBT.printf("SD_DOWNLOAD_COMPLETE: Flight %d\n", flight_number);
            }
            else
            {
                SerialBT.printf("SD_DOWNLOAD_FAILED: Flight %d\n", flight_number);
            }
        }
    }
    else if (command == "SD_INFO")
    {
        uint64_t total, used, free;
        if (getSDCardInfo(&total, &used, &free))
        {
            SerialBT.printf("SD_INFO: Total=%lluMB Used=%lluMB Free=%lluMB\n",
                           total/(1024*1024), used/(1024*1024), free/(1024*1024));
        }
        else
        {
            SerialBT.print("SD_INFO: Card not available\n");
        }
    }
    else if (command == "SD_VERIFY")
    {
        if (verifySDCard())
        {
            SerialBT.print("SD_VERIFY_OK\n");
        }
        else
        {
            SerialBT.print("SD_VERIFY_FAILED\n");
        }
    }
    else if (command.startsWith("SD_DELETE_FLIGHT:"))
    {
        // Extract flight number
        int colonIndex = command.indexOf(':');
        if (colonIndex != -1)
        {
            uint16_t flight_number = command.substring(colonIndex + 1).toInt();
            if (deleteSDFlight(flight_number))
            {
                SerialBT.printf("SD_DELETE_OK: Flight %d\n", flight_number);
            }
            else
            {
                SerialBT.printf("SD_DELETE_FAILED: Flight %d\n", flight_number);
            }
        }
    }
}

// Sends SD card status via Bluetooth
void sendSDStatus()
{
    t_sdConfig config;
    getSDStatus(&config);
    
    SerialBT.printf("SD_STATUS: State=%d Flights=%d Records=%d Size=%lluMB Used=%lluMB\n",
                    config.sd_state, config.total_flights, config.total_records_sd,
                    config.card_size_mb, config.used_space_mb);
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

