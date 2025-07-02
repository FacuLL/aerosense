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

#endif // COMMBLUETOOTH_hpp

