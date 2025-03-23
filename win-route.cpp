#include <winsock2.h>
#include <windows.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

/**
 * @brief 路由条目结构
 * @details 包含了一个路由条目所需的所有信息
 */
struct RouteEntry {
    std::string destination;  ///< 目标网络地址
    std::string mask;        ///< 子网掩码
    std::string gateway;     ///< 网关地址
    DWORD metric;           ///< 路由跃点数（度量值）
    DWORD ifIndex;          ///< 网络接口索引
};

/**
 * @brief 默认网关信息结构
 * @details 存储系统默认网关的相关信息
 */
struct DefaultGatewayInfo {
    DWORD ifIndex;          ///< 默认网关使用的接口索引
    std::string gateway;     ///< 默认网关地址
    DWORD metric;           ///< 默认路由的度量值
    bool valid;             ///< 标识信息是否有效
};

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
/**
 * @brief 重置路由表
 * @details 删除所有非默认路由：
 *          1. 保留目标地址为0.0.0.0的默认路由
 *          2. 删除其他所有路由
 *          3. 采用批量删除提高性能
 *          4. 提供删除统计信息
 */
void ResetRoutes();

/**
 * @brief 合并多个文件中的路由条目
 * @param filenames 路由文件名列表
 * @return 合并后的路由条目列表
 * @details 1. 依次读取每个文件中的路由
 *          2. 跳过无效的路由文件
 *          3. 提供每个文件的加载统计信息
 */
std::vector<RouteEntry> MergeRoutes(const std::vector<std::string> &filenames);

/**
 * @brief 打印程序使用帮助信息
 * @details 显示：
 *          1. 支持的命令和参数格式
 *          2. 文件格式示例
 *          3. 基本使用说明
 */
void PrintUsage();

/**
 * @brief 批量添加路由条目
 * @param routes 要添加的路由条目列表
 * @param gateway 网关地址
 * @param ifIndex 网络接口索引
 * @param metric 跃点数
 * @return true表示所有路由添加成功，false表示存在添加失败的路由
 * @details 采用批量处理方式提高性能：
 *          1. 预分配内存减少重新分配
 *          2. 集中错误处理减少系统调用
 *          3. 提供操作汇总信息
 */
bool BatchAddRoutes(const std::vector<RouteEntry> &routes, const std::string &gateway, DWORD ifIndex, DWORD metric);

/**
 * @brief 批量删除路由条目
 * @param routes 要删除的路由条目列表
 * @return true表示至少成功删除一个路由，false表示全部删除失败
 * @details 采用批量处理方式提高性能：
 *          1. 只获取一次路由表
 *          2. 预分配内存减少重新分配
 *          3. 集中错误处理
 *          4. 提供删除操作的统计信息
 */
bool DeleteRoutes(const std::vector<RouteEntry> &routes);

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
DWORD IpStringToDword(const std::string& ipAddress);

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
bool ParseCidr(const std::string& cidr, std::string& ip, std::string& mask);

/**
 * @brief 从文件读取路由条目
 * @param filename 路由文件路径
 * @return vector<RouteEntry> 读取到的路由条目列表
 * @details 1. 逐行读取文件内容
 *          2. 跳过空行和注释行(#开头)
 *          3. 解析CIDR格式的路由
 *          4. 设置默认网关为0.0.0.0(后续会被实际网关替换)
 */
std::vector<RouteEntry> ReadRoutesFromFile(const std::string& filename);

DefaultGatewayInfo GetDefaultGateway()
{
  DefaultGatewayInfo info = {0, "", 0, false};
  PMIB_IPFORWARDTABLE pIpForwardTable = NULL;
  DWORD dwSize = 0;
  DWORD dwRetVal = 0;

  if (GetIpForwardTable(NULL, &dwSize, TRUE) == ERROR_INSUFFICIENT_BUFFER)
  {
    pIpForwardTable = (PMIB_IPFORWARDTABLE)malloc(dwSize);
    if (pIpForwardTable == NULL)
    {
      return info;
    }
  }

  if ((dwRetVal = GetIpForwardTable(pIpForwardTable, &dwSize, TRUE)) == NO_ERROR)
  {
    for (DWORD i = 0; i < pIpForwardTable->dwNumEntries; i++)
    {
      if (pIpForwardTable->table[i].dwForwardDest == 0)
      {
        info.ifIndex = pIpForwardTable->table[i].dwForwardIfIndex;
        info.metric = pIpForwardTable->table[i].dwForwardMetric1;
        struct in_addr addr;
        addr.s_addr = pIpForwardTable->table[i].dwForwardNextHop;
        info.gateway = inet_ntoa(addr);
        info.valid = true;
        break;
      }
    }
  }

  if (pIpForwardTable)
  {
    free(pIpForwardTable);
  }

  return info;
}

