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

#ifdef ESP32
#include "esp_bt.h"
#endif

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

#ifdef ESP32
    static bool bleMemoryReleased = false;

    if (!bleMemoryReleased)
    {
        const esp_err_t releaseResult = esp_bt_controller_mem_release(ESP_BT_MODE_BLE);

        if (releaseResult != ESP_OK && releaseResult != ESP_ERR_INVALID_STATE)
        {
            Serial.printf("BLE mem release failed: %d\n", releaseResult);
            return 0;
        }

        bleMemoryReleased = true;
    }
#endif

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

    // Temporary variable to hold incoming data
    uint8_t data;

    /* --------------------- DATA HANDLING ------------------------ */

    // Check if data is available from Bluetooth
    if (!SerialBT.available())
    {
        return;
    }

    // Read incoming data
    data = SerialBT.read();

    // Process data if it is not a line break
    if (data == 10 || data == 13)
    {
        return;
    }

    // Map commands to corresponding actions
    if (data == '1')
    {
        *xEnableMeasuring = 1;
        SerialBT.print("START MEASURING \n");
    }

    else if (data == '0')
    {
        *xEnableMeasuring = 0;
        SerialBT.print("STOP MEASURING \n");
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

    // Format the data with no space between the name and the value
    if (unidad != "")
    {
        sprintf(buffer, "%s%d%s", nom.c_str(), data, unidad.c_str());
    }

    else
    {
        sprintf(buffer, "%s%d", nom.c_str(), data);
    }

    // Emit each reading on its own line for readability
    SerialBT.println(buffer);
    Serial.println(buffer);

    // Optional blank line when CR is set
    if (CR)
    {
        SerialBT.println();
        Serial.println();
    }
}


/* *****************************************************************
    *                    SECTION HEADER FUNCTION                  *
   ***************************************************************** */

// Prints a section divider to Serial and Bluetooth outputs
void sendSectionHeader(const char *sectionName)
{
    /* ------------------- SECTION FORMATTING ------------------- */

    const char divider[] = "------------------------------";

    SerialBT.println();
    SerialBT.println(divider);
    SerialBT.println(sectionName);
    SerialBT.println(divider);

    Serial.println();
    Serial.println(divider);
    Serial.println(sectionName);
    Serial.println(divider);
}

