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

// Includes the MQ-137 sensor functions
#include "sensors/MQ-137.hpp"

// Includes the GY-UV1 sensor functions
#include "sensors/GY-UV1.hpp"

// Includes the PMS5003 sensor functions
#include "sensors/PMS5003.hpp"

// Includes the Pixhawk interface functions
#include "sensors/Pixhawk.hpp"

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

// Stores data from MQ-137 sensor
t_dataMQ137 dataMQ137;

// Stores data from GY-UV1 sensor
t_dataGYUV1 dataGYUV1;

// Stores data from PMS5003 sensor
t_dataPMS5003 dataPMS5003;

// Stores data from Pixhawk
t_dataPixhawk dataPixhawk;

// Flag to enable or disable measurement
uint8_t xEnableMeasuring = 0;

// Stores the previous timestamp for periodic measurement
long preMillis;

// Measurement interval in milliseconds
#define PERIODE_MESURE 2000


/* *****************************************************************
    *                        SETUP FUNCTION                       *
   ***************************************************************** */

// Initializes the system, including sensors and Bluetooth communication
void setup()
{
    /* --------------------- INITIALIZATION --------------------- */
    Serial.println("Initialization");
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
            Serial.println("Measuring...");
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

    /* ====================== BME680 SENSOR ====================== */
    // sendSectionHeader("BME680 SENSOR");
    // // Acquire and forward BME680 environmental metrics
    // getDataBME680(&dataBME680);
    // sendData("Temp:", dataBME680.temp, "°", 0);
    // sendData("Humidity:", dataBME680.humidity, "%", 0);
    // sendData("Pressure:", dataBME680.pressure, "hPa", 0);
    // sendData("VOC Index:", dataBME680.vocIndex, "", 1);

    /* ====================== MH-Z19B SENSOR ===================== */
    // Uncomment when MH-Z19B data is ready to be reported
    // getDataMHZ19B(&dataMHZ19B);
    // sendData("CO2:", dataMHZ19B.CO2, "ppm", 0);

    /* ======================= MQ-4 SENSOR ======================= */
    sendSectionHeader("MQ-4 SENSOR");
    // Capture methane concentration from MQ-4
    getDataMQ4(&dataMQ4);
    sendData("CH4:", dataMQ4.methane, "ppm", 1);

    /* ======================= MQ-7 SENSOR ======================= */
    sendSectionHeader("MQ-7 SENSOR");
    // Capture carbon monoxide concentration from MQ-7
    getDataMQ7(&dataMQ7);
    sendData("CO:", dataMQ7.carbonMonoxyde, "ppm", 1);

    /* ====================== MQ-131 SENSOR ====================== */
    sendSectionHeader("MQ-131 SENSOR");
    // Capture ozone and NO2 levels from MQ-131
    getDataMQ131(&dataMQ131);
    sendData("O3:", dataMQ131.ozone, "ppm", 0);
    sendData("NO2:", dataMQ131.no2, "ppm", 1);

    /* ====================== MQ-137 SENSOR ====================== */
    sendSectionHeader("MQ-137 SENSOR");
    // Capture ammonia and CO readings from MQ-137
    getDataMQ137(&dataMQ137);
    sendData("NH3:", dataMQ137.nh3, "ppm", 0);
    sendData("CO_MQ137:", dataMQ137.co, "ppm", 1);

    /* ======================= GY-UV1 SENSOR ===================== */
    sendSectionHeader("GY-UV1 SENSOR");
    // Capture UV intensity from GY-UV1
    getDataGYUV1(&dataGYUV1);
    sendData("UV:", dataGYUV1.uvIntensity, "mW/cm2", 1);

    /* ====================== PMS5003 SENSOR ===================== */
    sendSectionHeader("PMS5003 SENSOR");
    // Capture particulate matter concentrations from PMS5003
    getDataPMS5003(&dataPMS5003);
    sendData("PM1.0:", dataPMS5003.pm1_0, "ug/m3", 0);
    sendData("PM2.5:", dataPMS5003.pm2_5, "ug/m3", 0);
    sendData("PM10:", dataPMS5003.pm10, "ug/m3", 1);

    /* ====================== PIXHAWK STATUS ===================== */
    // sendSectionHeader("PIXHAWK STATUS");
    // Read and send data from Pixhawk autopilot
    // getDataPixhawk(&dataPixhawk);
    // if (dataPixhawk.data_valid)
    // {
    //     sendData("LAT:", (int32_t)(dataPixhawk.latitude * 1000000), "µdeg", 0);
    //     sendData("LON:", (int32_t)(dataPixhawk.longitude * 1000000), "µdeg", 0);
    //     sendData("ALT:", (int32_t)(dataPixhawk.altitude * 100), "cm", 0);
    //     sendData("SAT:", dataPixhawk.satellites_visible, "", 0);
    //     sendData("FIX:", dataPixhawk.fix_type, "", 1);
    // }
    // else
    // {
        // sendData("GPS:", 0, "NO_FIX", 1);
    // }

    sendSectionHeader("END OF MEASUREMENT");
}


/* *****************************************************************
    *                   INITIALIZE ALL SENSORS                   *
   ***************************************************************** */

// Initializes all connected sensors
void initSensors()
{
    /* ------------------ INITIALIZE BME680 ------------------ */

    // Initialize BME680 sensor
    // Serial.println("Start Init BME680...");
    // if (!initBME680())
    // {
    //     Serial.println("Failed Init BME680");
    // }
    
    // else
    // {
    //     Serial.println("Init BME680 OK !");
    // }

    /* ------------------ INITIALIZE MH-Z19B ------------------ */

    // Initialize MH-Z19B sensor
    // Serial.println("Start Init MHZ19B...");
    // if (!initMHZ19B())
    // {
    //     Serial.println("Failed Init MHZ19B");
    // }

    // else
    // {
    //     Serial.println("Init MHZ19B OK !");
    // }

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

    /* ------------------ INITIALIZE MQ-137 ------------------ */

    // Initialize MQ-137 sensor
    Serial.println("Start Init MQ-137...");
    if (!initMQ137())
    {
        Serial.println("Failed Init MQ-137");
    }
    
    else
    {
        Serial.println("Init MQ-137 OK !");
    }

    /* ------------------ INITIALIZE PMS5003 ------------------ */

    // Initialize PMS5003 sensor
    Serial.println("Start Init PMS5003...");
    if (!initPMS5003())
    {
        Serial.println("Failed Init PMS5003");
    }

    else
    {
        Serial.println("Init PMS5003 OK !");
    }

    /* ------------------ INITIALIZE GY-UV1 ------------------ */

    // Initialize GY-UV1 sensor
    Serial.println("Start Init GY-UV1...");
    if (!initGYUV1())
    {
        Serial.println("Failed Init GY-UV1");
    }

    else
    {
        Serial.println("Init GY-UV1 OK !");
    }

    /* ------------------ INITIALIZE PIXHAWK ------------------ */

    // Initialize Pixhawk communication
    // Serial.println("Start Init Pixhawk...");
    // if (!initPixhawk())
    // {
    //     Serial.println("Failed Init Pixhawk");
    // }
    
    // else
    // {
    //     Serial.println("Init Pixhawk OK !");
    // }
}
