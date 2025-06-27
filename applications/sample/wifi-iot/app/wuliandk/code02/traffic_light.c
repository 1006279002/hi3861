#include <stdio.h>
#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"

#define RED_TIME_US     10*1000*1000   // 4 second
#define GREEN_TIME_US   10*1000*1000   // 4 second
#define YELLOW_TIME_US  5*1000*1000    // 2 second

static void *TrafficLightTask(const char *arg)
{
    (void)arg;
    while (1) {
        // Red light ON
        GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_10, 1); // Red LED ON
        GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_11, 0); // Green LED OFF
        GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_12, 0); // Yellow LED OFF
        usleep(RED_TIME_US);

        // Green light ON
        GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_10, 0); // Red LED OFF
        GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_11, 1); // Green LED ON
        GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_12, 0); // Yellow LED OFF
        usleep(GREEN_TIME_US);

        // Yellow light ON
        GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_10, 0); // Red LED OFF
        GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_11, 0); // Green LED OFF
        GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_12, 1); // Yellow LED ON
        usleep(YELLOW_TIME_US);
    }
    return NULL;
}

static void TrafficLightExampleEntry(void)
{
    osThreadAttr_t attr;

    GpioInit();
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_10, WIFI_IOT_IO_FUNC_GPIO_10_GPIO); // Red LED
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_11, WIFI_IOT_IO_FUNC_GPIO_11_GPIO); // Green LED
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_12, WIFI_IOT_IO_FUNC_GPIO_12_GPIO); // Yellow LED

    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_10, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_11, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_12, WIFI_IOT_GPIO_DIR_OUT);

    attr.name = "TrafficLightTask";
    attr.attr_bits = 0;
    attr.stack_size = 512;
    attr.priority = 25;

    if (osThreadNew((osThreadFunc_t)TrafficLightTask, NULL, &attr) == NULL) {
        printf("Failed to create TrafficLightTask!\n");
    } else {
        printf("TrafficLightTask created successfully!\n");
    }
}

SYS_RUN(TrafficLightExampleEntry);
