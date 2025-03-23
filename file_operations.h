#pragma once
#include "types.h"
#include <vector>
#include <string>

/**
 * @brief 从文件读取路由条目
 * @param filename 路由文件路径
 * @return vector<RouteEntry> 读取到的路由条目列表
 * @details 1. 逐行读取文件内容
 *          2. 跳过空行和注释行(#开头)
 *          3. 解析CIDR格式的路由
 *          4. 设置默认网关为0.0.0.0(后续会被实际网关替换)
 */
std::vector<RouteEntry> ReadRoutesFromFile(const std::string &filename);

/**
 * @brief 合并多个文件中的路由条目
 * @param filenames 路由文件名列表
 * @return 合并后的路由条目列表
 * @details 1. 依次读取每个文件中的路由
 *          2. 跳过无效的路由文件
 *          3. 提供每个文件的加载统计信息
 */
std::vector<RouteEntry> MergeRoutes(const std::vector<std::string> &filenames);
