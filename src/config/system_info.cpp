/*
 * @Author: vdavidyang vdavidyang@gmail.com
 * @Date: 2025-04-22 17:23:43
 * @LastEditors: vdavidyang vdavidyang@gmail.com
 * @LastEditTime: 2025-05-10 23:10:48
 * @FilePath: \GameOptimizerPro\src\config\system_info.cpp
 * @Description:
 * Copyright (c) 2025 by vdavidyang vdavidyang@gmail.com, All Rights Reserved.
 */
#include "config/system_info.h"

// 构造函数
SystemInfo::SystemInfo()
{
  os = "";
  cpu = "";
  arch = "";
}

// 析构函数
SystemInfo::~SystemInfo()
{
  clear();
}

SystemInfo &SystemInfo::operator=(const SystemInfo &other)
{
  if (this != &other)
  {
    os = other.os;
    cpu = other.cpu;
    arch = other.arch;
  }
  return *this;
}

SystemInfo &SystemInfo::operator=(SystemInfo &&other) noexcept
{
  if (this != &other)
  {
    os = std::move(other.os);
    cpu = std::move(other.cpu);
    arch = std::move(other.arch);
  }
  return *this;
}

bool SystemInfo::operator==(const SystemInfo &other) const
{
  return os == other.os &&
         cpu == other.cpu &&
         arch == other.arch;
}

bool SystemInfo::operator!=(const SystemInfo &other) const
{
  return !(*this == other);
}

std::string SystemInfo::toString() const
{
  std::string result;
  result += "os: " + os + "\n";
  result += "cpu: " + cpu + "\n";
  result += "arch: " + arch + "\n";
  return result;
}

void SystemInfo::fromJson(const nlohmann::json &json)
{
  if (json.contains("os"))
  {
    os = json["os"];
  }

  if (json.contains("cpu"))
  {
    cpu = json["cpu"];
  }

  if (json.contains("arch"))
  {
    arch = json["arch"];
  }
}

nlohmann::json SystemInfo::toJson() const
{
  nlohmann::json json;
  json["os"] = os;
  json["cpu"] = cpu;
  json["arch"] = arch;
  return json;
}

void SystemInfo::clear()
{
  os = "";
  cpu = "";
  arch = "";
}