/*
 * @Author: vdavidyang vdavidyang@gmail.com
 * @Date: 2025-04-21 20:34:41
 * @LastEditors: vdavidyang vdavidyang@gmail.com
 * @LastEditTime: 2025-05-12 23:22:42
 * @FilePath: \GameOptimizerPro\include\config\process_info.h
 * @Description:
 * Copyright (c) 2025 by vdavidyang vdavidyang@gmail.com, All Rights Reserved.
 */

#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

/**
 * @enum ProcessType
 * @brief 进程类型枚举，分别为未知、游戏进程和反作弊进程
 * @note 该枚举用于区分不同类型的进程
 */
enum class ProcessType
{
  UNKNOWN,
  GAME_PROCESS,
  ANTI_CHEAT_PROCESS
};

/**
 * @enum ProcessStatus
 * @brief 进程状态枚举，分别为未知、注册表未设置、注册表已设置、注册表设置失败、未限制、已限制和限制失败
 * @note 该枚举用于表示进程的当前状态
 */
enum class ProcessStatus
{
  UNKNOWN,          // 初始或未知状态
  REGISTRY_NOT_SET, // 游戏进程：注册表未设置
  REGISTRY_SET,     // 游戏进程：注册表已设置
  REGISTRY_FAILED,  // 游戏进程：注册表设置失败
  NOT_RESTRICTED,   // 反作弊进程：未限制
  RESTRICTED,       // 反作弊进程：已限制
  RESTRICT_FAILED   // 反作弊进程：限制失败
};

/**
 * @class ProcessInfo
 * @brief 进程信息类，负责存储进程的名称、类型、状态等信息
 * @note 该类用于表示游戏进程和反作弊进程的信息
 */
class ProcessInfo
{
public:
  // 构造函数
  ProcessInfo();
  // 析构函数
  ~ProcessInfo();

  // 拷贝构造函数
  ProcessInfo(const ProcessInfo &other) = default;

  // 原始名称
  std::string name;
  // 进程名称数组
  std::vector<std::string> processList;
  // 当前设置状态
  bool status = false;

  // 赋值运算符
  ProcessInfo &operator=(const ProcessInfo &other);

  // 移动赋值运算符
  ProcessInfo &operator=(ProcessInfo &&other) noexcept;

  // 比较运算符
  bool operator==(const ProcessInfo &other) const;
  bool operator!=(const ProcessInfo &other) const;

  nlohmann::json toJson() const;
  void fromJson(const nlohmann::json &json);
  std::string toString() const;
  void clear();
};
