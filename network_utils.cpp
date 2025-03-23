#include <winsock2.h>
#include <windows.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>
#include "network_utils.h"

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
