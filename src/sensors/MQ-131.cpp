/* MQ-131 CALIBRATED - OZONO y DIÓXIDO DE NITRÓGENO
 * O3 Básico:   PPM = 0.0258 * (Rs/R0)^1.972
 * O3 Completo: usa corrección T/HR/P con α=0.015, β=0.003
 * NO2 se estima como 50% de O3
 */

#include "MQ-131.hpp"
#include <math.h>

// ====================== Parámetros hardware ======================
static float MQ131_R0       = 10000.0f;
static const float MQ131_RL = 10.0f;
static const float MQ131_VREF = 5.0f;

// ====================== Calibración base para Ozono ======================
static const float MQ131_a = 0.0258f;
static const float MQ131_b = 1.972f;

// ====================== Compensación ambiental ======================
static const float MQ131_ALPHA = 0.0150f;
static const float MQ131_BETA  = 0.0030f;
static const float MQ_T0       = 20.0f;
static const float MQ_HR0      = 65.0f;
static const float MQ_P0       = 1013.25f;

// ====================== Estructura de calibración ======================
static t_calMQ131 calibration_mq131 = {
    .R0     = 10000.0f,
    .T_cal  = 20.0f,
    .HR_cal = 65.0f,
    .P_cal  = 1013.25f
};

// ====================== Prototipos internos ======================
static float mq131_calculateRs(float rawData, float adcMax);
static float mq131_basicPPM_from_RsR0(float rsR0);
static float mq131_compensatedPPM_from_Rs(float Rs, float T, float HR, float P);

// ====================== Interfaz pública ======================

int initMQ131() {
    pinMode(P_MQ131, INPUT);
    return 1;
}

/* Obtener puntero a datos de calibración */
t_calMQ131* getMQ131_CalibrationData() {
    return &calibration_mq131;
}

/* Calibración en condiciones REALES */
void calibrateMQ131_R0_real(uint16_t cleanAirADCValue, uint16_t adcMax,
                            float temperatureC, float humidityRH, float pressure_hPa) {
    float Rs = mq131_calculateRs((float)cleanAirADCValue, (float)adcMax);
    
    // Guardar calibración con condiciones reales
    calibration_mq131.R0     = Rs / 1.0f;   // ratio aire limpio MQ-131 (1.0 en aire limpio)
    calibration_mq131.T_cal  = temperatureC;
    calibration_mq131.HR_cal = humidityRH;
    calibration_mq131.P_cal  = pressure_hPa;
    
    // Actualizar variable estática
    MQ131_R0 = calibration_mq131.R0;
}

/* Lectura básica sin compensación */
void getDataMQ131_basic(t_dataMQ131 *newData, uint16_t adcRaw, uint16_t adcMax) {
    float Rs   = mq131_calculateRs((float)adcRaw, (float)adcMax);
    float rsR0 = Rs / MQ131_R0;

    if (rsR0 > 100.0f) rsR0 = 100.0f;
    if (rsR0 < 0.01f)  rsR0 = 0.01f;

    float ppm_o3 = mq131_basicPPM_from_RsR0(rsR0);
    
    newData->ozone = (int32_t)(ppm_o3 * 1000.0f);     // convertir a ppb
    newData->no2   = (int32_t)(ppm_o3 * 1000.0f * 0.5f); // estimación NO2
}

/* Lectura con compensación T/HR/P */
void getDataMQ131_compensated(t_dataMQ131 *newData,
                              uint16_t adcRaw, uint16_t adcMax,
                              float temperatureC,
                              float humidityRH,
                              float pressure_hPa) {
    float Rs     = mq131_calculateRs((float)adcRaw, (float)adcMax);
    float ppm_o3 = mq131_compensatedPPM_from_Rs(Rs, temperatureC, humidityRH, pressure_hPa);
    
    newData->ozone = (int32_t)(ppm_o3 * 1000.0f);     // convertir a ppb
    newData->no2   = (int32_t)(ppm_o3 * 1000.0f * 0.5f); // estimación NO2
}

// ====================== Implementación interna ======================

/* Cálculo de Rs a partir del ADC */
static float mq131_calculateRs(float rawData, float adcMax) {
    float vOut = (rawData / adcMax) * MQ131_VREF;

    if (vOut >= MQ131_VREF) vOut = MQ131_VREF - 0.001f;
    if (vOut <= 0.0f)        vOut = 0.001f;

    float Rs = MQ131_RL * (MQ131_VREF - vOut) / vOut;
    return Rs;
}

/* Ecuación base */
static float mq131_basicPPM_from_RsR0(float rsR0) {
    float ppm = MQ131_a * powf(rsR0, MQ131_b);

    if (ppm > 1.0f)   ppm = 1.0f;
    if (ppm < 0.001f) ppm = 0.001f;

    return ppm;
}

/* Ecuación completa con compensación: PPM = a * [(Rs * e^(-α(T-T0)) * (1 + β(HR-HR0)) / R0)^b] * (P0/P) */
static float mq131_compensatedPPM_from_Rs(float Rs, float T, float HR, float P) {
    float fT  = expf(-MQ131_ALPHA * (T  - MQ_T0));
    float fHR = 1.0f + MQ131_BETA * (HR - MQ_HR0);
    float fP  = MQ_P0 / (P > 10.0f ? P : MQ_P0);

    if (fHR < 0.1f) fHR = 0.1f;

    float Rs_equiv = Rs * fT * fHR;
    float rsR0 = Rs_equiv / MQ131_R0;
    
    if (rsR0 > 100.0f) rsR0 = 100.0f;
    if (rsR0 < 0.01f)  rsR0 = 0.01f;

    float ppm = MQ131_a * powf(rsR0, MQ131_b) * fP;

    if (ppm > 1.0f)   ppm = 1.0f;
    if (ppm < 0.001f) ppm = 0.001f;

    return ppm;
}
