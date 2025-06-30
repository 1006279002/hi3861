/**
 * @file wifi_utils.c
 * @brief Wi-Fi连接工具函数实现
 * @version 1.0
 * @date 2025-06-27
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include "wifi_utils.h"
#include "wifi_error_code.h"
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifi_device.h"
#include "lwip/netifapi.h"
#include "lwip/api_shell.h"

/* 私有函数声明 */
static WifiErrorCode wifi_enable_and_configure(const wifi_config_t *config, int *net_id);
static WifiErrorCode wifi_start_dhcp(struct netif *iface);
static void wifi_delay_ms(uint32_t milliseconds);

/* 扩展错误码映射宏 */
#define WIFI_ERROR_DHCP_FAILED          ERROR_WIFI_NOT_AVAILABLE
#define WIFI_ERROR_INTERFACE_NOT_FOUND  ERROR_WIFI_IFACE_INVALID
#define WIFI_ERROR_CONFIG_FAILED        ERROR_WIFI_INVALID_ARGS
#define WIFI_ERROR_ENABLE_FAILED        ERROR_WIFI_NOT_STARTED
#define WIFI_ERROR_CONNECT_FAILED       ERROR_WIFI_NOT_AVAILABLE

/**
 * @brief 连接到Wi-Fi网络
 * 
 * @param config Wi-Fi配置信息
 * @return WifiErrorCode 连接结果
 */
WifiErrorCode wifi_connect(const wifi_config_t *config)
{
    if (config == NULL) {
        printf("Error: Wi-Fi config is NULL\n");
        return ERROR_WIFI_INVALID_ARGS;
    }

    if (strlen(config->ssid) == 0 || strlen(config->ssid) >= WIFI_SSID_MAX_LEN) {
        printf("Error: Invalid SSID length\n");
        return ERROR_WIFI_INVALID_ARGS;
    }

    if (strlen(config->password) == 0 || strlen(config->password) >= WIFI_PASSWORD_MAX_LEN) {
        printf("Error: Invalid password length\n");
        return ERROR_WIFI_INVALID_ARGS;
    }

    int net_id = -1;
    WifiErrorCode result;

    // 启用Wi-Fi并配置
    result = wifi_enable_and_configure(config, &net_id);
    if (result != WIFI_SUCCESS) {
        return result;
    }

    // 连接到网络
    WifiErrorCode err_code = ConnectTo(net_id);
    printf("ConnectTo(%d): %d\n", net_id, err_code);
    
    if (err_code != WIFI_SUCCESS) {
        printf("Failed to connect to Wi-Fi network\n");
        return WIFI_ERROR_CONNECT_FAILED;
    }

    wifi_delay_ms(DHCP_START_DELAY_MS);

    // 查找网络接口并启动DHCP
    struct netif *iface = netifapi_netif_find(WIFI_INTERFACE_NAME);
    if (iface == NULL) {
        printf("Failed to find %s interface\n", WIFI_INTERFACE_NAME);
        return WIFI_ERROR_INTERFACE_NOT_FOUND;
    }

    return wifi_start_dhcp(iface);
}

/**
 * @brief 使用默认配置连接到Wi-Fi网络（向后兼容）
 * 
 * @return bool 连接状态，成功返回true，失败返回false
 */
bool connect_wifi(void)
{
    wifi_config_t config = {0};
    
    // 获取默认配置
    WifiErrorCode config_result = wifi_get_default_config(&config);
    if (config_result != WIFI_SUCCESS) {
        printf("Failed to get default Wi-Fi config: %d\n", config_result);
        return false;
    }
    
    WifiErrorCode result = wifi_connect(&config);
    return (result == WIFI_SUCCESS);
}

/**
 * @brief 启用Wi-Fi并配置网络
 * 
 * @param config Wi-Fi配置信息
 * @param net_id 输出参数，网络ID
 * @return WifiErrorCode 操作结果
 */