std::string GetInterfaceIpAddress(DWORD ifIndex)
{
  PIP_ADAPTER_ADDRESSES pAddresses = NULL;
  ULONG outBufLen = 0;
  DWORD dwRetVal = 0;

  // 首先获取需要的缓冲区大小
  if (GetAdaptersAddresses(AF_INET,
                           GAA_FLAG_INCLUDE_PREFIX,
                           NULL,
                           NULL,
                           &outBufLen) == ERROR_BUFFER_OVERFLOW)
  {
    pAddresses = (PIP_ADAPTER_ADDRESSES)malloc(outBufLen);
  }

  if (pAddresses == NULL)
  {
    return "";
  }

  // 获取适配器信息
  if (GetAdaptersAddresses(AF_INET,
                           GAA_FLAG_INCLUDE_PREFIX,
                           NULL,
                           pAddresses,
                           &outBufLen) == NO_ERROR)
  {

    PIP_ADAPTER_ADDRESSES pCurrAddresses = pAddresses;
    while (pCurrAddresses)
    {
      if (pCurrAddresses->IfIndex == ifIndex)
      {
        PIP_ADAPTER_UNICAST_ADDRESS pUnicast = pCurrAddresses->FirstUnicastAddress;
        if (pUnicast != NULL)
        {
          SOCKADDR_IN *pSockAddr = (SOCKADDR_IN *)pUnicast->Address.lpSockaddr;
          char ip[INET_ADDRSTRLEN];
          inet_ntop(AF_INET, &(pSockAddr->sin_addr), ip, INET_ADDRSTRLEN);
          free(pAddresses);
          return std::string(ip);
        }
      }
      pCurrAddresses = pCurrAddresses->Next;
    }
  }

  if (pAddresses)
  {
    free(pAddresses);
  }

  return "";
}

DWORD IpStringToDword(const std::string &ipAddress)
{
  return inet_addr(ipAddress.c_str());
}

// 解析CIDR格式的IP地址
bool ParseCidr(const std::string &cidr, std::string &ip, std::string &mask)
{
  size_t pos = cidr.find('/');
  if (pos == std::string::npos)
    return false;

  ip = cidr.substr(0, pos);
  int bits = std::stoi(cidr.substr(pos + 1));

  if (bits < 0 || bits > 32)
    return false;

  DWORD maskValue = (bits == 0) ? 0 : (0xFFFFFFFF << (32 - bits));
  struct in_addr addr;
  addr.s_addr = htonl(maskValue);
  mask = inet_ntoa(addr);

  return true;
}

std::vector<RouteEntry> ReadRoutesFromFile(const std::string &filename)
{
  std::vector<RouteEntry> routes;
  std::ifstream file(filename);
  std::string line;

  while (std::getline(file, line))
  {
    // 跳过空行
    if (line.empty() || line[0] == '#')
    {
      continue;
    }

    // 去除行尾可能的回车符
    if (!line.empty() && line[line.length() - 1] == '\r')
    {
      line = line.substr(0, line.length() - 1);
    }

    RouteEntry entry;
    // 只解析 CIDR，网关将在后续设置
    if (ParseCidr(line, entry.destination, entry.mask))
    {
      entry.gateway = "0.0.0.0"; // 默认设置为 0.0.0.0，后续会被实际网关替换
      entry.metric = 1;          // 默认跳数
      routes.push_back(entry);
    }
    else
    {
      std::cout << "Invalid CIDR format: " << line << "\n";
    }
  }

  return routes;
}

