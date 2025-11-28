// Ensure the header is included only once
#ifndef GYUV1_hpp
#define GYUV1_hpp

#include <stdint.h>

#define P_UV 39

typedef struct
{
    int32_t uvIndex;
    int32_t uvRaw;

} t_dataGYUV1;

int initGYUV1();
void getDataGYUV1(t_dataGYUV1 *newData);

#endif // GYUV1_hpp
