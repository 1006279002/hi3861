#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_i2c.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_adc.h"
#include "wifiiot_errno.h"

#include "aht20.h"

#define GAS_SENSOR_CHAN_NAME WIFI_IOT_ADC_CHANNEL_5

#define AHT20_BAUDRATE 400*1000
#define AHT20_I2C_IDX WIFI_IOT_I2C_IDX_0

void init(void){
    GpioInit();
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_13, WIFI_IOT_IO_FUNC_GPIO_13_I2C0_SDA);
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_14, WIFI_IOT_IO_FUNC_GPIO_14_I2C0_SCL);
    I2cInit(AHT20_I2C_IDX, AHT20_BAUDRATE);
}

static void AhtDemoTask(void *arg){
    (void)arg;
    uint32_t retval = 0;
    float temp = 0.0f, humi = 0.0f;
    unsigned short data = 0;

    while(WIFI_IOT_SUCCESS != AHT20_Calibrate()){
        printf("AHT20 sensor init failed!\r\n");
        usleep(1000);
    }
    while(1){
        int rstflag = AdcRead(GAS_SENSOR_CHAN_NAME, &data, WIFI_IOT_ADC_EQU_MODEL_4, WIFI_IOT_ADC_CUR_BAIS_DEFAULT, 0);
        retval = AHT20_StartMeasure();
        if(retval != WIFI_IOT_SUCCESS && rstflag!=WIFI_IOT_SUCCESS){
            printf("trigger measure failed!\r\n");
        }else{
            retval = AHT20_GetMeasureResult(&temp, &humi);
            printf("temp:%.2f, humi:%.2f, gas: %d\r\n", temp, humi, data);
        }
        sleep(1); // 每秒测量一次
    }
}

static void AhtDemo(void)
{
    init();
    osThreadAttr_t attr = {
        .name = "AhtDemoTask",
        .stack_size = 4096,
        .priority = osPriorityNormal,
    };
    if(osThreadNew(AhtDemoTask, NULL, &attr) == NULL) {
        printf("Failed to create AhtDemoTask!\r\n");
    }
}

APP_FEATURE_INIT(AhtDemo);
