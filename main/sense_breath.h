#ifndef FILE_sense_breath_SEEN
#define FILE_sense_breath_SEEN

#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "bmp180.h"

#include "kit.h"

#define ATMOS_OFFSET 36000
// #define PRESSURE_MULTIPLIER .5f
#define PRESSURE_MULTIPLIER 1.5f

void initSenseBreath(SemaphoreHandle_t commSemaphore);
bmp180_dev_t* getBmp180Device(void);
void breathRecalibrate(uint16_t duration);
void measureBreath(void);
bool getBreathPressure(
    int* out_pressure, callbackVoidVoid_t onCalibrateOk
);
#ifndef DEBUG_NO_BREATH
    I am using syntax error to denote undefined macro here!
#endif
#if DEBUG_NO_BREATH == 1
    void set_debug_no_breath_presure(int value);
#endif

#endif
