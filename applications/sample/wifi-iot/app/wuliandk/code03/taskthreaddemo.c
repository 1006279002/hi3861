#include <stdio.h>
#include "ohos_init.h"
#include "cmsis_os2.h"

static void *callThreadFunction(void *data)
{
    data=data;
    printf("\r\n\r\nExample task is running!\r\n\r\n");
    return data;
}

void taskThreadFunction(void){
    osThreadAttr_t attr;
    attr.name = "example_task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 1024; // Stack size in bytes
    attr.priority = osPriorityNormal;
    if(osThreadNew((osThreadFunc_t)callThreadFunction, NULL, &attr) == NULL) {
        printf("Failed to create thread!\r\n");
    }
    printf("Thread created successfully!\r\n");
}

APP_FEATURE_INIT(taskThreadFunction);
