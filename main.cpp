#include <iostream>
#include "types.h"
#include "route_operations.h"
#include "file_operations.h"
#include "network_utils.h"

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

/**
 * @brief 程序主入口函数
 * @param argc 命令行参数数量
 * @param argv 命令行参数数组
 * @return 0表示成功，1表示失败
 * @details 支持以下命令：
 *          1. add    - 添加路由(需要指定default使用默认网关)
 *          2. delete - 删除路由
 *          3. reset  - 重置路由表(保留默认路由)
 *
 *          用法示例：
 *          win-route add file1.txt file2.txt default
 *          win-route delete file1.txt file2.txt
 *          win-route reset
 */
int main(int argc, char *argv[]);

void PrintUsage()
{
  std::cout << "Usage:\n"
            << "  win-route add <file1.txt> [file2.txt ...] default   - Add routes from files using default gateway\n"
            << "  win-route delete <file1.txt> [file2.txt ...]        - Delete routes from files\n"
            << "  win-route reset                                     - Reset routing table\n"
            << "\nFile format example:\n"
            << "1.0.1.0/24\n"
            << "1.0.2.0/23\n"
            << "1.0.8.0/21\n";
}

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    PrintUsage();
    return 1;
  }

  std::string command = argv[1];

  if (command == "reset")
  {
    ResetRoutes();
    std::cout << "Routing table has been reset.\n";
    return 0;
  }

  // 检查是否至少有一个文件参数
  if (argc < 3)
  {
    PrintUsage();
    return 1;
  }

  // 收集所有文件名
  std::vector<std::string> filenames;
  int lastFileIndex = argc;

  if (command == "add")
  {
    // add 命令需要 default 参数
    if (std::string(argv[argc - 1]) != "default")
    {
      std::cout << "Please specify 'default' to use the system default gateway.\n";
      return 1;
    }
    lastFileIndex = argc - 1; // 排除 default 参数
  }

  // 收集所有文件名（从 argv[2] 到 lastFileIndex-1）
  for (int i = 2; i < lastFileIndex; i++)
  {
    filenames.push_back(argv[i]);
  }

  // 合并所有文件中的路由
  std::vector<RouteEntry> routes = MergeRoutes(filenames);

  if (routes.empty())
  {
    std::cout << "No valid routes found in any of the input files.\n";
    return 1;
  }

  if (command == "add")
  {
    std::string gateway;
    DWORD ifIndex = 0;
    DWORD metric = 1;

    DefaultGatewayInfo defaultInfo = GetDefaultGateway();
    if (!defaultInfo.valid)
    {
      std::cout << "Failed to get default gateway information.\n";
      return 1;
    }
    gateway = defaultInfo.gateway;
    ifIndex = defaultInfo.ifIndex;
    metric = defaultInfo.metric;
    std::cout << "Using default gateway: " << gateway
              << " (ifIndex: " << ifIndex << ")" << "\n"
              << "Total routes to add: " << routes.size() << "\n";

    return AddRoutes(routes, gateway, ifIndex, metric) ? 0 : 1;
  }
  else if (command == "delete")
  {
    std::cout << "Total routes to delete: " << routes.size() << "\n";
    return DeleteRoutes(routes) ? 0 : 1;
  }
  else
  {
    PrintUsage();
    return 1;
  }

  return 0;
}
