/* *****************************************************************
 * AEROSENSE MAIN v3 - With R0 Calibration in Real Conditions
 * 
 * Features:
 * - Stores R0 calibration with T, HR, P conditions
 * - Uses compensated formulas with environmental corrections
 * - CSV output via Bluetooth
 * - Measurements every 10 seconds
 *************************************************************** */

/* --------------------- NECESSARY HEADERS -------------------- */

#include "sensors/BME680.hpp"
#include "sensors/MQ-4.hpp"
#include "sensors/MQ-7.hpp"
#include "sensors/MQ-131.hpp"
#include "sensors/GY-UV1.hpp"
#include "sensors/PMS5003.hpp"
#include "protocols/Bluetooth.hpp"
#include <Preferences.h>

/* ---------------------- GLOBAL VARIABLES ---------------------- */

// Sensor data structures
t_dataBME680  dataBME680;
t_dataMQ4     dataMQ4;
t_dataMQ7     dataMQ7;
t_dataMQ131   dataMQ131;
t_dataGYUV1   dataGYUV1;
t_dataPMS5003 dataPMS5003;

// Preferences (NVS flash) for calibration storage
Preferences nvs;

// Flag to enable/disable measurement
uint8_t xEnableMeasuring = 0;

// Stores the previous timestamp for periodic measurement
long preMillis = 0;

// Measurement interval in milliseconds (10 seconds)
#define PERIODE_MESURE 10000

// Constants for ADC conversion (ESP32 12-bit)
#define ADC_MAX 4095

/* ====================== NVS FUNCTIONS ====================== */

/* Initialize NVS storage namespace */
void initNVS() {
    if (!nvs.begin("AeroSense", false)) {
        printBT("ERROR: NVS init failed");
    } else {
        printBT("NVS OK");
    }
}

/* Store calibration data in NVS */
void storeCalibrationData(const char* key_r0, const char* key_t, 
                         const char* key_hr, const char* key_p,
                         float r0, float t, float hr, float p) {
    nvs.putFloat(key_r0, r0);
    nvs.putFloat(key_t, t);
    nvs.putFloat(key_hr, hr);
    nvs.putFloat(key_p, p);
}

/* Load calibration data from NVS */
void loadCalibrationData(const char* key_r0, const char* key_t, 
                        const char* key_hr, const char* key_p,
                        float &r0, float &t, float &hr, float &p) {
    r0 = nvs.getFloat(key_r0, 10000.0);
    t  = nvs.getFloat(key_t, 20.0);
    hr = nvs.getFloat(key_hr, 65.0);
    p  = nvs.getFloat(key_p, 1013.25);
}

/* Check if calibration exists and run if needed */
void checkAndCalibrateR0() {
    bool has_r0_mq7   = nvs.isKey("R0_MQ7");
    bool has_r0_mq4   = nvs.isKey("R0_MQ4");
    bool has_r0_mq131 = nvs.isKey("R0_MQ131");
    
    if (has_r0_mq7 && has_r0_mq4 && has_r0_mq131) {
        // All calibrations exist - load and display
        float r0_mq7, t_mq7, hr_mq7, p_mq7;
        float r0_mq4, t_mq4, hr_mq4, p_mq4;
        float r0_mq131, t_mq131, hr_mq131, p_mq131;
        
        loadCalibrationData("R0_MQ7", "T_MQ7", "HR_MQ7", "P_MQ7", 
                           r0_mq7, t_mq7, hr_mq7, p_mq7);
        loadCalibrationData("R0_MQ4", "T_MQ4", "HR_MQ4", "P_MQ4", 
                           r0_mq4, t_mq4, hr_mq4, p_mq4);
        loadCalibrationData("R0_MQ131", "T_MQ131", "HR_MQ131", "P_MQ131", 
                           r0_mq131, t_mq131, hr_mq131, p_mq131);

        printBT("\n=== R0 VALUES LOADED ===");
        {
            char _buf[128];
            snprintf(_buf, sizeof(_buf),
                "MQ7: R0=%.3f T=%.2f HR=%.2f P=%.2f",
                r0_mq7, t_mq7, hr_mq7, p_mq7);
            printBT(_buf);
        }

        {
            char _buf[128];
            snprintf(_buf, sizeof(_buf),
                "MQ4: R0=%.3f T=%.2f HR=%.2f P=%.2f",
                r0_mq4, t_mq4, hr_mq4, p_mq4);
            printBT(_buf);
        }

        {
            char _buf[128];
            snprintf(_buf, sizeof(_buf),
                "MQ131: R0=%.3f T=%.2f HR=%.2f P=%.2f",
                r0_mq131, t_mq131, hr_mq131, p_mq131);
            printBT(_buf);
        }
        printBT("========================\n");
        
    } else {
        // Need calibration
        printBT("\n=== CALIBRATION REQUIRED ===");
        printBT("Place sensors in CLEAN AIR for 60 seconds...");

        // Wait 60 seconds
        delay(60000);
        
        // Read clean air values multiple times for averaging
        printBT("Reading clean air ADC values...");
        
        uint16_t adc_mq7 = 0, adc_mq4 = 0, adc_mq131 = 0;
        for (int i = 0; i < 10; i++) {
            adc_mq7   += analogRead(P_MQ7);
            adc_mq4   += analogRead(P_MQ4);
            adc_mq131 += analogRead(P_MQ131);
            delay(100);
        }
        adc_mq7   /= 10;
        adc_mq4   /= 10;
        adc_mq131 /= 10;
        
        // Read environmental conditions DURING calibration
        getDataBME680(&dataBME680);
        float t_cal  = dataBME680.temp;
        float hr_cal = dataBME680.humidity;
        float p_cal  = dataBME680.pressure;

        // Calibrate sensors with REAL conditions
        calibrateMQ7_R0_real(adc_mq7, ADC_MAX, t_cal, hr_cal, p_cal);
        calibrateMQ4_R0_real(adc_mq4, ADC_MAX, t_cal, hr_cal, p_cal);
        calibrateMQ131_R0_real(adc_mq131, ADC_MAX, t_cal, hr_cal, p_cal);
        
        // Get R0 values from sensors
        t_calMQ7* cal7 = getMQ7_CalibrationData();
        t_calMQ4* cal4 = getMQ4_CalibrationData();
        t_calMQ131* cal131 = getMQ131_CalibrationData();
        
        // Store in NVS
        storeCalibrationData("R0_MQ7", "T_MQ7", "HR_MQ7", "P_MQ7",
                            cal7->R0, cal7->T_cal, cal7->HR_cal, cal7->P_cal);
        storeCalibrationData("R0_MQ4", "T_MQ4", "HR_MQ4", "P_MQ4",
                            cal4->R0, cal4->T_cal, cal4->HR_cal, cal4->P_cal);
        storeCalibrationData("R0_MQ131", "T_MQ131", "HR_MQ131", "P_MQ131",
                            cal131->R0, cal131->T_cal, cal131->HR_cal, cal131->P_cal);
        
        printBT("\n=== CALIBRATION COMPLETE ===");
        printBT((String("MQ7 R0=") + String(cal7->R0)).c_str());
        printBT((String("MQ4 R0=") + String(cal4->R0)).c_str());
        printBT((String("MQ131 R0=") + String(cal131->R0)).c_str());
        printBT("=============================\n");
    }
}

