/**
 * @file tcp_client.c
 * @brief TCP客户端实现，包含环境监测和网络传输功能
 * @version 1.0
 * @date 2025-06-27
 */

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

#include "tcp_client.h"
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "hi_wifi_api.h"
#include "lwip/ip_addr.h"
#include "lwip/netifapi.h"
#include "lwip/sockets.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_pwm.h"
#include "wifiiot_i2c.h"
#include "wifiiot_errno.h"
#include "hi_pwm.h"

#include "oled_ssd1306.h"
#include "aht20.h"

/* 硬件配置常量 */
#define AHT20_BAUDRATE          (400 * 1000)           // 400KHz
#define AHT20_I2C_IDX           WIFI_IOT_I2C_IDX_0     // 湿度传感器I2C索引

/* 传感器阈值常量 */
#define TEMPERATURE_THRESHOLD   40.0f                  // 温度报警阈值(°C)

/* 显示和数据处理常量 */
#define DISPLAY_LINE_SIZE       32
#define STRING_BUFFER_SIZE      20
#define LINESTR_BUFFER_SIZE     80
#define RESPONSE_BUFFER_SIZE    128

/* 蜂鸣器PWM配置 */
#define BUZZER_FREQ_DIVISOR     34052
#define BUZZER_DURATION_MS      (2 * 1000 * 1000)      // 2秒
#define DETECTION_INTERVAL_MS   (1 * 1000 * 1000)     // 1秒

/* 数据采集间隔 */
#define SENSOR_READ_INTERVAL_S  2                      // 2秒

/* 全局变量 */
static char response[RESPONSE_BUFFER_SIZE] = "";

/* 传感器数据结构体 */
typedef struct {
    float temperature;
    float humidity;
} sensor_data_t;

/* 私有函数声明 */
static void init_hardware(void);
static int read_sensor_data(sensor_data_t *data);
static void display_sensor_data(const sensor_data_t *data);
static void check_alarm_conditions(const sensor_data_t *data);
static void format_sensor_data_string(const sensor_data_t *data, char *output, size_t size);
static int send_data_to_server(const char *host, unsigned short port, const char *data);
static bool run_sensor_monitoring_loop(const char *host, unsigned short port);

/**
 * @brief 初始化硬件设备
 */
static void init_hardware(void)
{
    // 初始化GPIO
    GpioInit();
    
    // 初始化OLED显示屏
    OledInit();
    OledFillScreen(0x00);
    
    // 初始化I2C用于温湿度传感器
    I2cInit(AHT20_I2C_IDX, AHT20_BAUDRATE);

    // 初始化蜂鸣器PWM
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_9, WIFI_IOT_IO_FUNC_GPIO_9_PWM0_OUT);
    PwmInit(WIFI_IOT_PWM_PORT_PWM0);

    // 初始化温湿度传感器
    while (WIFI_IOT_SUCCESS != AHT20_Calibrate()) {
        printf("AHT20 sensor initialization failed, retrying...\n");
        usleep(2000);
    }
    printf("Hardware initialization completed\n");
}

/**
 * @brief 读取传感器数据
 * 
 * @param data 传感器数据结构体指针
 * @return int 成功返回0，失败返回-1
 */
static int read_sensor_data(sensor_data_t *data)
{
    if (data == NULL) {
        printf("Error: sensor data pointer is NULL\n");
        return -1;
    }

    // 开始温湿度测量
    if (AHT20_StartMeasure() != WIFI_IOT_SUCCESS) {
        printf("AHT20 measurement start failed\n");
        return -1;
    }

    // 获取温湿度测量结果
    if (AHT20_GetMeasureResult(&data->temperature, &data->humidity) != WIFI_IOT_SUCCESS) {
        printf("AHT20 get measurement result failed\n");
        return -1;
    }

    return 0;
}

/**
 * @brief 在OLED上显示传感器数据
 * 
 * @param data 传感器数据结构体指针
 */
static void display_sensor_data(const sensor_data_t *data)
{
    char line[DISPLAY_LINE_SIZE] = {0};
    
    if (data == NULL) {
        printf("Error: sensor data pointer is NULL\n");
        return;
    }

    // 显示标题
    OledShowString(0, 0, "Sensor values:", 1);

    // 显示温度
    snprintf(line, sizeof(line), "Temp: %.2f C", data->temperature);
    OledShowString(0, 1, line, 1);

    // 显示湿度
    snprintf(line, sizeof(line), "Humi: %.2f %%", data->humidity);
    OledShowString(0, 2, line, 1);
}

/**
 * @brief 检查报警条件并触发蜂鸣器
 * 
 * @param data 传感器数据结构体指针
 */
