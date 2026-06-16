// This is a bad way to do this. Instead, a unit test should stay with its unit.  

#include <math.h>
#include <string.h>

#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/i2s_std.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "bmp180.h"

#include "kit.h"
#include "shared.h"
#include "env.h"
#include "tests.h"
#include "wave_cube_synth.h"
#include "timbre.h"
#include "music.h"
#include "servo.h"
#include "role.h"
#ifndef ROLE
    I am using syntax error to denote undefined macro here!
#endif
#if ROLE == 'F'
    #include "sense_breath.h"
#endif

#ifndef ROLE
    I am using syntax error to denote undefined macro here!
#endif
#if ROLE == 'F'
void testWaveCubeSynth(void) {
    bool use_vowel = false;
    while (1) {
        use_vowel = ! use_vowel;
        int pitch = 72;
        while (pitch <= 96) {
            int timbre_i = timbreOfPitch(pitch);
            if (! use_vowel) {
                timbre_i = 0;
            }
            for (float a = 0.0f; a < 1.0f; a += .02f) {
                updateWaveRow(pitch2freq((float)pitch + a * .3f), a, timbre_i);
                delayTaskMs(10);
            }
            pitch += 2;
            if (
                pitch % 12 == 6 ||
                pitch % 12 == 1
            ) {
                pitch --;
            }
        }
    }
}

void bmp180Task(void) {
    while (1) {
        uint32_t pressure;
        float temperature;
        esp_err_t err = bmp180_measure(getBmp180Device(), &temperature, &pressure, BMP180_MODE_STANDARD);
        if (err != ESP_OK) {
            ESP_LOGE(PROJECT_TAG, "Reading from BMP180 failed, err = %d", err);
        }
        ESP_LOGI(
            PROJECT_TAG, 
            "Pressure %d Pa, Temperature : %.1f oC", 
            ((int) pressure), (double) temperature
        );
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void testBMP180(void) {
    ESP_LOGI(PROJECT_TAG, "testBMP180()...");
    bmp180Task();
}
#endif

#ifndef ROLE
    I am using syntax error to denote undefined macro here!
#endif
#if ROLE == 'G'
void testServo(void) {
    while (1) {
        for (int i = 0; i < N_SERVOS; i++) {
            moveServo(i, 1);
        }
        for (int i = 0; i < N_SERVOS; i++) {
            moveServo(i, 179);
            delayTaskMs(333);
        }
        for (int angle = 179; angle >= 1; angle -= 2) {
            for (int i = 0; i < N_SERVOS; i++)
            {
                moveServo(i, angle);
            }
            delayTaskMs(10);
        }
    }
}
#endif

typedef enum {
    ABC = 4,
    DEF = 5,
} Hi;

void testIntCastEnum(void) {
    uint8_t tt = 5;
    Hi h = (Hi) (int) tt;
    switch (h) {
        case ABC:
            ESP_LOGI(PROJECT_TAG, "no");
            break;
        case DEF:
            ESP_LOGI(PROJECT_TAG, "yes");
            break;
        default:
            ESP_LOGI(PROJECT_TAG, "default");
            break;
    }
}

void testI2S(void) {
    // GPIO5=BCK, GPIO16=LRCK/WS, GPIO17=DIN on PCM5102A
    static int16_t buf[256 * 2];  // 256 frames, stereo
    static const int SAMPLE_RATE     = 44100;
    static const int BUF_FRAMES      = 256;
    static const int DURATION_FRAMES = 44100;   // 1 sec per tone
    static const float TONE_HZ       = 440.0f;
    static const float AMP           = 0.7f;
    static const float TWO_PI        = 6.28318530717959f;
    static const float PI            = 3.14159265358979f;
    static const char * const names[5] = {
        "sine", "square", "sawtooth", "triangle", "chirp 200-1000Hz"
    };

    ESP_LOGI(PROJECT_TAG, "testI2S: init");
    i2s_chan_handle_t tx;
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &tx, NULL));
    i2s_std_config_t std_cfg = {
        .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(44100),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(
            I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO
        ),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = GPIO_NUM_5,
            .ws   = GPIO_NUM_16,
            .dout = GPIO_NUM_17,
            .din  = I2S_GPIO_UNUSED,
        },
    };
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(tx, &std_cfg));
    ESP_ERROR_CHECK(i2s_channel_enable(tx));

    for (int wt = 0; wt < 5; wt++) {
        ESP_LOGI(PROJECT_TAG, "testI2S: %s", names[wt]);
        float phase = 0.0f;
        int buf_pos  = 0;
        for (int fi = 0; fi < DURATION_FRAMES; fi++) {
            float freq = (wt == 4)
                ? 200.0f + 800.0f * fi / (float)DURATION_FRAMES
                : TONE_HZ;
            float s;
            switch (wt) {
                case 0:  s = sinf(phase); break;                            // sine
                case 1:  s = (phase < PI) ? 1.0f : -1.0f; break;           // square
                case 2:  s = phase / PI - 1.0f; break;                     // sawtooth
                case 3:  s = (phase < PI)                                   // triangle
                             ? phase / PI * 2.0f - 1.0f
                             : 3.0f - phase / PI * 2.0f;
                         break;
                default: s = sinf(phase); break;                            // chirp
            }
            int16_t val = (int16_t)(s * AMP * 32767.0f);
            buf[buf_pos * 2    ] = val;
            buf[buf_pos * 2 + 1] = val;
            phase += TWO_PI * freq / (float)SAMPLE_RATE;
            if (phase >= TWO_PI) phase -= TWO_PI;
            if (++buf_pos == BUF_FRAMES) {
                size_t w;
                i2s_channel_write(tx, buf, sizeof(buf), &w, portMAX_DELAY);
                buf_pos = 0;
            }
        }
        if (buf_pos > 0) {
            memset(buf + buf_pos * 2, 0, (BUF_FRAMES - buf_pos) * 4);
            size_t w;
            i2s_channel_write(tx, buf, sizeof(buf), &w, portMAX_DELAY);
        }
    }

    ESP_ERROR_CHECK(i2s_channel_disable(tx));
    ESP_ERROR_CHECK(i2s_del_channel(tx));
    ESP_LOGI(PROJECT_TAG, "testI2S: done");
}
