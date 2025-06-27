#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_i2c.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_adc.h"
#include "wifiiot_errno.h"

#include "oled_ssd1306.h"
#include "aht20.h"

#define GAS_SENSOR_ChAN_NAME WIFI_IOT_ADC_CHANNEL_5
#define AHT20_BAUDRATE 400*1000
#define AHT20_I2C_IDX WIFI_IOT_I2C_IDX_0
#define ADC_RESOLUTION 2048

uint32_t retval = 0;
float humidity = 0.0f;
float temperature = 0.0f;
unsigned short data = 0;
static char line[32] = {0};

void init(void){
    GpioInit();
    OledInit();
    OledFillScreen(0x00);
    I2cInit(AHT20_I2C_IDX, AHT20_BAUDRATE);
    while(WIFI_IOT_SUCCESS != AHT20_Calibrate()){
        printf("AHT20 sensor init failed!\r\n");
        usleep(1000);
    }
}

static void EnvironmentTask(void *arg){
    (void)arg;
    init();
    while(1){
        if(AHT20_StartMeasure() != WIFI_IOT_SUCCESS){
            printf("trigger measure failed!\r\n");
        }
        if(AHT20_GetMeasureResult(&temperature,&humidity) != WIFI_IOT_SUCCESS){
            printf("get measure data failed!\r\n");
        }
        char str[100]={0};
        AdcRead(GAS_SENSOR_ChAN_NAME, &data,WIFI_IOT_ADC_EQU_MODEL_4,WIFI_IOT_ADC_CUR_BAIS_DEFAULT, 0);
        OledShowString(0,0,"Sensor values:", 1);
        snprintf(line, sizeof(line), "temp: %.2f", temperature);
        OledShowString(0,1,line, 1);
        snprintf(line, sizeof(line), "humi: %.2f", humidity);
        OledShowString(0,2,line, 1);
        snprintf(line, sizeof(line), "gas: %d", data);
        OledShowString(0,3,line, 1);
        snprintf(str,sizeof(str),"%.2f,%.2f,%d", temperature, humidity, data);
        printf("temp,humi,gas: %s\r\n", str);
        sleep(1);
    }
}

static void EnvironmentDemo(void)
{
    osThreadAttr_t attr = {
        .name = "EnvironmentTask",
        .stack_size = 4096,
        .priority = osPriorityNormal,
    };
    if(osThreadNew(EnvironmentTask, NULL, &attr) == NULL) {
        printf("Failed to create EnvironmentTask!\n");
    }
}

APP_FEATURE_INIT(EnvironmentDemo);