bool AddRoute(const RouteEntry &entry)
{
  MIB_IPFORWARDROW row = {0};
  row.dwForwardDest = IpStringToDword(entry.destination);
  row.dwForwardMask = IpStringToDword(entry.mask);
  row.dwForwardNextHop = IpStringToDword(entry.gateway);
  row.dwForwardMetric1 = entry.metric;
  row.dwForwardIfIndex = entry.ifIndex; // 使用指定的接口索引
  row.dwForwardType = MIB_IPROUTE_TYPE_INDIRECT;
  row.dwForwardProto = MIB_IPPROTO_NETMGMT;
  row.dwForwardAge = 0;

  DWORD result = CreateIpForwardEntry(&row);
  if (result != NO_ERROR)
  {
    // 添加错误信息输出
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        result,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf,
        0,
        NULL);
    std::cout << "Error: " << (char *)lpMsgBuf << std::endl;
    LocalFree(lpMsgBuf);
  }
  return (result == NO_ERROR);
}

bool DeleteRoute(const RouteEntry &entry)
{
  // 首先获取现有路由的信息
  PMIB_IPFORWARDTABLE pIpForwardTable = NULL;
  DWORD dwSize = 0;
  MIB_IPFORWARDROW row = {0};
  bool found = false;

  // 获取路由表大小
  if (GetIpForwardTable(NULL, &dwSize, TRUE) == ERROR_INSUFFICIENT_BUFFER)
  {
    pIpForwardTable = (PMIB_IPFORWARDTABLE)malloc(dwSize);
    if (pIpForwardTable == NULL)
    {
      return false;
    }
  }

  // 获取路由表
  if (GetIpForwardTable(pIpForwardTable, &dwSize, TRUE) == NO_ERROR)
  {
    DWORD destIp = IpStringToDword(entry.destination);
    DWORD maskIp = IpStringToDword(entry.mask);

    // 查找匹配的路由
    for (DWORD i = 0; i < pIpForwardTable->dwNumEntries; i++)
    {
      if (pIpForwardTable->table[i].dwForwardDest == destIp &&
          pIpForwardTable->table[i].dwForwardMask == maskIp)
      {
        row = pIpForwardTable->table[i];
        found = true;
        break;
      }
    }
  }

  if (pIpForwardTable)
  {
    free(pIpForwardTable);
  }

  // 若找到匹配的路由，则删除
  if (found)
  {
    DWORD result = DeleteIpForwardEntry(&row);
    if (result != NO_ERROR)
    {
      LPVOID lpMsgBuf;
      FormatMessage(
          FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
          NULL,
          result,
          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
          (LPTSTR)&lpMsgBuf,
          0,
          NULL);
      std::cout << "Error: " << (char *)lpMsgBuf;
      LocalFree(lpMsgBuf);
      return false;
    }
    return true;
  }

  return false;
}

void ResetRoutes()
{
  // 首先获取所有路由
  PMIB_IPFORWARDTABLE pIpForwardTable = NULL;
  DWORD dwSize = 0;
  std::vector<MIB_IPFORWARDROW> rowsToDelete;

  // 获取路由表大小
  if (GetIpForwardTable(NULL, &dwSize, TRUE) == ERROR_INSUFFICIENT_BUFFER)
  {
    pIpForwardTable = (PMIB_IPFORWARDTABLE)malloc(dwSize);
    if (pIpForwardTable == NULL)
    {
      std::cout << "Failed to allocate memory for routing table.\n";
      return;
    }
  }

  // 获取路由表并找出要删除的路由
  if (GetIpForwardTable(pIpForwardTable, &dwSize, TRUE) == NO_ERROR)
  {
    for (DWORD i = 0; i < pIpForwardTable->dwNumEntries; i++)
    {
      // 跳过默认路由（目标地址为0.0.0.0的路由）
      if (pIpForwardTable->table[i].dwForwardDest != 0)
      {
        rowsToDelete.push_back(pIpForwardTable->table[i]);
      }
    }
  }

  // 批量删除路由
  int totalDeleted = 0;
  for (const auto &row : rowsToDelete)
  {
    MIB_IPFORWARDROW mutableRow = row;
    if (DeleteIpForwardEntry(&mutableRow) == NO_ERROR)
    {
      totalDeleted++;
    }
  }

  if (pIpForwardTable)
  {
    free(pIpForwardTable);
  }

  std::cout << "Reset completed. Deleted " << totalDeleted << " routes.\n";
}

