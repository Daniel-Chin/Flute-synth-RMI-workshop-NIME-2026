#include "role.h"
#ifndef ROLE
    I am using syntax error to denote undefined macro here!
#endif
#if ROLE == 'F'

#include "esp_log.h"
#include "esp_timer.h"

#include "shared.h"
#include "env.h"
#include "sense_breath.h"
#include "bmp180.h"
#include "print_and_send_log.h"

static int offset_pressure = 0;
static int atmos_pressure = -1;
static int measure_atmosphere_state = 20;
static long measure_atmosphere_tmp = 0;
static int measure_atmosphere_times = 0;
#ifndef DEBUG_NO_BREATH
    I am using syntax error to denote undefined macro here!
#endif
#if DEBUG_NO_BREATH == 1
  static int debug_no_breath_presure = 0;
#endif
static bmp180_dev_t device;

void initSenseBreath(SemaphoreHandle_t commSemaphore) {
    ESP_LOGI(PROJECT_TAG, "initSenseBreath()...");
    #ifndef DEBUG_NO_BREATH
        I am using syntax error to denote undefined macro here!
    #endif
    #if DEBUG_NO_BREATH == 0
        memset(&device, 0, sizeof(bmp180_dev_t)); // Zero descriptor

        ESP_ERROR_CHECK(bmp180_init_desc(&device, 0, I2C_PIN_SDA, I2C_PIN_SCL));
        device.i2c_dev.cfg.sda_pullup_en = 1;
        device.i2c_dev.cfg.scl_pullup_en = 1;
        ESP_ERROR_CHECK(bmp180_init(&device));
    #endif
    ESP_LOGI(PROJECT_TAG, "initSenseBreath() ok");
}

void breathRecalibrate(uint16_t duration) {
    measure_atmosphere_tmp = 0;
    measure_atmosphere_times = 0;
    measure_atmosphere_state = (int) duration;
}

// #ifndef DEBUG_NO_BREATH
//     I am using syntax error to denote undefined macro here!
// #endif
// #if DEBUG_NO_BREATH == 0
// static inline int getMedian(int a, int b, int c) {
//     if (b < a) {
//         swap(&a, &b);
//     }
//     if (c < b) {
//         swap(&b, &c);
//     }
//     if (b < a) {
//         swap(&a, &b);
//     }
//     return b;
// }

// static int medianFilter(int c) {
//     static bool ready = false;
//     static int a = 0;
//     static int b = 0;
//     if (! ready) {
//         ready = true;
//         a = c;
//         b = c;
//     }
//     int m = getMedian(a, b, c);
//     a = b;
//     b = c;
//     return m;
// }
// #endif

bmp180_dev_t* getBmp180Device(void) {
    return &device;
}

void measureBreath(void) {
    #ifndef DEBUG_NO_BREATH
        I am using syntax error to denote undefined macro here!
    #endif
    #if DEBUG_NO_BREATH == 0
        uint32_t raw_pressure;
        float _;
        esp_err_t res = bmp180_measure(&device, &_, &raw_pressure, BMP180_MODE_STANDARD);
        if (res != ESP_OK) {
            ESP_LOGE(PROJECT_TAG, "BMP180 read failed, err = %d", res);
            return;
        }
        offset_pressure = ((int) raw_pressure) - ATMOS_OFFSET;
    #endif
}

bool getBreathPressure(
    int* out_pressure, callbackVoidVoid_t onCalibrateOk
) {
    #ifndef DEBUG_NO_BREATH
        I am using syntax error to denote undefined macro here!
    #endif
    #if DEBUG_NO_BREATH == 1
        if (debug_no_breath_presure > 0) {
            debug_no_breath_presure -= 1;
        }
        *out_pressure = debug_no_breath_presure;
        return true;
    #endif
    if (measure_atmosphere_state >= 0) {
        measure_atmosphere_tmp += offset_pressure;
        measure_atmosphere_times ++;
        if (measure_atmosphere_state == 0) {
            atmos_pressure = measure_atmosphere_tmp / measure_atmosphere_times;
            if (onCalibrateOk != NULL) {
                onCalibrateOk();
            }
            ESP_LOGI(PROJECT_TAG, "atmos_pressure recalibrated: %d", atmos_pressure);
        }
        measure_atmosphere_state --;
        return false;
    }
    int p = offset_pressure - atmos_pressure;
    p = (int) (PRESSURE_MULTIPLIER * (float) p);
    // ESP_LOGI(PROJECT_TAG, "breath pressure = %d", p);
    p = MAX(0, p);
    // ESP_LOGI(PROJECT_TAG, "abs breath pressure = %d", p);
    *out_pressure = p;
    return true;
}

#ifndef DEBUG_NO_BREATH
    I am using syntax error to denote undefined macro here!
#endif
#if DEBUG_NO_BREATH == 1
    void set_debug_no_breath_presure(int value) {
        debug_no_breath_presure = value;
    }
#endif

#endif