static WifiErrorCode wifi_enable_and_configure(const wifi_config_t *config, int *net_id)
{
    WifiErrorCode err_code;
    WifiDeviceConfig ap_config = {0};

    // 配置AP参数
    strncpy(ap_config.ssid, config->ssid, sizeof(ap_config.ssid) - 1);
    strncpy(ap_config.preSharedKey, config->password, sizeof(ap_config.preSharedKey) - 1);
    ap_config.securityType = WIFI_SEC_TYPE_PSK;

    // 启用Wi-Fi
    err_code = EnableWifi();
    if (err_code != WIFI_SUCCESS) {
        printf("Failed to enable Wi-Fi: %d\n", err_code);
        return WIFI_ERROR_ENABLE_FAILED;
    }

    // 添加设备配置
    err_code = AddDeviceConfig(&ap_config, net_id);
    if (err_code != WIFI_SUCCESS) {
        printf("Failed to add device config: %d\n", err_code);
        return WIFI_ERROR_CONFIG_FAILED;
    }

    return WIFI_SUCCESS;
}

/**
 * @brief 启动DHCP服务
 * 
 * @param iface 网络接口
 * @return WifiErrorCode 操作结果
 */
static WifiErrorCode wifi_start_dhcp(struct netif *iface)
{
    err_t ret = netifapi_dhcp_start(iface);
    printf("netifapi_dhcp_start: %d\n", ret);

    if (ret != ERR_OK) {
        printf("Failed to start DHCP on %s interface\n", WIFI_INTERFACE_NAME);
        return WIFI_ERROR_DHCP_FAILED;
    }

    wifi_delay_ms(DHCP_CHECK_DELAY_MS);

    ret = netifapi_netif_common(iface, dhcp_clients_info_show, NULL);
    printf("netifapi_netif_common: %d\n", ret);

    if (ret != ERR_OK) {
        printf("Failed to get DHCP client info on %s interface\n", WIFI_INTERFACE_NAME);
        return WIFI_ERROR_DHCP_FAILED;
    }

    printf("DHCP started successfully on %s interface\n", WIFI_INTERFACE_NAME);
    return WIFI_SUCCESS;
}

/**
 * @brief 延时函数（毫秒）
 * 
 * @param milliseconds 延时时间（毫秒）
 */
static void wifi_delay_ms(uint32_t milliseconds)
{
    usleep(milliseconds * 1000);
}

/**
 * @brief 设置Wi-Fi配置参数
 * 
 * @param ssid Wi-Fi网络名称
 * @param password Wi-Fi密码
 * @param config 输出参数，配置结构体
 * @return WifiErrorCode 配置结果
 */
WifiErrorCode wifi_set_config(const char *ssid, const char *password, wifi_config_t *config)
{
    if (ssid == NULL || password == NULL || config == NULL) {
        printf("Error: Invalid parameters for Wi-Fi config\n");
        return ERROR_WIFI_INVALID_ARGS;
    }

    if (strlen(ssid) == 0 || strlen(ssid) >= WIFI_SSID_MAX_LEN) {
        printf("Error: Invalid SSID length\n");
        return ERROR_WIFI_INVALID_ARGS;
    }

    if (strlen(password) == 0 || strlen(password) >= WIFI_PASSWORD_MAX_LEN) {
        printf("Error: Invalid password length\n");
        return ERROR_WIFI_INVALID_ARGS;
    }

    // 清空配置结构体
    memset(config, 0, sizeof(wifi_config_t));
    
    // 设置SSID和密码
    strncpy(config->ssid, ssid, sizeof(config->ssid) - 1);
    strncpy(config->password, password, sizeof(config->password) - 1);
    
    return WIFI_SUCCESS;
}

/**
 * @brief 获取默认Wi-Fi配置
 * 
 * @param config 输出参数，配置结构体
 * @return WifiErrorCode 配置结果
 */
WifiErrorCode wifi_get_default_config(wifi_config_t *config)
{
    if (config == NULL) {
        printf("Error: Config parameter is NULL\n");
        return ERROR_WIFI_INVALID_ARGS;
    }
    
    return wifi_set_config("Apple shop", "20031126Gao!", config);
}
