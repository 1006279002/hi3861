#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_adc.h"
#include "wifiiot_errno.h"

#define GAS_SENSOR_CHAN_NAME WIFI_IOT_ADC_CHANNEL_5

static void Mq2DemoTask(void *arg){
    (void)arg;
    GpioInit();
    while(1){
        unsigned short data = 0;

        int rstflag = AdcRead(GAS_SENSOR_CHAN_NAME, &data, WIFI_IOT_ADC_EQU_MODEL_4, WIFI_IOT_ADC_CUR_BAIS_DEFAULT, 0);
        if(rstflag == WIFI_IOT_SUCCESS) {
            printf("mq-2 values:%d\n", data);
        }

        sleep(1); // Sleep for 1 second
    }
}

static void Mq2Demo(void)
{
    osThreadAttr_t attr = {
        .name = "Mq2DemoTask",
        .stack_size = 4096,
        .priority = osPriorityNormal,
    };
    if(osThreadNew(Mq2DemoTask, NULL, &attr) == NULL) {
        printf("Failed to create Mq2DemoTask!\n");
    }
}

APP_FEATURE_INIT(Mq2Demo);



