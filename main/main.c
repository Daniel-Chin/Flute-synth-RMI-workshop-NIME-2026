/*
todo
    upgrade wave_cube_synth to use I2S or continuous DAC output.
in prduction, change
    uncheck CONFIG_LWIP_CHECK_THREAD_SAFETY
    CONFIG_COMPILER_OPTIMIZATION
    CONFIG_COMPILER_STACK_CHECK_MODE
*/

#include <math.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "i2cdev.h"

#include "kit.h"
#include "shared.h"
#include "diagnose.h"
#include "role.h"
#include "env.h"
#include "tests.h"
#include "wifi.h"
#include "comm.h"
#include "handshake.h"
#include "recv_packet.h"
#include "send_packet.h"
#include "time_keep.h"
#include "priorities.h"
#include "startup_barrier.h"
#include "measure_battery.h"
#ifndef ROLE
    I am using syntax error to denote undefined macro here!
#endif
#if ROLE == 'F'
    #include "sense_breath.h"
    #include "wave_cube_synth.h"
    #include "touch_sensor.h"
    #include "note_queue.h"
    #include "electric_flute.h"
    #include "bootup_jingle.h"
    #include "touch_sensor.h"
#endif
#ifndef ROLE
    I am using syntax error to denote undefined macro here!
#endif
#if ROLE == 'G'
    #include "servo.h"
    #include "haptic.h"
#endif

static SemaphoreHandle_t local_features_ready;
static bool startup_barrier_passed = false;

#ifndef ROLE
    I am using syntax error to denote undefined macro here!
#endif
#if ROLE == 'F'
    static void process_fingers(void* _);
    static void process_pressure(void* _);
    static void relayResidualPressureService(void* _);
    #endif
static void battery_service_burst(void* _);
static void battery_service_chill(void* _);
static TaskHandle_t battery_service_chill_task = NULL;
static TaskHandle_t battery_service_burst_task = NULL;

static void initCommChannel(void* _) {
    initComm(onRecv);
    handshake();

    vTaskDelete(NULL);
}

static void initLocalFeatures(SemaphoreHandle_t handshakeSemaphore) {
    init_measure_battery();
    assert_pdPASS(xTaskCreate(
        (TaskFunction_t) battery_service_chill, "batterySvcChill", 
        3800, NULL, PRIORITY_BATTERY_SERVICE_CHILL, &battery_service_chill_task
    ));
    assert_pdPASS(xTaskCreate(
        (TaskFunction_t) battery_service_burst, "batterySvcBurst", 
        768, NULL, PRIORITY_BATTERY_SERVICE_BURST, &battery_service_burst_task
    ));

    #ifndef ROLE
        I am using syntax error to denote undefined macro here!
    #endif
    #if ROLE == 'F'
        SemaphoreHandle_t jingleOk = xSemaphoreCreateBinary();
        #ifndef DO_SYNTH
            I am using syntax error to denote undefined macro here!
        #endif
        #if DO_SYNTH == 1
            initWaveCubeSynth();
            assert_pdPASS(xTaskCreate(
                (TaskFunction_t) bootupJingle, "bootupJingle", 
                4000, jingleOk, PRIORITY_JINGLE, NULL
            ));
        #endif
        ESP_ERROR_CHECK(i2cdev_init());
        initTouch();
        initNoteQueues();
        initSenseBreath(handshakeSemaphore);
        initElectricFlute();
        while (1) {
            if (xSemaphoreTake(jingleOk, 1) == pdTRUE)
                break;
            int _;
            // calibrate atmos
            measureBreath();
            getBreathPressure(&_, NULL);
        }
        assert_pdPASS(xTaskCreate(
            (TaskFunction_t) process_fingers, "fingersSvc", 
            2300, NULL, PRIORITY_FINGERS, NULL
        ));
        assert_pdPASS(xTaskCreate(
            (TaskFunction_t) process_pressure, "pressureSvc", 
            4100, NULL, PRIORITY_PRESSURE, NULL
        ));
    #endif
    #ifndef ROLE
        I am using syntax error to denote undefined macro here!
    #endif
    #if ROLE == 'G'
        initServos();
        initHaptic();
    #endif

    (void) xSemaphoreGive(local_features_ready);
    vTaskDelete(NULL);
}

