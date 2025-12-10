/* MQ-4 CALIBRATED - METANO
 * Básico:   PPM = 1000 * (Rs/R0)^-2.949
 * Completo: usa corrección T/HR/P con α=0.017, β=0.004
 */

#include "MQ-4.hpp"
#include <math.h>

// ====================== Parámetros hardware ======================
static float MQ4_R0        = 10000.0f;
static const float MQ4_RL  = 10.0f;
static const float MQ4_VREF = 5.0f;

// ====================== Calibración base ======================
static const float MQ4_a = 1000.0f;
static const float MQ4_b = -2.949f;

// ====================== Compensación ambiental ======================
static const float MQ4_ALPHA = 0.0170f;
static const float MQ4_BETA  = 0.0040f;
static const float MQ_T0     = 20.0f;
static const float MQ_HR0    = 65.0f;
static const float MQ_P0     = 1013.25f;

// ====================== Estructura de calibración ======================
static t_calMQ4 calibration_mq4 = {
    .R0     = 10000.0f,
    .T_cal  = 20.0f,
    .HR_cal = 65.0f,
    .P_cal  = 1013.25f
};

// ====================== Prototipos internos ======================
static float mq4_calculateRs(float rawData, float adcMax);
static float mq4_basicPPM_from_RsR0(float rsR0);
static float mq4_compensatedPPM_from_Rs(float Rs, float T, float HR, float P);

// ====================== Interfaz pública ======================

int initMQ4() {
    pinMode(P_MQ4, INPUT);
    return 1;
}

/* Obtener puntero a datos de calibración */
t_calMQ4* getMQ4_CalibrationData() {
    return &calibration_mq4;
}

/* Calibración en condiciones REALES */
void calibrateMQ4_R0_real(uint16_t cleanAirADCValue, uint16_t adcMax,
                          float temperatureC, float humidityRH, float pressure_hPa) {
    float Rs = mq4_calculateRs((float)cleanAirADCValue, (float)adcMax);
    
    // Guardar calibración con condiciones reales
    calibration_mq4.R0     = Rs / 4.4f;     // ratio aire limpio MQ-4
    calibration_mq4.T_cal  = temperatureC;
    calibration_mq4.HR_cal = humidityRH;
    calibration_mq4.P_cal  = pressure_hPa;
    
    // Actualizar variable estática
    MQ4_R0 = calibration_mq4.R0;
}

/* Lectura básica sin compensación */
void getDataMQ4_basic(t_dataMQ4 *newData, uint16_t adcRaw, uint16_t adcMax) {
    float Rs   = mq4_calculateRs((float)adcRaw, (float)adcMax);
    float rsR0 = Rs / MQ4_R0;

    if (rsR0 > 100.0f) rsR0 = 100.0f;
    if (rsR0 < 0.01f)  rsR0 = 0.01f;

    float ppm = mq4_basicPPM_from_RsR0(rsR0);
    newData->methane = (int32_t)ppm;
}

/* Lectura con compensación T/HR/P */
void getDataMQ4_compensated(t_dataMQ4 *newData,
                            uint16_t adcRaw, uint16_t adcMax,
                            float temperatureC,
                            float humidityRH,
                            float pressure_hPa) {
    float Rs  = mq4_calculateRs((float)adcRaw, (float)adcMax);
    float ppm = mq4_compensatedPPM_from_Rs(Rs, temperatureC, humidityRH, pressure_hPa);
    newData->methane = (int32_t)ppm;
}

// ====================== Implementación interna ======================

/* Cálculo de Rs a partir del ADC */
static float mq4_calculateRs(float rawData, float adcMax) {
    float vOut = (rawData / adcMax) * MQ4_VREF;

    if (vOut >= MQ4_VREF) vOut = MQ4_VREF - 0.001f;
    if (vOut <= 0.0f)      vOut = 0.001f;

    float Rs = MQ4_RL * (MQ4_VREF - vOut) / vOut;
    return Rs;
}

/* Ecuación base */
static float mq4_basicPPM_from_RsR0(float rsR0) {
    float ppm = MQ4_a * powf(rsR0, MQ4_b);

    if (ppm > 5000.0f) ppm = 5000.0f;
    if (ppm <  100.0f) ppm =  100.0f;

    return ppm;
}

/* Ecuación completa con compensación: PPM = a * [(Rs * e^(-α(T-T0)) * (1 + β(HR-HR0)) / R0)^b] * (P0/P) */
static float mq4_compensatedPPM_from_Rs(float Rs, float T, float HR, float P) {
    float fT  = expf(-MQ4_ALPHA * (T  - MQ_T0));
    float fHR = 1.0f + MQ4_BETA * (HR - MQ_HR0);
    float fP  = MQ_P0 / (P > 10.0f ? P : MQ_P0);

    if (fHR < 0.1f) fHR = 0.1f;

    float Rs_equiv = Rs * fT * fHR;
    float rsR0 = Rs_equiv / MQ4_R0;
    
    if (rsR0 > 100.0f) rsR0 = 100.0f;
    if (rsR0 < 0.01f)  rsR0 = 0.01f;

    float ppm = MQ4_a * powf(rsR0, MQ4_b) * fP;

    if (ppm > 5000.0f) ppm = 5000.0f;
    if (ppm <  100.0f) ppm =  100.0f;

    return ppm;
}
