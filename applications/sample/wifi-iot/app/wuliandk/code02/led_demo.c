#include <stdio.h>
#include <unistd.h>
#include "ohos_init.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "cmsis_os2.h"

void* LedExampleEntry(void *arg)
{
    (void)arg;
    while (1) {
        GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_9, WIFI_IOT_GPIO_VALUE0); // LED ON
        usleep(4000*1000); // 4 s
        GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_9, WIFI_IOT_GPIO_VALUE1); // LED OFF
        usleep(4000*1000); // 4 s
    }
    return NULL;
}

void LedExample(void)
{
    osThreadAttr_t attr;

    GpioInit();
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_9, WIFI_IOT_IO_FUNC_GPIO_9_GPIO);
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_9, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_9, WIFI_IOT_GPIO_VALUE1); // Initialize LED to OFF

    attr.name = "LedTask";
    attr.attr_bits = 0;
    attr.stack_size = 512;
    attr.priority = 25;

    if(osThreadNew((osThreadFunc_t)LedExampleEntry, NULL, &attr) == NULL) {
        printf("Failed to create LedTask!\n");
    } else {
        printf("LedTask created successfully!\n");
    }
}

SYS_RUN(LedExample);

