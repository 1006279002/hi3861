/**
 * @file hm_package_storage_system.c
 * @brief 环境监测包存储系统主程序
 * @details 该程序实现Wi-Fi连接和TCP客户端功能，用于环境传感器数据采集和传输
 * @version 1.0
 * @date 2025-06-27
 */

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

/* 网络配置常量 */
#define SERVER_IP_ADDRESS   "192.168.80.8"
#define SERVER_PORT         5001
#define TASK_STACK_SIZE     10240

/**
 * @brief 网络任务主函数
 * @details 负责Wi-Fi连接和TCP客户端数据传输
 * 
 * @param arg 任务参数（未使用）
 */
static void NetworkTask(void *arg)
{
    // 避免编译器未使用参数警告
    (void)arg;

    printf("Starting network task...\n");
    
    // 连接Wi-Fi网络
    printf("Attempting to connect to Wi-Fi...\n");
    if (!connect_wifi()) {
        printf("ERROR: Failed to connect to Wi-Fi network\n");
        printf("Network task terminated due to Wi-Fi connection failure\n");
        return;
    }
    
    printf("SUCCESS: Wi-Fi connected successfully\n");

    // 启动TCP客户端进行数据传输
    printf("Starting TCP client connection to %s:%d\n", SERVER_IP_ADDRESS, SERVER_PORT);
    
    if (!conent_tcp_server(SERVER_IP_ADDRESS, SERVER_PORT)) {
        printf("ERROR: TCP client failed to start or encountered critical error\n");
        printf("Network task terminated due to TCP client failure\n");
        return;
    }
    
    // 理论上不应该到达这里，因为TCP客户端应该一直运行
    printf("WARNING: TCP client unexpectedly returned\n");
}

/**
 * @brief 创建并启动网络任务
 * @details 初始化网络任务属性并创建任务线程
 */
static void NetworkDemo(void)
{
    printf("[NetworkDemo] Initializing network task...\n");
    
    // 配置任务属性
    osThreadAttr_t attr = {
        .name = "NetworkTask",
        .stack_size = TASK_STACK_SIZE,
        .priority = osPriorityNormal,
    };

    // 创建网络任务线程
    if (osThreadNew(NetworkTask, NULL, &attr) == NULL) {
        printf("[NetworkDemo] ERROR: Failed to create NetworkTask\n");
    } else {
        printf("[NetworkDemo] SUCCESS: NetworkTask created successfully\n");
    }
}

APP_FEATURE_INIT(NetworkDemo);


