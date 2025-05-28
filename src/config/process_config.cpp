/*
 * @Author: vdavidyang vdavidyang@gmail.com
 * @Date: 2025-05-10 18:09:58
 * @LastEditors: vdavidyang vdavidyang@gmail.com
 * @LastEditTime: 2025-05-10 23:09:52
 * @FilePath: \GameOptimizerPro\src\config\process_config.cpp
 * @Description:
 * Copyright (c) 2025 by vdavidyang vdavidyang@gmail.com, All Rights Reserved.
 */

#include "config/process_config.h"

ProcessConfig::ProcessConfig()
{
  gameProcessList = {};
  antiCheatProcessList = {};
}

ProcessConfig::~ProcessConfig()
{
  gameProcessList.clear();
  antiCheatProcessList.clear();
}

ProcessConfig &ProcessConfig::operator=(const ProcessConfig &other)
{
  if (this != &other)
  {
    gameProcessList = other.gameProcessList;
    antiCheatProcessList = other.antiCheatProcessList;
  }
  return *this;
}

ProcessConfig &ProcessConfig::operator=(ProcessConfig &&other) noexcept
{
  if (this != &other)
  {
    gameProcessList = std::move(other.gameProcessList);
    antiCheatProcessList = std::move(other.antiCheatProcessList);
  }
  return *this;
}

bool ProcessConfig::operator==(const ProcessConfig &other) const
{
  return gameProcessList == other.gameProcessList && antiCheatProcessList == other.antiCheatProcessList;
}

bool ProcessConfig::operator!=(const ProcessConfig &other) const
{
  return !(*this == other);
}

// TODO: ProcessConfig 转换为 JSON 对象
// nlohmann::json ProcessConfig::toJson() const
// {
//   nlohmann::json json;
//   json["gameProcessList"] = gameProcessList;
//   json["antiCheatProcessList"] = antiCheatProcessList;
//   return json;
// }

// TODO: ProcessConfig 从json文件加载配置
// void ProcessConfig::fromJson(const nlohmann::json &json)
// {
//   if (json.contains("gameProcessList"))
//   {
//     gameProcessList = json["gameProcessList"].get<std::vector<ProcessInfo>>();
//   }
//   if (json.contains("antiCheatProcessList"))
//   {
//     antiCheatProcessList = json["antiCheatProcessList"].get<std::vector<ProcessInfo>>();
//   }
// }

std::string ProcessConfig::toString() const
{
  std::string str = "Game Process List:\n";
  for (const auto &process : gameProcessList)
  {
    str += process.toString() + "\n";
  }
  str += "Anti-Cheat Process List:\n";
  for (const auto &process : antiCheatProcessList)
  {
    str += process.toString() + "\n";
  }
  return str;
}

void ProcessConfig::clear()
{
  gameProcessList.clear();
  antiCheatProcessList.clear();
}
