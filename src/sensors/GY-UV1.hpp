// Ensure the header is included only once
#ifndef GYUV1_hpp
#define GYUV1_hpp

#include <stdint.h>

#ifndef P_GYUV1_UVOUT
#define P_GYUV1_UVOUT 36
#endif

#ifndef P_GYUV1_REF
#define P_GYUV1_REF 39
#endif

#ifndef P_GYUV1_ALS
#define P_GYUV1_ALS 25
#endif

typedef struct
{
    int32_t uvRaw;
    int32_t refRaw;
    int32_t uvIntensity;

} t_dataGYUV1;

int initGYUV1();
void getDataGYUV1(t_dataGYUV1 *newData);

#endif // GYUV1_hpp
