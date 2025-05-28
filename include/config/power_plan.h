/*
 * @Author: vdavidyang vdavidyang@gmail.com
 * @Date: 2025-05-02 12:57:10
 * @LastEditors: vdavidyang vdavidyang@gmail.com
 * @LastEditTime: 2025-05-04 19:24:23
 * @FilePath: \GameOptimizerPro\include\config\power_plan.h
 * @Description:
 * Copyright (c) 2025 by vdavidyang vdavidyang@gmail.com, All Rights Reserved.
 */

#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

/**
 * @class PowerPlan
 * @brief 电源计划配置类，负责存储和管理电源计划的相关配置
 * @note 该类使用 nlohmann::json 库来处理 JSON 数据
 */
class PowerPlan
{
public:
  PowerPlan();
  ~PowerPlan();

  // 是否优化电源计划
  bool optimizePowerPlan = false;
  // 是否锁定电源计划
  bool lockPowerPlan = false;
  // 电源计划GUID
  std::string powerPlanGuid;

  // 赋值运算符
  PowerPlan &operator=(const PowerPlan &other);

  // 移动赋值运算符
  PowerPlan &operator=(PowerPlan &&other) noexcept;

  // 比较运算符
  bool operator==(const PowerPlan &other) const;
  bool operator!=(const PowerPlan &other) const;
  
  std::string toString() const;
  void fromJson(const nlohmann::json &json);
  nlohmann::json toJson() const;
  void clear();
};
