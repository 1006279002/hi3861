#include <stdio.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_pwm.h"

#define RED_LED_PIN_NAME WIFI_IOT_IO_NAME_GPIO_10
#define RED_LED_PIN_FUNCTION WIFI_IOT_IO_FUNC_GPIO_10_GPIO

#define GREEN_LED_PIN_NAME WIFI_IOT_IO_NAME_GPIO_11
#define GREEN_LED_PIN_FUNCTION WIFI_IOT_IO_FUNC_GPIO_11_GPIO

#define BLUE_LED_PIN_NAME WIFI_IOT_IO_NAME_GPIO_12
#define BLUE_LED_PIN_FUNCTION WIFI_IOT_IO_FUNC_GPIO_12_GPIO

#define PWM_DUTY 64000
#define PWM_FREQ_DIVITION 64000

void initLightGPIO(void)
{
    GpioInit();
    IoSetFunc(RED_LED_PIN_NAME, WIFI_IOT_IO_FUNC_GPIO_10_PWM1_OUT);
    IoSetFunc(GREEN_LED_PIN_NAME, WIFI_IOT_IO_FUNC_GPIO_11_PWM2_OUT);
    IoSetFunc(BLUE_LED_PIN_NAME, WIFI_IOT_IO_FUNC_GPIO_12_PWM3_OUT);

    PwmInit(WIFI_IOT_PWM_PORT_PWM1);
    PwmInit(WIFI_IOT_PWM_PORT_PWM2);
    PwmInit(WIFI_IOT_PWM_PORT_PWM3);
}

static void ColorfulLightTask(void *arg){
    (void)arg;
    initLightGPIO();

    while(1){
        for(int i=1;i<=PWM_DUTY;i*=2){
            PwmStart(WIFI_IOT_PWM_PORT_PWM1,i,PWM_FREQ_DIVITION);
            usleep(500*1000); // 500ms
            PwmStop(WIFI_IOT_PWM_PORT_PWM1); 
        }
        for(int i=1;i<=PWM_DUTY;i*=2){
            PwmStart(WIFI_IOT_PWM_PORT_PWM2,i,PWM_FREQ_DIVITION);
            usleep(500*1000); // 500ms
            PwmStop(WIFI_IOT_PWM_PORT_PWM2); 
        }
        for(int i=1;i<=PWM_DUTY;i*=2){
            PwmStart(WIFI_IOT_PWM_PORT_PWM3,i,PWM_FREQ_DIVITION);
            usleep(300*1000); // 300ms
            PwmStop(WIFI_IOT_PWM_PORT_PWM3);
        }
    }
}

static void ColorfulLightDemo(void){
    osThreadAttr_t attr;

    attr.name = "ColorfulLightTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 4096;
    attr.priority = osPriorityNormal;

    if(osThreadNew((osThreadFunc_t)ColorfulLightTask, NULL, &attr) == NULL) {
        printf("[ColorfulLightDemo]Failed to create ColorfulLightTask!\n");
    }
}

APP_FEATURE_INIT(ColorfulLightDemo);

