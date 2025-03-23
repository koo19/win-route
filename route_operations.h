#pragma once
#include "types.h"
#include <vector>

/**
 * @brief 添加单个路由条目
 * @param entry 要添加的路由条目
 * @return true表示添加成功，false表示添加失败
 * @details 使用 CreateIpForwardEntry API 添加路由
 *          设置路由参数包括：目标网络、掩码、网关、接口索引、度量值等
 *          失败时会输出详细的错误信息
 */
bool AddRoute(const RouteEntry &entry);

/**
 * @brief 批量添加路由的包装函数
 * @param routes 要添加的路由条目列表
 * @param gateway 网关地址
 * @param ifIndex 网络接口索引
 * @param metric 跃点数
 * @return true表示全部添加成功，false表示存在添加失败的路由
 */
bool AddRoutes(const std::vector<RouteEntry> &routes, const std::string &gateway,
               DWORD ifIndex, DWORD metric);

/**
 * @brief 删除单个路由条目
 * @param entry 要删除的路由条目
 * @return true表示删除成功，false表示删除失败或路由不存在
 * @details 先在路由表中查找匹配的路由条目
 *          如果找到则使用 DeleteIpForwardEntry API 删除
 *          失败时会输出详细的错误信息
 */
bool DeleteRoute(const RouteEntry &entry);

/**
 * @brief 检查路由是否存在
 * @param entry 要检查的路由条目
 * @return true表示路由存在，false表示不存在
 * @details 通过查询系统路由表检查指定的路由是否存在
 *          只匹配目标网络和掩码，不考虑网关和接口
 */
bool RouteExists(const RouteEntry &entry);

/**
 * @brief 获取现有路由的详细信息
 * @param destination 目标网络地址
 * @param mask 网络掩码
 * @param[out] entry 用于存储找到的路由信息
 * @return true表示找到路由，false表示未找到
 * @details 在系统路由表中查找指定的路由
 *          如果找到则填充完整的路由信息，包括接口索引和网关地址
 */
bool GetExistingRouteInfo(const std::string &destination, const std::string &mask,
                          RouteEntry &entry);

bool DeleteRoutes(const std::vector<RouteEntry> &routes);

/**
 * @brief 重置路由表
 * @details 删除所有非默认路由：
 *          1. 保留目标地址为0.0.0.0的默认路由
 *          2. 删除其他所有路由
 *          3. 采用批量删除提高性能
 *          4. 提供删除统计信息
 */
void ResetRoutes();
