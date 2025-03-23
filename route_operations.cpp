#include <winsock2.h>
#include <windows.h>
#include <iphlpapi.h>
#include <iostream>
#include "network_utils.h"
#include "route_operations.h"

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
