/*
 * @Author: vdavidyang vdavidyang@gmail.com
 * @Date: 2025-05-02 13:00:49
 * @LastEditors: vdavidyang vdavidyang@gmail.com
 * @LastEditTime: 2025-05-02 13:03:43
 * @FilePath: \GameOptimizerPro\src\config\power_plan.cpp
 * @Description:
 * Copyright (c) 2025 by vdavidyang vdavidyang@gmail.com, All Rights Reserved.
 */
#include "config/power_plan.h"

PowerPlan::PowerPlan()
{
  optimizePowerPlan = false;
  lockPowerPlan = false;
  powerPlanGuid = "";
}

PowerPlan::~PowerPlan()
{
  clear();
}

void PowerPlan::clear()
{
  optimizePowerPlan = false;
  lockPowerPlan = false;
  powerPlanGuid = "";
}

PowerPlan &PowerPlan::operator=(const PowerPlan &other)
{
  if (this != &other)
  {
    optimizePowerPlan = other.optimizePowerPlan;
    lockPowerPlan = other.lockPowerPlan;
    powerPlanGuid = other.powerPlanGuid;
  }
  return *this;
}

PowerPlan &PowerPlan::operator=(PowerPlan &&other) noexcept
{
  if (this != &other)
  {
    optimizePowerPlan = std::move(other.optimizePowerPlan);
    lockPowerPlan = std::move(other.lockPowerPlan);
    powerPlanGuid = std::move(other.powerPlanGuid);
  }
  return *this;
}

bool PowerPlan::operator==(const PowerPlan &other) const
{
  return optimizePowerPlan == other.optimizePowerPlan &&
         lockPowerPlan == other.lockPowerPlan &&
         powerPlanGuid == other.powerPlanGuid;
}

bool PowerPlan::operator!=(const PowerPlan &other) const
{
  return !(*this == other);
}

std::string PowerPlan::toString() const
{
  return "optimizePowerPlan: " + std::to_string(optimizePowerPlan) +
         " lockPowerPlan: " + std::to_string(lockPowerPlan) +
         " powerPlanGuid: " + powerPlanGuid;
}

void PowerPlan::fromJson(const nlohmann::json &json)
{
  if (json.contains("optimizePowerPlan"))
    optimizePowerPlan = json["optimizePowerPlan"];
  if (json.contains("lockPowerPlan"))
    lockPowerPlan = json["lockPowerPlan"];
  if (json.contains("powerPlanGuid"))
    powerPlanGuid = json["powerPlanGuid"];
}

nlohmann::json PowerPlan::toJson() const
{
  nlohmann::json json;
  json["optimizePowerPlan"] = optimizePowerPlan;
  json["lockPowerPlan"] = lockPowerPlan;
  json["powerPlanGuid"] = powerPlanGuid;
  return json;
}