void app_main(void) {
    // start_task_list_dumper();
    initStartupBarrier();
    local_features_ready = xSemaphoreCreateBinary();
    if (local_features_ready == NULL) {
        ESP_LOGE(PROJECT_TAG, "xSemaphoreCreateBinary failed");
        abort();
    }

    #ifndef MUSX_DEBUG
        I am using syntax error to denote undefined macro here!
    #endif
    #if MUSX_DEBUG == 1
        // delayTaskMs((int) (ROLE == 'F' ? 4.0e3 : 8.0e3)); 
        // for serial to connect and restart ESP32
        // otherwise, proc may get duplicate handshakes when you plug ESP32 in. 
        ESP_LOGI(PROJECT_TAG, "Starting init.");
    #endif
    
    assert_pdPASS(xTaskCreate(
        (TaskFunction_t) initCommChannel, "initCommChann", 
        1024 * 20, NULL, PRIORITY_INIT_COMM_CHANNEL, NULL
    ));
    assert_pdPASS(xTaskCreate(
        (TaskFunction_t) initLocalFeatures, "initLocalFs", 
        1024 * 20, startupBarrierGetSema(), PRIORITY_INIT_LOCAL_FEATURES, NULL
    ));

    while (xSemaphoreTake(local_features_ready, 1000 / portTICK_PERIOD_MS) != pdTRUE) {
        ESP_LOGI(PROJECT_TAG, "Waiting for local init...");
    }

    {// tests block
        // testBMP180();
        // testServo();
        // testWaveCubeSynth();
    }

    while (xSemaphoreTake(
        startupBarrierGetSema(), 1000 / portTICK_PERIOD_MS
    ) != pdTRUE) {
        ESP_LOGI(PROJECT_TAG, "Waiting for startup barrier...");
    }
    (void) xSemaphoreGive(startupBarrierGetSema());
    startup_barrier_passed = true;

    xTaskNotifyGiveIndexed(
        battery_service_burst_task, TASK_NOTIFICATION_INDEX
    );

    initTimeKeep();

    #ifndef ROLE
        I am using syntax error to denote undefined macro here!
    #endif
    #if ROLE == 'F'
        assert_pdPASS(xTaskCreate(
            (TaskFunction_t) relayResidualPressureService, "relayBrPrSvc", 
            2300, NULL, PRIORITY_RELAY_RESIDUAL_PRESSURE, NULL
        ));
    #endif

    ESP_LOGI(PROJECT_TAG, "app_main() ok");
}

#ifndef ROLE
    I am using syntax error to denote undefined macro here!
#endif
#if ROLE == 'F'
static void relayResidualPressureService(void* _) {
    while (1) {
        sendPacketS(get_residual_pressure());
        delayTaskMs(1000 / RESIDUAL_PRESSURE_FPS);
    }
}
#endif

#ifndef ROLE
    I am using syntax error to denote undefined macro here!
#endif
#if ROLE == 'F'
    static void process_fingers(void* _) {
        while (1) {
            static int finger_i;
            if (didTouchChange(&finger_i)) {
                onFingerChange();
                if (startup_barrier_passed) {
                    sendPacketF(finger_i, getFingers()[finger_i]);
                    // proc first knows the fingers, then the note event. 
                    flushNoteEvent();
                }
            }

            vTaskDelay(1);
        }
    }

    static void process_pressure(void* _) {
        while (1) {
            static int breath_pressure;
            measureBreath();
            callbackVoidVoid_t onCalibrateOk = NULL;
            if (startup_barrier_passed) { 
                onCalibrateOk = (callbackVoidVoid_t) sendPacketR;
            }
            if (getBreathPressure(
                &breath_pressure, onCalibrateOk
            )) {
                onPressureChange(breath_pressure);
                if (startup_barrier_passed) {
                    flushNoteEvent();
                }
            }

            vTaskDelay(1);
        }
    }
#endif

#define BATTERY_REPORT_INTERVAL_MS_BURST 100
static void battery_service_burst(void* _) {
    for (int j = 2; j > 0; j --) {
        for (int i = 20; i > 0; i --) {
            (void) xTaskNotifyGiveIndexed(
                battery_service_chill_task, TASK_NOTIFICATION_INDEX
            );
            delayTaskMs(BATTERY_REPORT_INTERVAL_MS_BURST);
        }
        if (j == 2) {
            ulTaskNotifyTakeIndexed(
                TASK_NOTIFICATION_INDEX, pdTRUE, portMAX_DELAY
            );
        }
    }
    vTaskDelete(NULL);
}

#define BATTERY_REPORT_INTERVAL_MS_CHILL 3000
static void battery_service_chill(void* _) {
    while (1) {
        float battery_level = measure_battery_voltage();
        if (isfinite(battery_level)) {
            ESP_LOGI(PROJECT_TAG, "Battery voltage: %.3f V", (double) battery_level);
            if (battery_level < 3.35f) {
                ESP_LOGW(PROJECT_TAG, "Battery voltage low!");
            }
            if (battery_level > 4.02f) {
                ESP_LOGW(PROJECT_TAG, "Battery voltage high!");
            }
            if (startup_barrier_passed) {
                sendPacketU(battery_level);
            }
        }
        (void) ulTaskNotifyTakeIndexed(
            TASK_NOTIFICATION_INDEX, pdTRUE, pdMS_TO_TICKS(
                BATTERY_REPORT_INTERVAL_MS_CHILL
            )
        );
    }
}
