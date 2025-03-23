#pragma once
#include <string>
#include <windows.h>

/**
 * @brief 路由条目结构
 * @details 包含了一个路由条目所需的所有信息
 */
struct RouteEntry
{
  std::string destination; ///< 目标网络地址
  std::string mask;        ///< 子网掩码
  std::string gateway;     ///< 网关地址
  DWORD metric;            ///< 路由跃点数（度量值）
  DWORD ifIndex;           ///< 网络接口索引
};

/**
 * @brief 默认网关信息结构
 * @details 存储系统默认网关的相关信息
 */
struct DefaultGatewayInfo
{
  DWORD ifIndex;       ///< 默认网关使用的接口索引
  std::string gateway; ///< 默认网关地址
  DWORD metric;        ///< 默认路由的度量值
  bool valid;          ///< 标识信息是否有效
};