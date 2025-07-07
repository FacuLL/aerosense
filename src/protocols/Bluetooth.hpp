/* *****************************************************************
    *                    HEADER CONFIGURATION                     *
   ***************************************************************** */

// We make sure the header is only included once 
#ifndef COMMBLUETOOTH_hpp
#define COMMBLUETOOTH_hpp

/* --------------------- NECESSARY LIBRARIES --------------------- */

// Required for Bluetooth communication
#include "BluetoothSerial.h"

// Provides fixed-width integer types
#include <stdint.h>

/* ---------------- PUBLIC FUNCTIONS PROTOTYPES ---------------- */

// Initializes the Bluetooth communication module
int initCommBT();

// Handles incoming Bluetooth commands to enable or disable measurements
void handleBT(uint8_t *xEnableMeasuring);

// Sends data via Bluetooth
void sendData(String nom, uint16_t data, String unidad, uint8_t CR);

// Sends help message with available commands
void sendHelpMessage();

// Handles SD card commands via Bluetooth
// @param command: The SD card command
void handleSDCommands(String command);

// Sends SD card status via Bluetooth
void sendSDStatus();

#endif // COMMBLUETOOTH_hpp

