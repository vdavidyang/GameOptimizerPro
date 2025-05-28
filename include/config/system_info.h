/*
 * @Author: vdavidyang vdavidyang@gmail.com
 * @Date: 2025-04-22 17:11:24
 * @LastEditors: vdavidyang vdavidyang@gmail.com
 * @LastEditTime: 2025-05-04 19:28:33
 * @FilePath: \GameOptimizerPro\include\config\system_info.h
 * @Description:
 * Copyright (c) 2025 by vdavidyang vdavidyang@gmail.com, All Rights Reserved.
 */

#pragma once

#include <string>
#include <nlohmann/json.hpp>

/**
 * @class SystemInfo
 * @brief 系统信息配置类，负责存储和管理系统信息
 * @note 该类使用 nlohmann::json 库来处理 JSON 数据
 */
class SystemInfo
{
public:
  std::string os;
  std::string arch;
  std::string cpu;

  SystemInfo();
  ~SystemInfo();

  // 赋值运算符
  SystemInfo &operator=(const SystemInfo &other);

  // 移动赋值运算符
  SystemInfo &operator=(SystemInfo &&other) noexcept;

  // 比较运算符
  bool operator==(const SystemInfo &other) const;
  bool operator!=(const SystemInfo &other) const;

  nlohmann::json toJson() const;
  void fromJson(const nlohmann::json &json);

  std::string toString() const;
  void clear();
};
