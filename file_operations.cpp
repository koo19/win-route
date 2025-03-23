#include "file_operations.h"
#include <fstream>
#include <iostream>
#include "network_utils.h"

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
