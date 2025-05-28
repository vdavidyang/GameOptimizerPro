/*
 * @Author: vdavidyang vdavidyang@gmail.com
 * @Date: 2025-04-21 22:40:50
 * @LastEditors: vdavidyang vdavidyang@gmail.com
 * @LastEditTime: 2025-05-10 22:56:14
 * @FilePath: \GameOptimizerPro\include\core\config_manager.h
 * @Description:
 * Copyright (c) 2025 by vdavidyang vdavidyang@gmail.com, All Rights Reserved.
 */

#pragma once

#include <vector>
#include <string>
#include <atomic>
#include <mutex>
#include <windows.h>

#include <nlohmann/json.hpp>

#include "config/app_config.h"

#include "log/logging.h"
#include "utils/system_utils.h"

struct DefaultConfig
{
  static AppConfig Get()
  {
    AppConfig config;
    config.version = "1.0.0.0";

    // 系统信息默认值
    config.systemInfo.os = "Windows 10";
    config.systemInfo.arch = "x64";
    config.systemInfo.cpu = "Unknown";

    // 优化配置默认值
    config.optimismConfig.autoStartUp = false;
    config.optimismConfig.autoLimitAntiCheat = false;
    config.optimismConfig.powerPlan.optimizePowerPlan = false;
    config.optimismConfig.powerPlan.lockPowerPlan = false;
    config.optimismConfig.powerPlan.powerPlanGuid = "{b3700455-1a40-4754-9352-fc99a63055db}";
    config.optimismConfig.limitBackgroundActivity = false;
    config.optimismConfig.optimizeNetworkDelay = false;
    config.optimismConfig.optimizeSystemScheduling = false;
    config.optimismConfig.optimizeSystemService = false;

    return config;
  }
};

/**
 * @class ConfigManager
 * @brief 配置管理器类，负责加载、保存和监控配置文件的变化
 */
class ConfigManager
{
public:
  /**
   * @brief 构造函数
   * @param {wstring} &configPath 配置路径
   */
  ConfigManager(const std::wstring &configPath);

  /**
   * @brief 私有析构函数
   */
  ~ConfigManager();

  // 删除拷贝构造和赋值操作
  ConfigManager(const ConfigManager &) = delete;
  ConfigManager &operator=(const ConfigManager &) = delete;

  /**
   * @brief 启动/停止配置文件监控
   * @param {bool} enable
   * @return {bool}
   */
  bool setConfigMonitor(bool enable);

  /**
   * @brief 获取当前配置的副本
   * @return {AppConfig} 当前配置的副本
   */
  AppConfig getConfig();

  /**
   * @brief 设置配置
   * @param {AppConfig} &config 配置
   * @return {bool} 是否设置成功
   */
  bool setConfig(const AppConfig &config);

private:
  /**
   * @brief 获取配置路径
   * @return {wstring} 配置路径
   */
  std::wstring getConfigPath() const;

  /**
   * @brief 加载配置 允许传入路径或使用默认
   * @param {wstring} &configPath 配置路径
   * @return {bool} 是否加载成功
   */
  bool loadConfig(const std::wstring &configPath = L"");

  /**
   * @brief 保存配置 允许传入路径或使用默认
   * @param {wstring} &configPath 配置路径
   * @return {bool} 是否保存成功
   */
  bool saveConfig(const std::wstring &configPath = L"");

  /**
   * @brief 重新加载配置
   */
  void reloadConfig();

  /**
   * @brief 验证配置
   * @param {AppConfig} &config 配置
   * @return {bool} 是否验证成功
   */
  bool validateConfig(const AppConfig &config) const;

  // 配置类
  AppConfig m_appConfig;
  // 配置文件路径
  std::wstring m_configPath;

  // 线程安全的互斥锁
  std::mutex m_mutex;
};