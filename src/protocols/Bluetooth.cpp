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

