/*
 * @Author: vdavidyang vdavidyang@gmail.com
 * @Date: 2025-04-25 17:25:40
 * @LastEditors: vdavidyang vdavidyang@gmail.com
 * @LastEditTime: 2025-05-10 23:06:02
 * @FilePath: \GameOptimizerPro\src\config\app_config.cpp
 * @Description:
 * Copyright (c) 2025 by vdavidyang vdavidyang@gmail.com, All Rights Reserved.
 */

#include "config/app_config.h"

AppConfig::AppConfig()
{
}

AppConfig::~AppConfig()
{
}

AppConfig::AppConfig(const AppConfig &other)
{
  version = other.version;
  systemInfo = other.systemInfo;
  optimismConfig = other.optimismConfig;
  processConfig = other.processConfig;
}

AppConfig::AppConfig(AppConfig &&other) noexcept
{
  version = std::move(other.version);
  systemInfo = std::move(other.systemInfo);
  optimismConfig = std::move(other.optimismConfig);
  processConfig = std::move(other.processConfig);
}

AppConfig &AppConfig::operator=(const AppConfig &other)
{
  version = other.version;
  systemInfo = other.systemInfo;
  optimismConfig = other.optimismConfig;
  processConfig = other.processConfig;
  return *this;
}

AppConfig &AppConfig::operator=(AppConfig &&other) noexcept
{
  version = std::move(other.version);
  systemInfo = std::move(other.systemInfo);
  optimismConfig = std::move(other.optimismConfig);
  processConfig = std::move(other.processConfig);
  return *this;
}

bool AppConfig::operator==(const AppConfig &other) const
{
  return version == other.version &&
         systemInfo == other.systemInfo &&
         optimismConfig == other.optimismConfig &&
         processConfig == other.processConfig;
}

bool AppConfig::operator!=(const AppConfig &other) const
{
  return !(*this == other);
}

// TODO: AppConfig 从json文件加载配置
// void AppConfig::fromJson(const nlohmann::json &json)
// {
//   version = json["version"];
//   if (json.contains("systemInfo"))
//   {
//     systemInfo.fromJson(json["systemInfo"]);
//   }
//   if (json.contains("optimismConfig"))
//   {
//     optimismConfig.fromJson(json["optimismConfig"]);
//   }
//   json gameProcessesJson;
//   if (json.contains("gameProcesses"))
//   {
//     for (const auto &process : json["gameProcesses"])
//   }
// }

// TODO: AppConfig 转换为json
// nlohmann::json AppConfig::toJson() const
// {
//   nlohmann::json json;
//   json["version"] = version;
//   json["systemInfo"] = systemInfo.toJson();
//   json["optimismConfig"] = optimismConfig.toJson();
//   json["gameProcesses"] = gameProcesses.toJson();
//   json["antiCheatProcesses"] = antiCheatProcesses.toJson();
//   return json;
// }

// TODO: AppConfig 转换为字符串
// std::string AppConfig::toString() const
// {
//   return toJson().dump();
// }

// 清空配置
void AppConfig::clear()
{
  version.clear();
  systemInfo.clear();
  optimismConfig.clear();
  processConfig.clear();
}
