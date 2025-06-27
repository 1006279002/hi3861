#include <stdio.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"

#include "oled_ssd1306.h"

void init(void){
    GpioInit();
    OledInit();
}

static void OledTask(void *arg){
    (void)arg;
    init();
    OledFillScreen(0x00);
    OledShowString(0, 0, "Hello, HarmoneyOS!", 1);
    printf("OLED initialized and message displayed.\n");
    sleep(1);
    for(int i=0;i<3;i++){
        OledFillScreen(0x00);
        for(int j=0;j<8;j++){
            static const char text[]="ABCDEFGHIJKLMNOP";
            OledShowString(0, j, text, 1);
        }
        sleep(1);
    }
}

static void OledDemo(void)
{
    osThreadAttr_t attr = {
        .name = "OledTask",
        .stack_size = 4096,
        .priority = osPriorityNormal,
    };
    if(osThreadNew(OledTask, NULL, &attr) == NULL) {
        printf("Failed to create OledTask!\n");
    }
}

APP_FEATURE_INIT(OledDemo);
