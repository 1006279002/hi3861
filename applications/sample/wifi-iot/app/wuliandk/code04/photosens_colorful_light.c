#include <stdio.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_adc.h"
#include "wifiiot_pwm.h"

#define RED_LED_PIN_NAME WIFI_IOT_IO_NAME_GPIO_10
#define RED_LED_PIN_FUNCTION WIFI_IOT_IO_FUNC_GPIO_10_GPIO

#define GREEN_LED_PIN_NAME WIFI_IOT_IO_NAME_GPIO_11
#define GREEN_LED_PIN_FUNCTION WIFI_IOT_IO_FUNC_GPIO_11_GPIO

#define BLUE_LED_PIN_NAME WIFI_IOT_IO_NAME_GPIO_12
#define BLUE_LED_PIN_FUNCTION WIFI_IOT_IO_FUNC_GPIO_12_GPIO

#define PWM_DUTY 64000
#define PWM_FREQ_DIVITION 64000

#define PHOTO_SENSOR_CHAN_NAME WIFI_IOT_ADC_CHANNEL_4

unsigned short data = 0;

void initLightGPIO(void){
    GpioInit();

    IoSetFunc(RED_LED_PIN_NAME, WIFI_IOT_IO_FUNC_GPIO_10_PWM1_OUT);
    IoSetFunc(GREEN_LED_PIN_NAME, WIFI_IOT_IO_FUNC_GPIO_11_PWM2_OUT);
    IoSetFunc(BLUE_LED_PIN_NAME, WIFI_IOT_IO_FUNC_GPIO_12_PWM3_OUT);

    PwmInit(WIFI_IOT_PWM_PORT_PWM1);
    PwmInit(WIFI_IOT_PWM_PORT_PWM2);
    PwmInit(WIFI_IOT_PWM_PORT_PWM3);
}

static void PhotoSensorTask(void *arg){
    (void)arg;
    initLightGPIO();

    while(1){
        AdcRead(PHOTO_SENSOR_CHAN_NAME, &data, WIFI_IOT_ADC_EQU_MODEL_4, WIFI_IOT_ADC_CUR_BAIS_DEFAULT, 0);
        printf("GL5537 adc: %d\n", data);
        if(data>=1800 && data<=1999){
            PwmStart(WIFI_IOT_PWM_PORT_PWM1, PWM_DUTY, PWM_FREQ_DIVITION); // Red LED ON
            PwmStart(WIFI_IOT_PWM_PORT_PWM2, PWM_DUTY, PWM_FREQ_DIVITION); // Green LED ON
            PwmStart(WIFI_IOT_PWM_PORT_PWM3, PWM_DUTY, PWM_FREQ_DIVITION); // Blue LED ON
        }else{
            PwmStop(WIFI_IOT_PWM_PORT_PWM1); // Red LED OFF
            PwmStop(WIFI_IOT_PWM_PORT_PWM2); // Green LED OFF
            PwmStop(WIFI_IOT_PWM_PORT_PWM3); // Blue LED OFF
        }
        sleep(1);
    }
}

static void PhotoSensorDemo(void)
{
    osThreadAttr_t attr={
        .name = "PhotoSensorTask",
        .stack_size = 4096, // Stack size for the task
        .priority = osPriorityNormal, // Task priority
    };

    if (osThreadNew(PhotoSensorTask, NULL, &attr) == NULL) {
        printf("[PhotoSensorDemo]Failed to create PhotoSensorTask!\n");
    }
}

APP_FEATURE_INIT(PhotoSensorDemo);
