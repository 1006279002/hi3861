/**
 * @file tcp_client.h
 * @brief TCP客户端头文件
 * @version 1.0
 * @date 2025-06-27
 */

#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 连接到TCP服务器并发送传感器数据
 * 
 * @param host 服务器IP地址
 * @param port 服务器端口号
 * @return bool 成功返回true，失败返回false
 */
bool conent_tcp_server(const char *host, unsigned short port);

#ifdef __cplusplus
}
#endif

#endif /* TCP_CLIENT_H */

