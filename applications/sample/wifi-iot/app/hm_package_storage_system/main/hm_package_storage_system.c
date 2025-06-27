#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifi_device.h"
#include "lwip/netifapi.h"
#include "lwip/api_shell.h"

#include "wifi_utils.h"
#include "tcp_client.h"

static void NetWorkTask(void *arg)
{
    (void)arg;

    connect_wifi();
    printf("begin demo\r\n");

    unsigned short port = 5001; // 服务器端口
    conent_tcp_server("192.168.80.8", port); // 服务器IP地址
}

static void NetWorkDemo(void){
    osThreadAttr_t attr = {
        .name = "NetWorkTask",
        .stack_size = 10240,
        .priority = osPriorityNormal,
    };

    if (osThreadNew(NetWorkTask, NULL, &attr) == NULL) {
        printf("[NetWorkDemo]Failed to create NetWorkTask!\n");
    }
}

APP_FEATURE_INIT(NetWorkDemo);


