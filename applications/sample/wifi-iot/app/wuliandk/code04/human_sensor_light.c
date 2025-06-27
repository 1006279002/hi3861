#include <stdio.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"

void init(void){
    GpioInit();

    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_10, WIFI_IOT_IO_FUNC_GPIO_10_GPIO);
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_11, WIFI_IOT_IO_FUNC_GPIO_11_GPIO);
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_12, WIFI_IOT_IO_FUNC_GPIO_12_GPIO);

    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_10, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_11, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_12, WIFI_IOT_GPIO_DIR_OUT);

    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_9, WIFI_IOT_IO_FUNC_GPIO_9_GPIO);
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_9, WIFI_IOT_GPIO_DIR_IN);

    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_7, WIFI_IOT_IO_FUNC_GPIO_7_GPIO);
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_7, WIFI_IOT_GPIO_DIR_IN);
    IoSetPull(WIFI_IOT_IO_NAME_GPIO_7, WIFI_IOT_IO_PULL_UP);
}

static void sensorLightTask(void *arg){
    (void)arg;
    init();

    WifiIotGpioValue rel = 0; // 用于接收人体红外感应器电压数据变量
    WifiIotGpioValue hwrel = 0; // 用于接收光敏传感器电压数据变量

    while(1){
        printf("[sensor_light====>]sensorLight rentiganying value ref = %d\r\n", (int)rel);
        printf("[sensor_light====>]sensorLight guangmindianzu value hwref = %d\r\n", (int)hwrel);

        GpioGetInputVal(WIFI_IOT_IO_NAME_GPIO_7, &rel); // 获取人体红外感应器电压数据
        GpioGetInputVal(WIFI_IOT_IO_NAME_GPIO_9, &hwrel); // 获取光敏传感器电压数据

        GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_10, (int)rel);
        GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_11, (int)rel);
        GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_12, (int)rel);
        usleep(20*1000); // 延时20毫秒
    }
}

static void startSensorLightTask(void){
    osThreadAttr_t attr = {
        .name = "sensorLightTask",
        .stack_size = 4096,
        .priority = osPriorityNormal,
    };
    if(osThreadNew(sensorLightTask, NULL, &attr) == NULL) {
        printf("[sensor_light====>]Failed to create sensorLightTask!\n");
    }
}

APP_FEATURE_INIT(startSensorLightTask);




