// Handles initialization and data retrieval for the PMS5003 sensor.

#include "PMS5003.hpp"
#include <Arduino.h>

static HardwareSerial pmsSerial(PMS5003_SERIAL_INDEX);
static bool serialReady = false;

static bool readFrame(t_dataPMS5003 *out);

int initPMS5003()
{
    if (serialReady)
    {
        return 1;
    }

    pmsSerial.begin(PMS5003_BAUD, SERIAL_8N1, P_PMS5003_RX, P_PMS5003_TX);
    pmsSerial.setTimeout(1000);
    serialReady = true;

    // Give the sensor a moment after power-up before the first read.
    delay(1000);
    return 1;
}

void getDataPMS5003(t_dataPMS5003 *newData)
{
    if (!newData)
    {
        return;
    }

    if (!serialReady && !initPMS5003())
    {
        newData->pm1_0 = 0;
        newData->pm2_5 = 0;
        newData->pm10 = 0;
        return;
    }

    if (!readFrame(newData))
    {
        newData->pm1_0 = 0;
        newData->pm2_5 = 0;
        newData->pm10 = 0;
    }
}

static bool readFrame(t_dataPMS5003 *out)
{
    if (!serialReady)
    {
        return false;
    }

    while (pmsSerial.available() >= 32)
    {
        uint8_t frame[32];
        int startByte = pmsSerial.read();

        if (startByte != 0x42)
        {
            continue;
        }

        frame[0] = static_cast<uint8_t>(startByte);
        size_t readBytes = pmsSerial.readBytes(&frame[1], 31);
        if (readBytes != 31)
        {
            continue;
        }

        if (frame[1] != 0x4D)
        {
            continue;
        }

        uint16_t sum = 0;
        for (uint8_t i = 0; i < 30; ++i)
        {
            sum += frame[i];
        }

        uint16_t checksum = (static_cast<uint16_t>(frame[30]) << 8) | frame[31];
        if (sum != checksum)
        {
            continue;
        }

        out->pm1_0 = (static_cast<uint16_t>(frame[10]) << 8) | frame[11];
        out->pm2_5 = (static_cast<uint16_t>(frame[12]) << 8) | frame[13];
        out->pm10 = (static_cast<uint16_t>(frame[14]) << 8) | frame[15];
        return true;
    }

    return false;
}
