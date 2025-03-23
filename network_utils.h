#pragma once
#include "types.h"
#include <string>

/**
 * @brief 获取系统默认网关信息
 * @return DefaultGatewayInfo 包含网关地址、接口索引等信息的结构体
 * @details 通过查询系统路由表获取默认网关信息：
 *          1. 查找目标地址为0.0.0.0的路由
 *          2. 获取该路由的网关地址、接口索引和度量值
 *          3. 返回包含这些信息的结构体
 */
DefaultGatewayInfo GetDefaultGateway();

/**
 * @brief 获取指定网络接口的IP地址
 * @param ifIndex 网络接口索引
 * @return string 接口IP地址，失败返回空字符串
 * @details 通过Windows API获取指定接口的IP地址：
 *          1. 获取系统所有网络适配器信息
 *          2. 查找指定索引的适配器
 *          3. 获取该适配器的第一个单播地址
 */
std::string GetInterfaceIpAddress(DWORD ifIndex);

/**
 * @brief 将IP地址字符串转换为32位整数
 * @param ipAddress IP地址字符串(如"192.168.1.1")
 * @return DWORD 转换后的32位整数
 * @details 使用inet_addr函数将点分十进制IP地址转换为网络字节序的32位整数
 */
DWORD IpStringToDword(const std::string &ipAddress);

/**
 * @brief 解析CIDR格式的IP地址
 * @param cidr CIDR格式的IP地址(如"192.168.1.0/24")
 * @param[out] ip 输出参数，存储解析出的IP地址
 * @param[out] mask 输出参数，存储解析出的子网掩码
 * @return bool 解析成功返回true，失败返回false
 * @details 1. 分离IP地址和掩码位数
 *          2. 验证掩码位数的有效性(0-32)
 *          3. 计算子网掩码
 *          4. 转换为点分十进制格式
 */
bool ParseCidr(const std::string &cidr, std::string &ip, std::string &mask);