std::vector<RouteEntry> MergeRoutes(const std::vector<std::string> &filenames)
{
  std::vector<RouteEntry> allRoutes;

  for (const auto &filename : filenames)
  {
    std::vector<RouteEntry> routes = ReadRoutesFromFile(filename);
    if (routes.empty())
    {
      std::cout << "Warning: No valid routes found in file: " << filename << "\n";
      continue;
    }
    std::cout << "Loaded " << routes.size() << " routes from " << filename << "\n";
    allRoutes.insert(allRoutes.end(), routes.begin(), routes.end());
  }

  return allRoutes;
}

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

bool BatchAddRoutes(const std::vector<RouteEntry> &routes, const std::string &gateway, DWORD ifIndex, DWORD metric)
{
  std::vector<MIB_IPFORWARDROW> rows;
  rows.reserve(routes.size()); // 预分配内存

  // 首先准备所有路由条目
  for (const auto &route : routes)
  {
    MIB_IPFORWARDROW row = {0};
    row.dwForwardDest = IpStringToDword(route.destination);
    row.dwForwardMask = IpStringToDword(route.mask);
    row.dwForwardNextHop = IpStringToDword(gateway);
    row.dwForwardMetric1 = metric;
    row.dwForwardIfIndex = ifIndex;
    row.dwForwardType = MIB_IPROUTE_TYPE_INDIRECT;
    row.dwForwardProto = MIB_IPPROTO_NETMGMT;
    row.dwForwardAge = 0;
    rows.push_back(row);
  }

  // 批量添加路由
  int succeeded = 0;
  int failed = 0;
  DWORD lastError = NO_ERROR;
  LPVOID errorMsg = nullptr;

  for (const auto &row : rows)
  {
    MIB_IPFORWARDROW mutableRow = row;
    DWORD result = CreateIpForwardEntry(&mutableRow);
    if (result == NO_ERROR)
    {
      succeeded++;
    }
    else
    {
      failed++;
      lastError = result; // 只保存最后一个错误
    }
  }

  // 只在发生错误时获取一次错误信息
  if (failed > 0)
  {
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        lastError,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&errorMsg,
        0,
        NULL);
    std::cout << "Some routes failed to add. Last error: " << (char *)errorMsg << std::endl;
    LocalFree(errorMsg);
  }

  std::cout << "\nRoute Addition Summary:\n"
            << "Total routes: " << routes.size() << "\n"
            << "Successfully added: " << succeeded << "\n"
            << "Failed: " << failed << "\n";

  return failed == 0;
}

bool AddRoutes(const std::vector<RouteEntry> &routes, const std::string &gateway, DWORD ifIndex, DWORD metric)
{
  return BatchAddRoutes(routes, gateway, ifIndex, metric);
}

bool RouteExists(const RouteEntry &entry)
{
  PMIB_IPFORWARDTABLE pIpForwardTable = NULL;
  DWORD dwSize = 0;
  bool exists = false;

  if (GetIpForwardTable(NULL, &dwSize, TRUE) == ERROR_INSUFFICIENT_BUFFER)
  {
    pIpForwardTable = (PMIB_IPFORWARDTABLE)malloc(dwSize);
    if (pIpForwardTable == NULL)
    {
      return false;
    }
  }

  if (GetIpForwardTable(pIpForwardTable, &dwSize, TRUE) == NO_ERROR)
  {
    DWORD destIp = IpStringToDword(entry.destination);
    DWORD maskIp = IpStringToDword(entry.mask);

    for (DWORD i = 0; i < pIpForwardTable->dwNumEntries; i++)
    {
      if (pIpForwardTable->table[i].dwForwardDest == destIp &&
          pIpForwardTable->table[i].dwForwardMask == maskIp)
      {
        exists = true;
        break;
      }
    }
  }

  if (pIpForwardTable)
  {
    free(pIpForwardTable);
  }

  return exists;
}