/* Send CSV data via Bluetooth */
void sendCSVData() {
    char buffer[200];
    snprintf(buffer, sizeof(buffer),
        "%lu,%.2f,%.2f,%.2f,%u,%u,%u,%u,%.2f,%u,%u,%u",
        millis(),
        dataBME680.temp,
        dataBME680.humidity,
        dataBME680.pressure,
        dataMQ7.carbonMonoxyde,
        dataMQ4.methane,
        dataMQ131.ozone,
        dataMQ131.no2,
        dataGYUV1.uvIndex,
        dataPMS5003.pm1_0,
        dataPMS5003.pm2_5,
        dataPMS5003.pm10
    );
    
    printBT(buffer);
}

/* ====================== SETUP FUNCTION ====================== */

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n=== AeroSense v3 - Initialization ===\n");
    
    // Initialize NVS
    initNVS();
    
    // Initialize all sensors
    initSensors();
    
    // Initialize Bluetooth
    if (!initCommBT()) {
        Serial.println("ERROR: BT init failed");
    }
    
    delay(2000);  // Give BT time to start
    
    // Check calibration
    checkAndCalibrateR0();
    
    // Send CSV header
    printBT("Timestamp_ms,Temp_C,Humidity_RH,Pressure_hPa,MQ7_CO_ppm,MQ4_CH4_ppm,MQ131_O3_ppm,MQ131_NO2_ppm,GY_UV_mW_cm2,PM1_0_ug_m3,PM2_5_ug_m3,PM10_ug_m3");
}

/* ====================== MAIN LOOP ====================== */

void loop() {
    handleBT(&xEnableMeasuring);
    
    if (millis() - preMillis >= PERIODE_MESURE) {
        preMillis = millis();
        
        if (xEnableMeasuring) {
            readAllSensorsAndSend();
        }
    }
}

/* ====================== SENSOR READING ====================== */

void readAllSensorsAndSend() {
    // Read environmental data
    getDataBME680(&dataBME680);
    
    // Read ADC values
    uint16_t adc_mq7   = analogRead(P_MQ7);
    uint16_t adc_mq4   = analogRead(P_MQ4);
    uint16_t adc_mq131 = analogRead(P_MQ131);
    
    // Read sensors with compensation
    getDataMQ7_compensated(&dataMQ7, adc_mq7, ADC_MAX,
                          dataBME680.temp, dataBME680.humidity, dataBME680.pressure);
    
    getDataMQ4_compensated(&dataMQ4, adc_mq4, ADC_MAX,
                          dataBME680.temp, dataBME680.humidity, dataBME680.pressure);
    
    getDataMQ131_compensated(&dataMQ131, adc_mq131, ADC_MAX,
                            dataBME680.temp, dataBME680.humidity, dataBME680.pressure);
    
    // Read other sensors
    getDataGYUV1(&dataGYUV1);
    getDataPMS5003(&dataPMS5003);
    
    // Send CSV
    sendCSVData();
}

/* ====================== INITIALIZE SENSORS ====================== */

void initSensors() {
    Serial.println("Initializing sensors...");
    
    Serial.print("BME680... ");
    if (!initBME680()) {
        Serial.println("FAILED");
    } else {
        Serial.println("OK");
    }
    
    Serial.print("MQ-4... ");
    if (!initMQ4()) {
        Serial.println("FAILED");
    } else {
        Serial.println("OK");
    }
    
    Serial.print("MQ-7... ");
    if (!initMQ7()) {
        Serial.println("FAILED");
    } else {
        Serial.println("OK");
    }
    
    Serial.print("MQ-131... ");
    if (!initMQ131()) {
        Serial.println("FAILED");
    } else {
        Serial.println("OK");
    }
    
    Serial.print("GY-UV1... ");
    if (!initGYUV1()) {
        Serial.println("FAILED");
    } else {
        Serial.println("OK");
    }
    
    Serial.print("PMS5003... ");
    if (!initPMS5003()) {
        Serial.println("FAILED");
    } else {
        Serial.println("OK");
    }
    
    Serial.println("All sensors initialized.\n");
}
