/*
 * @Author: vdavidyang vdavidyang@gmail.com
 * @Date: 2025-04-25 17:23:02
 * @LastEditors: vdavidyang vdavidyang@gmail.com
 * @LastEditTime: 2025-05-10 23:00:16
 * @FilePath: \GameOptimizerPro\include\config\app_config.h
 * @Description:
 * Copyright (c) 2025 by vdavidyang vdavidyang@gmail.com, All Rights Reserved.
 */

#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

#include "config/system_info.h"
#include "config/optimism_config.h"
#include "config/process_config.h"

/**
 * @class AppConfig
 * @brief 应用程序配置类，负责存储应用程序的配置，包括版本、系统信息、优化配置和进程信息等
 * @note 该类使用 nlohmann::json 库来处理 JSON 数据
 */
class AppConfig
{
public:
  std::string version;
  SystemInfo systemInfo;
  OptimismConfig optimismConfig;
  ProcessConfig processConfig;

  // 构造函数
  AppConfig();

  // 析构函数
  ~AppConfig();

  // 拷贝构造函数
  AppConfig(const AppConfig &other);

  // 移动构造函数
  AppConfig(AppConfig &&other) noexcept;

  // 赋值运算符
  AppConfig &operator=(const AppConfig &other);

  // 移动赋值运算符
  AppConfig &operator=(AppConfig &&other) noexcept;

  // 比较运算符
  bool operator==(const AppConfig &other) const;
  bool operator!=(const AppConfig &other) const;

  // 转换为 JSON 对象
  // nlohmann::json toJson() const;

  // 从 JSON 对象转换
  // void fromJson(const nlohmann::json &json);

  // 转换为字符串
  std::string toString() const;

  // 清空配置
  void clear();
};
