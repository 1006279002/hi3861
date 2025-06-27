#include <stdio.h>
#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_watchdog.h"
#include "wifiiot_pwm.h"
#include "hi_pwm.h"
#include "hi_gpio.h"

static volatile int g_buttonPressed = 0;

static const uint16_t g_tuneFreqs[]={
    0,
    38223,
    34052,
    30338,
    28635,
    25511,
    22728,
    20249,
    51021
};

static const uint8_t g_scoreNotes[]={
    1,2,3,1,1,2,3,1,3,4,5,3,4,5,
    5,6,5,4,3,1,5,6,5,4,3,1,1,8,1,1,8,1,
};

static const uint8_t g_scoreDurations[]={
    4,4,4,4,4,4,4,4,4,4,8,4,4,8,
    3,1,3,1,4,4,3,1,3,1,4,4,4,4,8,4,4,8,
};

int count = 1;
int flag = 1;
static void *BeeperMusicTask(const char* arg){
    (void)arg;
    printf("BeeperMusicTask started\n");
    hi_pwm_set_clock(PWM_CLK_XTAL);
    while(flag){
        for(size_t i = 0;i<sizeof(g_scoreNotes)/sizeof(g_scoreNotes[0]);i++){
            uint32_t tune = g_scoreNotes[i];
            uint16_t freqDivisor = g_tuneFreqs[tune];
            uint32_t tuneInterval = g_scoreDurations[i]*(125*1000);
            printf("%d %d %d %d\r\n",tune,(40*1000*1000)/freqDivisor,freqDivisor,tuneInterval);
            PwmStart(WIFI_IOT_PWM_PORT_PWM0, freqDivisor/2, freqDivisor);
            
            GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_11, WIFI_IOT_GPIO_VALUE1);
            usleep(tuneInterval/3);
            GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_11, WIFI_IOT_GPIO_VALUE0);
            GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_12, WIFI_IOT_GPIO_VALUE1);
            usleep(tuneInterval/3);
            GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_12, WIFI_IOT_GPIO_VALUE0);
            GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_10, WIFI_IOT_GPIO_VALUE1);
            usleep(tuneInterval/3);
            GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_10, WIFI_IOT_GPIO_VALUE0);
            
            PwmStop(WIFI_IOT_PWM_PORT_PWM0);
        }
    }
    return NULL;
}

static void OnButtonPressed(char* arg)
{
    (void)arg;
    printf("Button pressed!\r\n");
    count++;
    if(count % 2 == 0){
        flag = 0;
    }
}

static void StartS3Task(void){
    osThreadAttr_t attr;
    GpioInit();

    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_8, WIFI_IOT_IO_FUNC_GPIO_8_GPIO);
    GpioSetDir(WIFI_IOT_GPIO_IDX_8, WIFI_IOT_GPIO_DIR_IN);
    IoSetPull(WIFI_IOT_GPIO_IDX_8, WIFI_IOT_IO_PULL_UP);

    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_10,WIFI_IOT_IO_FUNC_GPIO_10_GPIO);
    GpioSetDir(WIFI_IOT_GPIO_IDX_10, WIFI_IOT_GPIO_DIR_OUT);
    
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_11,WIFI_IOT_IO_FUNC_GPIO_11_GPIO);
    GpioSetDir(WIFI_IOT_GPIO_IDX_11, WIFI_IOT_GPIO_DIR_OUT);

    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_12,WIFI_IOT_IO_FUNC_GPIO_12_GPIO);
    GpioSetDir(WIFI_IOT_GPIO_IDX_12, WIFI_IOT_GPIO_DIR_OUT);

    GpioRegisterIsrFunc(WIFI_IOT_GPIO_IDX_8, WIFI_IOT_INT_TYPE_EDGE,WIFI_IOT_GPIO_EDGE_FALL_LEVEL_LOW, OnButtonPressed, NULL);

    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_9, WIFI_IOT_IO_FUNC_GPIO_9_PWM0_OUT);
    PwmInit(WIFI_IOT_PWM_PORT_PWM0);
    WatchDogDisable();

    attr.name = "BeeperMusicTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 1024;
    attr.priority = osPriorityNormal;
    if(osThreadNew((osThreadFunc_t)BeeperMusicTask, NULL, &attr) == NULL) {
        printf("Failed to create BeeperMusicTask!\n");
    }
    printf("BeeperMusicTask created successfully!\n");
}

static void StartS3demo(void)
{
    osThreadAttr_t attr;

    attr.name = "S3Task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 1024;
    attr.priority = osPriorityNormal;

    if(osThreadNew((osThreadFunc_t)StartS3Task, NULL, &attr) == NULL) {
        printf("Failed to create StartS3Task!\n");
    }
    printf("StartS3Task created successfully!\n");
}

APP_FEATURE_INIT(StartS3demo);
