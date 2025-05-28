/*
 * @Author: vdavidyang vdavidyang@gmail.com
 * @Date: 2025-04-22 16:53:38
 * @LastEditors: vdavidyang vdavidyang@gmail.com
 * @LastEditTime: 2025-05-04 19:22:51
 * @FilePath: \GameOptimizerPro\include\config\optimism_config.h
 * @Description:
 * Copyright (c) 2025 by vdavidyang vdavidyang@gmail.com, All Rights Reserved.
 */
#pragma once

#include <nlohmann/json.hpp>
#include "config/power_plan.h"

/**
 * @class OptimismConfig
 * @brief 优化配置类，负责存储优化相关的配置，包括是否自动启动、限制反作弊、优化电源计划等
 * @note 该类使用 nlohmann::json 库来处理 JSON 数据
 */
class OptimismConfig
{
public:
  // 默认构造函数
  OptimismConfig();
  ~OptimismConfig();

  // 是否自动启动
  bool autoStartUp = false;
  // 是否自动限制反作弊
  bool autoLimitAntiCheat = false;
  // 电源计划
  PowerPlan powerPlan;
  // 是否限制后台活动
  bool limitBackgroundActivity = false;
  // 是否优化网络延迟
  bool optimizeNetworkDelay = false;
  // 是否优化系统调度
  bool optimizeSystemScheduling = false;
  // 是否优化系统服务
  bool optimizeSystemService = false;

  //赋值运算符
  OptimismConfig &operator=(const OptimismConfig &other);

  // 移动赋值运算符
  OptimismConfig &operator=(OptimismConfig &&other) noexcept;

  // 比较运算符
  bool operator==(const OptimismConfig &other) const;
  bool operator!=(const OptimismConfig &other) const;

  nlohmann::json toJson() const;
  void fromJson(const nlohmann::json &json);

  std::string toString() const;
  void clear();
};
