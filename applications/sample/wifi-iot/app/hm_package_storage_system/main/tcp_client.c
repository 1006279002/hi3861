#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "hi_wifi_api.h"
#include "lwip/ip_addr.h"
#include "lwip/netifapi.h"
#include "lwip/sockets.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_pwm.h"
#include "wifiiot_adc.h"
#include "wifiiot_i2c.h"
#include "wifiiot_errno.h"
#include "hi_pwm.h"

#include "oled_ssd1306.h"
#include "aht20.h"

// static char request[]="Hello";
static char response[128]="";

/**
 * @brief TCP client request message
 * 扩展，在检测中加入蜂鸣器，温度高于40或者空气质量高于800时，报警
 * 环境监测，获取环境温度和湿度，空气质量等信息，显示在OLED上，通过网络传输给java端
 */

#define AHT20_BAUDRATE 400*1000 // 400KHz
#define AHT20_I2C_IDX WIFI_IOT_I2C_IDX_0 // 定义湿度传感器使用的I2C索引
#define GAS_SENSOR_CHAN_NAME WIFI_IOT_ADC_CHANNEL_5 // 定义气体传感器使用的ADC通道
#define ADC_RESOLUTION 2048 // ADC分辨率

// 声明全局变量，获取温度、湿度和气体传感器的值，以及显示的字符串信息
uint32_t reval = 0; // 返回值
float temperature = 0.0f; // 温度
float humidity = 0.0f; // 湿度
unsigned short data = 0; // 气体传感器数据
char line[32] = {0}; // 用于存储显示信息

float humidity_temp = 0.0f; // 临时湿度变量
float temperature_temp = 0.0f; // 临时温度变量
short gas_temp = 0; // 临时气体传感器数据变量

char str_humidity[20] = {0}; // 用于存储湿度字符串
char str_temperature[20] = {0}; // 用于存储温度字符串
char str_gas[20] = {0}; // 用于存储气体传感器数据字符串
char linestr[80] = {0}; // 用于存储显示信息字符串

// 传感器数据收集
/**
 * @brief 初始化设备的方法
 */
void init(void){
    GpioInit(); // 初始化GPIO
    OledInit(); // 初始化OLED显示屏
    OledFillScreen(0x00); // 清屏
    I2cInit(AHT20_I2C_IDX, AHT20_BAUDRATE); // 初始化烟雾传感器

    // 蜂鸣器的PWM初始化
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_9, WIFI_IOT_IO_FUNC_GPIO_9_PWM0_OUT);
    PwmInit(WIFI_IOT_PWM_PORT_PWM0); // 初始化PWM

    // 初始化温湿度板卡
    while(WIFI_IOT_SUCCESS != AHT20_Calibrate()){
        printf("AHT20 SENSOR init failed!\r\n");
        usleep(2000);
    }
}

// 网络传输
/**
 * @brief 连接TCP服务器并发送请求
 * @param host 服务器IP地址
 * @param port 服务器端口号
 */
void conent_tcp_server(const char *host,unsigned short port){
    init(); // 初始化设备

    while(1){
        // 传感器数据接收
        if(AHT20_StartMeasure()!=WIFI_IOT_SUCCESS){ // 开始测量温湿度
            printf("AHT20 measure failed!\r\n");
        }
        if(AHT20_GetMeasureResult(&temperature,&humidity)!=WIFI_IOT_SUCCESS){ // 获取温湿度测量结果
            printf("AHT20 get measure result failed!\r\n");
        }
        // 使用ADC函数，获取气体传感器数据
        AdcRead(GAS_SENSOR_CHAN_NAME, &data, WIFI_IOT_ADC_EQU_MODEL_4, WIFI_IOT_ADC_CUR_BAIS_DEFAULT, 0);

        //在OLED上显示温度、湿度和气体传感器数据
        OledShowString(0, 0, "Sensor values: ", 1);

        snprintf(line, sizeof(line), "Temp: %.2f C", temperature);
        OledShowString(0, 1, line, 1); // 显示温度

        snprintf(line, sizeof(line), "Humi: %.2f %%", humidity);
        OledShowString(0, 2, line, 1); // 显示湿度

        snprintf(line, sizeof(line), "Gas: %d", data);
        OledShowString(0, 3, line, 1); // 显示气体传感器数据

        // 接收传感器传递的数值准备传递给服务器
        humidity_temp = humidity; // 将湿度值赋给临时变量
        // 把浮点数转换为字符串
        sprintf(str_humidity, "%.2f", humidity_temp);

        temperature_temp = temperature; // 将温度值赋给临时变量
        // 把浮点数转换为字符串
        sprintf(str_temperature, "%.2f", temperature_temp);

        gas_temp = (short)(data * 100 / ADC_RESOLUTION); // 将气体传感器数据转换为百分比
        // 把整数转换为字符串
        sprintf(str_gas, "%d", gas_temp);

        // 将转换的字符串拼接成一行
        sprintf(linestr, "%s,%s,%s", str_temperature, str_humidity, str_gas); // 接收到的传感器数据转换为字符串准备上传服务器
        printf("linestr: %s\r\n", linestr); // 打印传感器数据字符串

        if(temperature > 40.0f || gas_temp > 800) { // 如果温度大于40摄氏度或者气体传感器数据大于800
            printf("Temperature or gas level is high, triggering alarm!\r\n");
            uint16_t freqDivisor = 34052;
            PwmStart(WIFI_IOT_PWM_PORT_PWM0, freqDivisor/2 , freqDivisor); // 启动蜂鸣器，设置频率分频器和占空比
            usleep(2*1000*100); // 蜂鸣器响2秒
            PwmStop(WIFI_IOT_PWM_PORT_PWM0); // 停止蜂鸣器
            usleep(1*1000*1000); // 等待1秒后再次检测
        } else {
            PwmStop(WIFI_IOT_PWM_PORT_PWM0); // 停止蜂鸣器
        }

        sleep(2); // 每2秒钟获取一次传感器数据

        // 网络连接
        ssize_t retval = 0;
        int sockfd = socket(AF_INET, SOCK_STREAM, 0); // 创建TCP套接字

        struct sockaddr_in serverAddr = {0};
        serverAddr.sin_family = AF_INET; // IPv4
        serverAddr.sin_port = htons(port); // 服务器端口号
        if(inet_pton(AF_INET, host, &serverAddr.sin_addr) <= 0) { // 将IP地址从文本转换为二进制格式
            printf("inet_pton failed\r\n");
            goto do_cleanup;
        }

        // 尝试和目标主机建立连接，成功返回0，失败返回-1
        if(connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
            printf("connect failed\r\n");
            goto do_cleanup;
        }
        printf("Connected to server %s\r\n", host);

        // 建立连接成功之后，sockfd就有了连接状态，由connect参数指定主机和端口
        // retval = send(sockfd, request, sizeof(request), 0);
        retval = send(sockfd, linestr, sizeof(linestr), 0); // 发送请求数据到服务器

        if(retval < 0) {
            printf("send request failed\r\n");
            goto do_cleanup;
        }
        printf("send request {%s} %ld to server done!\r\n", linestr, retval);

        retval = recv(sockfd, response, sizeof(response), 0);
        if(retval < 0) {
            printf("send response from server failed or done,%ld\r\n",retval);
            goto do_cleanup;
        }
        response[retval] = '\0'; // 确保字符串以 null 结尾
        printf("recv response {%s} %ld from server done!\r\n", response,retval);

    do_cleanup:
        printf("do cleanup...\r\n");
        closesocket(sockfd);
    }
}




