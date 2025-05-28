/*
 * @Author: vdavidyang vdavidyang@gmail.com
 * @Date: 2025-04-21 20:34:15
 * @LastEditors: vdavidyang vdavidyang@gmail.com
 * @LastEditTime: 2025-05-02 13:16:09
 * @FilePath: \GameOptimizerPro\src\config\process_info.cpp
 * @Description:
 * Copyright (c) 2025 by vdavidyang vdavidyang@gmail.com, All Rights Reserved.
 */
#include "config/process_info.h"

ProcessInfo::ProcessInfo()
{
  name = "";
  status = false;
  processList = {};
}

ProcessInfo::~ProcessInfo()
{
}

ProcessInfo &ProcessInfo::operator=(const ProcessInfo &other)
{
  if (this != &other)
  {
    name = other.name;
    status = other.status;
    processList = other.processList;
  }
  return *this;
}

ProcessInfo &ProcessInfo::operator=(ProcessInfo &&other) noexcept
{
  if (this != &other)
  {
    name = std::move(other.name);
    status = std::move(other.status);
    processList = std::move(other.processList);
  }
  return *this;
}

bool ProcessInfo::operator==(const ProcessInfo &other) const
{
  return name == other.name &&
         processList == other.processList &&
         status == other.status;
}

// 转换为字符串
std::string ProcessInfo::toString() const
{
  std::string result;
  result += "name: " + name + "\n";
  result += "status: " + std::to_string(status) + "\n";
  result += "processList: [";
  for (const auto &process : processList)
  {
    result += process + ", ";
  }
  if (!processList.empty())
  {
    result.pop_back(); // 移除最后一个空格
    result.pop_back(); // 移除最后一个逗号
  }
  result += "]";
  return result;
}

nlohmann::json ProcessInfo::toJson() const
{
  nlohmann::json json;
  json["name"] = name;
  json["status"] = status;
  json["processList"] = processList;
  return json;
}

void ProcessInfo::fromJson(const nlohmann::json &json)
{
  name = json["name"];
  status = json["status"];
  processList = json["processList"];
}

void ProcessInfo::clear()
{
  name = "";
  status = false;
  processList = {};
}
