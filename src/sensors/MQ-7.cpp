/* MQ-7 CALIBRATED - MONÓXIDO DE CARBONO
 * Básico:   PPM = 117.49 * (Rs/R0)^-1.761
 * Completo: usa corrección T/HR/P con α=0.0074, β=0.005
 */

#include "MQ-7.hpp"
#include <math.h>

// ====================== Parámetros hardware ======================
static float MQ7_R0        = 10000.0f;
static const float MQ7_RL  = 10.0f;
static const float MQ7_VREF = 5.0f;

// ====================== Calibración base ======================
static const float MQ7_a = 117.49f;
static const float MQ7_b = -1.761f;

// ====================== Compensación ambiental ======================
static const float MQ7_ALPHA = 0.0074f;
static const float MQ7_BETA  = 0.0050f;
static const float MQ_T0     = 20.0f;
static const float MQ_HR0    = 65.0f;
static const float MQ_P0     = 1013.25f;

// ====================== Estructura de calibración ======================
static t_calMQ7 calibration_mq7 = {
    .R0     = 10000.0f,
    .T_cal  = 20.0f,
    .HR_cal = 65.0f,
    .P_cal  = 1013.25f
};

// ====================== Prototipos internos ======================
static float mq7_calculateRs(float rawData, float adcMax);
static float mq7_basicPPM_from_RsR0(float rsR0);
static float mq7_compensatedPPM_from_Rs(float Rs, float T, float HR, float P);

// ====================== Interfaz pública ======================

int initMQ7() {
    pinMode(P_MQ7, INPUT);
    return 1;
}

/* Obtener puntero a datos de calibración */
t_calMQ7* getMQ7_CalibrationData() {
    return &calibration_mq7;
}

/* Calibración en condiciones REALES */
void calibrateMQ7_R0_real(uint16_t cleanAirADCValue, uint16_t adcMax,
                          float temperatureC, float humidityRH, float pressure_hPa) {
    float Rs = mq7_calculateRs((float)cleanAirADCValue, (float)adcMax);
    
    // Guardar calibración con condiciones reales
    calibration_mq7.R0     = Rs / 9.83f;   // ratio aire limpio MQ-7
    calibration_mq7.T_cal  = temperatureC;
    calibration_mq7.HR_cal = humidityRH;
    calibration_mq7.P_cal  = pressure_hPa;
    
    // Actualizar variable estática
    MQ7_R0 = calibration_mq7.R0;
}

/* Lectura básica sin compensación */
void getDataMQ7_basic(t_dataMQ7 *newData, uint16_t adcRaw, uint16_t adcMax) {
    float Rs   = mq7_calculateRs((float)adcRaw, (float)adcMax);
    float rsR0 = Rs / MQ7_R0;

    if (rsR0 > 100.0f) rsR0 = 100.0f;
    if (rsR0 < 0.01f)  rsR0 = 0.01f;

    float ppm = mq7_basicPPM_from_RsR0(rsR0);
    newData->carbonMonoxyde = (int32_t)ppm;
}

/* Lectura con compensación T/HR/P */
void getDataMQ7_compensated(t_dataMQ7 *newData,
                            uint16_t adcRaw, uint16_t adcMax,
                            float temperatureC,
                            float humidityRH,
                            float pressure_hPa) {
    float Rs  = mq7_calculateRs((float)adcRaw, (float)adcMax);
    float ppm = mq7_compensatedPPM_from_Rs(Rs, temperatureC, humidityRH, pressure_hPa);
    newData->carbonMonoxyde = (int32_t)ppm;
}

// ====================== Implementación interna ======================

/* Cálculo de Rs a partir del ADC */
static float mq7_calculateRs(float rawData, float adcMax) {
    float vOut = (rawData / adcMax) * MQ7_VREF;

    if (vOut >= MQ7_VREF) vOut = MQ7_VREF - 0.001f;
    if (vOut <= 0.0f)      vOut = 0.001f;

    float Rs = MQ7_RL * (MQ7_VREF - vOut) / vOut;
    return Rs;
}

/* Ecuación base */
static float mq7_basicPPM_from_RsR0(float rsR0) {
    float ppm = MQ7_a * powf(rsR0, MQ7_b);

    if (ppm > 2000.0f) ppm = 2000.0f;
    if (ppm <   20.0f) ppm =   20.0f;

    return ppm;
}

/* Ecuación completa con compensación: PPM = a * [(Rs * e^(-α(T-T0)) * (1 + β(HR-HR0)) / R0)^b] * (P0/P) */
static float mq7_compensatedPPM_from_Rs(float Rs, float T, float HR, float P) {
    float fT  = expf(-MQ7_ALPHA * (T  - MQ_T0));
    float fHR = 1.0f + MQ7_BETA * (HR - MQ_HR0);
    float fP  = MQ_P0 / (P > 10.0f ? P : MQ_P0);

    if (fHR < 0.1f) fHR = 0.1f;

    float Rs_equiv = Rs * fT * fHR;
    float rsR0 = Rs_equiv / MQ7_R0;
    
    if (rsR0 > 100.0f) rsR0 = 100.0f;
    if (rsR0 < 0.01f)  rsR0 = 0.01f;

    float ppm = MQ7_a * powf(rsR0, MQ7_b) * fP;

    if (ppm > 2000.0f) ppm = 2000.0f;
    if (ppm <   20.0f) ppm =   20.0f;

    return ppm;
}
