#include "GY-UV1.hpp"

#include <Arduino.h>

static bool pinsConfigured = false;

static int averageAnalogRead(int pinToRead)
{
    const uint8_t samples = 8;
    uint32_t accumulator = 0;

    for (uint8_t i = 0; i < samples; ++i)
    {
        accumulator += analogRead(pinToRead);
        delayMicroseconds(200);
    }

    return static_cast<int>(accumulator / samples);
}

static float mapFloat(float x, float inMin, float inMax, float outMin, float outMax)
{
    return (x - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
}

int initGYUV1()
{
    pinMode(P_GYUV1_UVOUT, INPUT);
    pinMode(P_GYUV1_REF, INPUT);
    pinsConfigured = true;
    return 1;
}

void getDataGYUV1(t_dataGYUV1 *newData)
{
    if (!newData)
    {
        return;
    }

    if (!pinsConfigured && !initGYUV1())
    {
        newData->uvRaw = 0;
        newData->refRaw = 0;
        newData->uvIntensity = 0;
        return;
    }

    const int uvLevel = averageAnalogRead(P_GYUV1_UVOUT);
    const int refLevel = averageAnalogRead(P_GYUV1_REF);

    newData->uvRaw = uvLevel;
    newData->refRaw = refLevel;

    if (refLevel <= 0)
    {
        newData->uvIntensity = 0;
        return;
    }

    const float outputVoltage = (3.3f * static_cast<float>(uvLevel)) /
                                 static_cast<float>(refLevel);

    float uvIntensity = mapFloat(outputVoltage, 0.99f, 2.9f, 0.0f, 15.0f);

    if (uvIntensity < 0.0f)
    {
        uvIntensity = 0.0f;
    }
    else if (uvIntensity > 15.0f)
    {
        uvIntensity = 15.0f;
    }

    newData->uvIntensity = static_cast<int32_t>(uvIntensity + 0.5f);
}
