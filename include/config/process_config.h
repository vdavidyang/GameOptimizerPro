/*
 * @Author: vdavidyang vdavidyang@gmail.com
 * @Date: 2025-05-10 18:06:44
 * @LastEditors: vdavidyang vdavidyang@gmail.com
 * @LastEditTime: 2025-05-10 19:17:11
 * @FilePath: \GameOptimizerPro\include\config\process_config.h
 * @Description:
 * Copyright (c) 2025 by vdavidyang vdavidyang@gmail.com, All Rights Reserved.
 */
#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

#include "config/process_info.h"

class ProcessConfig
{
public:
  ProcessConfig();
  ~ProcessConfig();

  // 游戏进程列表
  std::vector<ProcessInfo> gameProcessList;
  // 反作弊进程列表
  std::vector<ProcessInfo> antiCheatProcessList;

  // 赋值运算符
  ProcessConfig &operator=(const ProcessConfig &other);

  // 移动赋值运算符
  ProcessConfig &operator=(ProcessConfig &&other) noexcept;

  // 比较运算符
  bool operator==(const ProcessConfig &other) const;
  bool operator!=(const ProcessConfig &other) const;

  nlohmann::json toJson() const;
  void fromJson(const nlohmann::json &json);
  std::string toString() const;
  void clear();
};