/* *****************************************************************
    *                        INFORMATION                          *
   ***************************************************************** 

    This file serves as the main program for the AeroSense system.
    It initializes sensors, handles Bluetooth communication, and
    performs periodic data measurement and transmission.

*/


/* *****************************************************************
    *                     FILE CONFIGURATION                      *
   ***************************************************************** */

/* ---------------------- NECESSARY HEADERS ---------------------- */

// Includes the BME680 sensor functions
#include "sensors/BME680.hpp"

// Includes the MH-Z19B sensor functions
#include "sensors/MH-Z19B.hpp"

// Includes the MQ-4 sensor functions
#include "sensors/MQ-4.hpp"

// Includes the MQ-7 sensor functions
#include "sensors/MQ-7.hpp"

// Includes the MQ-131 sensor functions
#include "sensors/MQ-131.hpp"

// Includes Bluetooth communication functions
#include "protocols/Bluetooth.hpp"

/* ---------------------- GLOBAL VARIABLES ---------------------- */

// Stores data from BME680 sensor
t_dataBME680 dataBME680;

// Stores data from MH-Z19B sensor
t_dataMHZ19B dataMHZ19B;

// Stores data from MQ-4 sensor
t_dataMQ4 dataMQ4;

// Stores data from MQ-7 sensor
t_dataMQ7 dataMQ7;

// Stores data from MQ-131 sensor
t_dataMQ131 dataMQ131;

// Flag to enable or disable measurement
uint8_t xEnableMeasuring = 0;

// Stores the previous timestamp for periodic measurement
long preMillis;

// Measurement interval in milliseconds
#define PERIODE_MESURE 10000


/* *****************************************************************
    *                        SETUP FUNCTION                       *
   ***************************************************************** */

// Initializes the system, including sensors and Bluetooth communication
void setup()
{
    /* --------------------- INITIALIZATION --------------------- */

    // Start the serial communication at 115200 baud
    Serial.begin(115200);

    // Initialize all sensors
    initSensors();

    // Initialize Bluetooth communication
    if (!initCommBT())
    {
        Serial.println("Failed Init BT");
    }

    else
    {
        Serial.println("Init BT Done");
    }
}


/* *****************************************************************
    *                         LOOP FUNCTION                       *
   ***************************************************************** */

// Main loop that handles Bluetooth commands and performs periodic measurement
void loop()
{
    /* -------------------- HANDLE BLUETOOTH -------------------- */

    // Handle incoming Bluetooth commands
    handleBT(&xEnableMeasuring);

    /* ------------------ PERIODIC MEASUREMENT ------------------ */

    // Perform measurement if the interval has elapsed
    if (millis() - preMillis >= PERIODE_MESURE)
    {
        preMillis = millis();

        if (xEnableMeasuring)
        {
            readAllSensors();
        }
    }
}


/* *****************************************************************
    *                       READ ALL SENSORS                     *
   ***************************************************************** */

// Reads data from all sensors and sends it via Bluetooth
void readAllSensors()
{
    /* ------------------- SENSOR MEASUREMENTS ------------------- */

    // Read and send data from BME680 sensor
    getDataBME680(&dataBME680);
    sendData("Temp:", dataBME680.temp, "°", 0);
    sendData("Humidity:", dataBME680.humidity, "%", 0);
    sendData("Pressure:", dataBME680.pressure, "hPa", 0);
    sendData("VOC Index:", dataBME680.vocIndex, "", 0);

    // Read and send data from MH-Z19B sensor
    getDataMHZ19B(&dataMHZ19B);
    sendData("CO2:", dataMHZ19B.CO2, "ppm", 0);

    // Read and send data from MQ-4 sensor
    getDataMQ4(&dataMQ4);
    sendData("CH4:", dataMQ4.methane, "ppm", 0);

    // Read and send data from MQ-7 sensor
    getDataMQ7(&dataMQ7);
    sendData("CO:", dataMQ7.carbonMonoxyde, "ppm", 0);

    // Read and send data from MQ-131 sensor
    getDataMQ131(&dataMQ131);
    sendData("O3:", dataMQ131.ozone, "ppm", 0);
    sendData("NO2:", dataMQ131.no2, "ppm", 1);
}


/* *****************************************************************
    *                   INITIALIZE ALL SENSORS                   *
   ***************************************************************** */

// Initializes all connected sensors
void initSensors()
{
    /* ------------------ INITIALIZE BME680 ------------------ */

    // Initialize BME680 sensor
    Serial.println("Start Init BME680...");
    if (!initBME680())
    {
        Serial.println("Failed Init BME680");
    }
    
    else
    {
        Serial.println("Init BME680 OK !");
    }

    /* ------------------ INITIALIZE MH-Z19B ------------------ */

    // Initialize MH-Z19B sensor
    Serial.println("Start Init MHZ19B...");
    if (!initMHZ19B())
    {
        Serial.println("Failed Init MHZ19B");
    }

    else
    {
        Serial.println("Init MHZ19B OK !");
    }

    /* ------------------ INITIALIZE MQ-4 ------------------ */

    // Initialize MQ-4 sensor
    Serial.println("Start Init MQ-4...");
    if (!initMQ4())
    {
        Serial.println("Failed Init MQ-4");
    }
    
    else
    {
        Serial.println("Init MQ-4 OK !");
    }

    /* ------------------ INITIALIZE MQ-7 ------------------ */

    // Initialize MQ-7 sensor
    Serial.println("Start Init MQ-7...");
    if (!initMQ7())
    {
        Serial.println("Failed Init MQ-7");
    }
    
    else
    {
        Serial.println("Init MQ-7 OK !");
    }

    /* ------------------ INITIALIZE MQ-131 ------------------ */

    // Initialize MQ-131 sensor
    Serial.println("Start Init MQ-131...");
    if (!initMQ131())
    {
        Serial.println("Failed Init MQ-131");
    }
    
    else
    {
        Serial.println("Init MQ-131 OK !");
    }
}
