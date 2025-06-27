/**
 * @file wifi_utils.h
 * @brief Wi-Fi连接工具函数头文件
 * @version 1.0
 * @date 2025-06-27
 */

#ifndef WIFI_UTILS_H
#define WIFI_UTILS_H

#include <stdbool.h>
#include "wifi_error_code.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 常量定义 */
#define WIFI_SSID_MAX_LEN       32
#define WIFI_PASSWORD_MAX_LEN   64
#define WIFI_INTERFACE_NAME     "wlan0"
#define DHCP_START_DELAY_MS     1000
#define DHCP_CHECK_DELAY_MS     2000


/* Wi-Fi配置结构体 */
typedef struct {
    char ssid[WIFI_SSID_MAX_LEN];
    char password[WIFI_PASSWORD_MAX_LEN];
} wifi_config_t;

/**
 * @brief 连接到Wi-Fi网络
 * 
 * @param config Wi-Fi配置信息
 * @return WifiErrorCode 连接结果
 */
WifiErrorCode wifi_connect(const wifi_config_t *config);

/**
 * @brief 使用默认配置连接到Wi-Fi网络（向后兼容）
 * 
 * @return bool 连接状态，成功返回true，失败返回false
 */
bool connect_wifi(void);

/**
 * @brief 设置Wi-Fi配置参数
 * 
 * @param ssid Wi-Fi网络名称
 * @param password Wi-Fi密码
 * @param config 输出参数，配置结构体
 * @return WifiErrorCode 配置结果
 */
WifiErrorCode wifi_set_config(const char *ssid, const char *password, wifi_config_t *config);

/**
 * @brief 获取默认Wi-Fi配置
 * 
 * @param config 输出参数，配置结构体
 * @return WifiErrorCode 配置结果
 */
WifiErrorCode wifi_get_default_config(wifi_config_t *config);

#ifdef __cplusplus
}
#endif

#endif /* WIFI_UTILS_H */