bool DeleteRoutes(const std::vector<RouteEntry> &routes)
{
  // 首先获取一次路由表
  PMIB_IPFORWARDTABLE pIpForwardTable = NULL;
  DWORD dwSize = 0;
  DWORD lastError = NO_ERROR;
  int deleted = 0, notFound = 0;
  std::vector<MIB_IPFORWARDROW> rowsToDelete;
  rowsToDelete.reserve(routes.size()); // 预分配内存

  // 获取路由表大小
  if (GetIpForwardTable(NULL, &dwSize, TRUE) == ERROR_INSUFFICIENT_BUFFER)
  {
    pIpForwardTable = (PMIB_IPFORWARDTABLE)malloc(dwSize);
    if (pIpForwardTable == NULL)
    {
      return false;
    }
  }

  // 获取路由表
  if (GetIpForwardTable(pIpForwardTable, &dwSize, TRUE) == NO_ERROR)
  {
    // 遍历要删除的路由
    for (const auto &route : routes)
    {
      DWORD destIp = IpStringToDword(route.destination);
      DWORD maskIp = IpStringToDword(route.mask);

      // 在路由表中查找匹配项
      for (DWORD i = 0; i < pIpForwardTable->dwNumEntries; i++)
      {
        if (pIpForwardTable->table[i].dwForwardDest == destIp &&
            pIpForwardTable->table[i].dwForwardMask == maskIp)
        {
          rowsToDelete.push_back(pIpForwardTable->table[i]);
          break;
        }
      }
    }
  }

  if (pIpForwardTable)
  {
    free(pIpForwardTable);
  }

  // 批量删除找到的路由
  LPVOID errorMsg = nullptr;
  for (const auto &row : rowsToDelete)
  {
    MIB_IPFORWARDROW mutableRow = row;
    DWORD result = DeleteIpForwardEntry(&mutableRow);
    if (result == NO_ERROR)
    {
      deleted++;
    }
    else
    {
      notFound++;
      lastError = result; // 只留最后一个错误
    }
  }

  // 只在发生错误时获取一次错误信息
  if (notFound > 0)
  {
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        lastError,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&errorMsg,
        0,
        NULL);
    std::cout << "Some routes failed to delete. Last error: " << (char *)errorMsg << std::endl;
    LocalFree(errorMsg);
  }

  std::cout << "\nRoute Deletion Summary:\n"
            << "Total routes: " << routes.size() << "\n"
            << "Found and deleted: " << deleted << "\n"
            << "Not found or failed: " << notFound << "\n";

  return deleted > 0; // 如果至少删除了一个路由就返回成功
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

bool GetExistingRouteInfo(const std::string &destination, const std::string &mask, RouteEntry &entry)
{
  PMIB_IPFORWARDTABLE pIpForwardTable = NULL;
  DWORD dwSize = 0;
  bool found = false;

  if (GetIpForwardTable(NULL, &dwSize, TRUE) == ERROR_INSUFFICIENT_BUFFER)
  {
    pIpForwardTable = (PMIB_IPFORWARDTABLE)malloc(dwSize);
    if (pIpForwardTable == NULL)
    {
      return false;
    }
  }

  if (GetIpForwardTable(pIpForwardTable, &dwSize, TRUE) == NO_ERROR)
  {
    DWORD destIp = IpStringToDword(destination);
    DWORD maskIp = IpStringToDword(mask);

    for (DWORD i = 0; i < pIpForwardTable->dwNumEntries; i++)
    {
      if (pIpForwardTable->table[i].dwForwardDest == destIp &&
          pIpForwardTable->table[i].dwForwardMask == maskIp)
      {
        entry.ifIndex = pIpForwardTable->table[i].dwForwardIfIndex;
        struct in_addr addr;
        addr.s_addr = pIpForwardTable->table[i].dwForwardNextHop;
        entry.gateway = inet_ntoa(addr);
        found = true;
        break;
      }
    }
  }

  if (pIpForwardTable)
  {
    free(pIpForwardTable);
  }

  return found;
}
