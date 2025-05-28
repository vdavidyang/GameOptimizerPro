/*
 * @Author: vdavidyang vdavidyang@gmail.com
 * @Date: 2025-04-22 16:56:32
 * @LastEditors: vdavidyang vdavidyang@gmail.com
 * @LastEditTime: 2025-05-10 23:07:00
 * @FilePath: \GameOptimizerPro\src\config\optimism_config.cpp
 * @Description:
 * Copyright (c) 2025 by vdavidyang vdavidyang@gmail.com, All Rights Reserved.
 */

#include "config/optimism_config.h"

// 构造函数
OptimismConfig::OptimismConfig()
{
  autoStartUp = false;
  autoLimitAntiCheat = false;
  powerPlan.clear();
  limitBackgroundActivity = false;
  optimizeNetworkDelay = false;
  optimizeSystemScheduling = false;
  optimizeSystemService = false;
}

// 析构函数
OptimismConfig::~OptimismConfig()
{
  powerPlan.clear();
}

// 清空配置
void OptimismConfig::clear()
{
  autoStartUp = false;
  autoLimitAntiCheat = false;
  powerPlan.clear();
  limitBackgroundActivity = false;
  optimizeNetworkDelay = false;
  optimizeSystemScheduling = false;
  optimizeSystemService = false;
}

OptimismConfig &OptimismConfig::operator=(const OptimismConfig &other)
{
  if (this != &other)
  {
    autoStartUp = other.autoStartUp;
    autoLimitAntiCheat = other.autoLimitAntiCheat;
    powerPlan = other.powerPlan;
    limitBackgroundActivity = other.limitBackgroundActivity;
    optimizeNetworkDelay = other.optimizeNetworkDelay;
    optimizeSystemScheduling = other.optimizeSystemScheduling;
    optimizeSystemService = other.optimizeSystemService;
  }
  return *this;
}

OptimismConfig &OptimismConfig::operator=(OptimismConfig &&other) noexcept
{
  if (this != &other)
  {
    autoStartUp = std::move(other.autoStartUp);
    autoLimitAntiCheat = std::move(other.autoLimitAntiCheat);
    powerPlan = std::move(other.powerPlan);
    limitBackgroundActivity = std::move(other.limitBackgroundActivity);
    optimizeNetworkDelay = std::move(other.optimizeNetworkDelay);
    optimizeSystemScheduling = std::move(other.optimizeSystemScheduling);
    optimizeSystemService = std::move(other.optimizeSystemService);
  }
  return *this;
}

bool OptimismConfig::operator==(const OptimismConfig &other) const
{
  return autoStartUp == other.autoStartUp &&
         autoLimitAntiCheat == other.autoLimitAntiCheat &&
         powerPlan == other.powerPlan &&
         limitBackgroundActivity == other.limitBackgroundActivity &&
         optimizeNetworkDelay == other.optimizeNetworkDelay &&
         optimizeSystemScheduling == other.optimizeSystemScheduling &&
         optimizeSystemService == other.optimizeSystemService;
}

bool OptimismConfig::operator!=(const OptimismConfig &other) const
{
  return !(*this == other);
}

std::string OptimismConfig::toString() const
{
  std::string result;
  result += "autoStartUp: " + std::to_string(autoStartUp) + "\n";
  result += "autoLimitAntiCheat: " + std::to_string(autoLimitAntiCheat) + "\n";
  result += "powerPlan: " + powerPlan.toString() + "\n";
  result += "limitBackgroundActivity: " + std::to_string(limitBackgroundActivity) + "\n";
  result += "optimizeNetworkDelay: " + std::to_string(optimizeNetworkDelay) + "\n";
  result += "optimizeSystemScheduling: " + std::to_string(optimizeSystemScheduling) + "\n";
  result += "optimizeSystemService: " + std::to_string(optimizeSystemService);
  return result;
}

void OptimismConfig::fromJson(const nlohmann::json &json)
{
  if (json.contains("autoStartUp"))
    autoStartUp = json["autoStartUp"];
  if (json.contains("autoLimitAntiCheat"))
    autoLimitAntiCheat = json["autoLimitAntiCheat"];
  if (json.contains("powerPlan"))
    powerPlan.fromJson(json["powerPlan"]);
  if (json.contains("limitBackgroundActivity"))
    limitBackgroundActivity = json["limitBackgroundActivity"];
  if (json.contains("optimizeNetworkDelay"))
    optimizeNetworkDelay = json["optimizeNetworkDelay"];
  if (json.contains("optimizeSystemScheduling"))
    optimizeSystemScheduling = json["optimizeSystemScheduling"];
  if (json.contains("optimizeSystemService"))
    optimizeSystemService = json["optimizeSystemService"];
}

nlohmann::json OptimismConfig::toJson() const
{
  nlohmann::json json;
  json["autoStartUp"] = autoStartUp;
  json["autoLimitAntiCheat"] = autoLimitAntiCheat;
  json["powerPlan"] = powerPlan.toJson();
  json["limitBackgroundActivity"] = limitBackgroundActivity;
  json["optimizeNetworkDelay"] = optimizeNetworkDelay;
  json["optimizeSystemScheduling"] = optimizeSystemScheduling;
  json["optimizeSystemService"] = optimizeSystemService;
  return json;
}