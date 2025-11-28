#include "GY-UV1.hpp"

#include <Arduino.h>

static bool pinsConfigured = false;

int initGYUV1()
{

    pinsConfigured = true;
    return 1;
}

void getDataGYUV1(t_dataGYUV1 *newData)
{
    if (!newData) return;

    // Auto-inicialización si se olvidó llamar a init
    if (!pinsConfigured) {
        initGYUV1();
    }

    uint16_t uvRaw = analogRead(P_UV);
    newData->uvRaw = (int32_t)uvRaw;
}
