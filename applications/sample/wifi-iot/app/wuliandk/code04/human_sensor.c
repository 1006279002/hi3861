#include <stdio.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_adc.h"

#define HUMAN_SENSOR_CHAN_NAME WIFI_IOT_ADC_CHANNEL_3

unsigned short data = 0;

static void HumanSensorTask(void *arg){
    (void)arg;

    while(1){
        AdcRead(HUMAN_SENSOR_CHAN_NAME, &data, WIFI_IOT_ADC_EQU_MODEL_4, WIFI_IOT_ADC_CUR_BAIS_DEFAULT, 0);
        printf("AS312 feinier adc:%d\n", data);
        sleep(1); // Sleep for 1 second
    }
}

static void HumanSensorDemo(void){
    osThreadAttr_t attr;

    attr.name = "HumanSensorTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 4096; // Stack size in bytes
    attr.priority = osPriorityNormal;

    if (osThreadNew(HumanSensorTask, NULL, &attr) == NULL) {
        printf("[HumanSensorDemo]Failed to create HumanSensorTask!\n");
    }
}

APP_FEATURE_INIT(HumanSensorDemo);