static void check_alarm_conditions(const sensor_data_t *data)
{
    if (data == NULL) {
        printf("Error: sensor data pointer is NULL\n");
        return;
    }
    
    if (data->temperature > TEMPERATURE_THRESHOLD) {
        printf("ALARM: Temperature=%.2f°C - Triggering buzzer\n", data->temperature);
        
        // 启动蜂鸣器
        PwmStart(WIFI_IOT_PWM_PORT_PWM0, BUZZER_FREQ_DIVISOR / 2, BUZZER_FREQ_DIVISOR);
        usleep(BUZZER_DURATION_MS);
        PwmStop(WIFI_IOT_PWM_PORT_PWM0);
        usleep(DETECTION_INTERVAL_MS);
    } else {
        PwmStop(WIFI_IOT_PWM_PORT_PWM0);
    }
}

/**
 * @brief 格式化传感器数据为字符串
 * 
 * @param data 传感器数据结构体指针
 * @param output 输出字符串缓冲区
 * @param size 缓冲区大小
 */
static void format_sensor_data_string(const sensor_data_t *data, char *output, size_t size)
{
    if (data == NULL || output == NULL) {
        printf("Error: invalid parameters for data formatting\n");
        return;
    }

    snprintf(output, size, "%.2f,%.2f", data->temperature, data->humidity);
}

/**
 * @brief 发送数据到TCP服务器
 * 
 * @param host 服务器IP地址
 * @param port 服务器端口号
 * @param data 要发送的数据
 * @return int 成功返回0，失败返回-1
 */
static int send_data_to_server(const char *host, unsigned short port, const char *data)
{
    if (host == NULL || data == NULL) {
        printf("Error: invalid parameters for server connection\n");
        return -1;
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("Socket creation failed\n");
        return -1;
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, host, &server_addr.sin_addr) <= 0) {
        printf("Invalid IP address format\n");
        closesocket(sockfd);
        return -1;
    }

    // 连接到服务器
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        printf("Connection to server %s:%d failed\n", host, port);
        closesocket(sockfd);
        return -1;
    }
    
    printf("Connected to server %s:%d\n", host, port);

    // 发送数据
    ssize_t sent_bytes = send(sockfd, data, strlen(data), 0);
    if (sent_bytes < 0) {
        printf("Failed to send data to server\n");
        closesocket(sockfd);
        return -1;
    }
    
    printf("Sent data {%s} (%zd bytes) to server\n", data, sent_bytes);

    // 接收响应
    ssize_t recv_bytes = recv(sockfd, response, sizeof(response) - 1, 0);
    if (recv_bytes > 0) {
        response[recv_bytes] = '\0';
        printf("Received response {%s} (%zd bytes) from server\n", response, recv_bytes);
    } else if (recv_bytes == 0) {
        printf("Server closed connection\n");
    } else {
        printf("Failed to receive response from server\n");
    }

    closesocket(sockfd);
    return 0;
}

/**
 * @brief 运行传感器监控循环
 * 
 * @param host 服务器IP地址
 * @param port 服务器端口号
 * @return bool 成功返回true，失败返回false
 */
static bool run_sensor_monitoring_loop(const char *host, unsigned short port)
{
    sensor_data_t sensor_data = {0};
    char data_string[LINESTR_BUFFER_SIZE] = {0};
    int consecutive_failures = 0;
    const int max_failures = 5; // 最大连续失败次数

    printf("Starting sensor monitoring loop for %s:%d\n", host, port);

    while (1) {
        // 读取传感器数据
        if (read_sensor_data(&sensor_data) != 0) {
            consecutive_failures++;
            printf("Failed to read sensor data (failure count: %d/%d)\n", 
                   consecutive_failures, max_failures);
            
            if (consecutive_failures >= max_failures) {
                printf("ERROR: Too many consecutive sensor reading failures\n");
                return false;
            }
            
            sleep(SENSOR_READ_INTERVAL_S);
            continue;
        }

        // 重置失败计数器
        consecutive_failures = 0;

        // 在OLED上显示传感器数据
        display_sensor_data(&sensor_data);

        // 检查报警条件
        check_alarm_conditions(&sensor_data);

        // 格式化数据字符串
        format_sensor_data_string(&sensor_data, data_string, sizeof(data_string));
        printf("Sensor data: %s\n", data_string);

        // 发送数据到服务器
        if (send_data_to_server(host, port, data_string) != 0) {
            printf("Failed to send data to server, will retry next cycle\n");
            // 网络失败不中断循环，继续尝试
        }

        // 等待下一次采集
        sleep(SENSOR_READ_INTERVAL_S);
    }

    return true; // 理论上不会到达这里
}

// 网络传输
/**
 * @brief 连接到TCP服务器并发送传感器数据
 * 
 * @param host 服务器IP地址
 * @param port 服务器端口号
 * @return bool 成功返回true，失败返回false
 */
bool conent_tcp_server(const char *host, unsigned short port)
{
    if (host == NULL) {
        printf("ERROR: Host parameter is NULL\n");
        return false;
    }

    if (port == 0) {
        printf("ERROR: Invalid port number\n");
        return false;
    }

    printf("Initializing TCP client for %s:%d\n", host, port);

    // 初始化硬件设备
    printf("Initializing hardware devices...\n");
    init_hardware();
    printf("Hardware initialization completed\n");

    // 运行传感器监控循环
    return run_sensor_monitoring_loop(host, port);
}




