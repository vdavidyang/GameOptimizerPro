/*
 * @Author: vdavidyang vdavidyang@gmail.com
 * @Date: 2025-04-26 22:50:11
 * @LastEditors: vdavidyang vdavidyang@gmail.com
 * @LastEditTime: 2025-05-28 00:39:18
 * @FilePath: \GameOptimizerPro\include\core\application.h
 * @Description:
 * Copyright (c) 2025 by vdavidyang vdavidyang@gmail.com, All Rights Reserved.
 */

#pragma once

#include <thread>
#include <memory>
#include <atomic>
#include <QSystemTrayIcon>

#include "log/logging.h"
#include "core/config_manager.h"
#include "core/optimizer.h"

/**
 * @class Application
 * @brief 应用程序类，负责应用程序的初始化、关闭和主要功能接口
 * @note 该类使用单例模式实现，确保全局只有一个实例
 */
class Application
{
public:
  explicit Application(const std::wstring &configPath, QSystemTrayIcon *trayApp = nullptr);
  ~Application();

  /**
   * @brief 开启/关闭 优化游戏进程
   * @param {int} gameIndex 游戏索引
   * @param {bool} isOptimize 是否优化
   * @return {bool} 是否设置成功
   */
  bool setOptimizeGameProcess(int gameIndex, bool isOptimize);

  /**
   * @brief 开启/关闭 开机自启动
   * @param {bool} isAutoStartup 是否开机自启动
   * @return {bool} 是否设置成功
   */
  bool setAutoStartup(bool isAutoStartup);

  /**
   * @brief 开启/关闭 自动限制反作弊进程
   * @param {bool} isAutoLimit 是否自动限制
   * @return {bool} 是否设置成功
   */
  bool setAutoLimitAntiCheat(bool isAutoLimit, bool isQuit = false);

  /**
   * @brief 开启/关闭 游戏优化电源计划
   * @param {GUID *} PowerPlanGuid 电源计划GUID
   * @param {bool} isOptimize 是否优化
   * @return {bool} 是否设置成功
   */
  bool setGameOptimizePowerPlan(bool isOptimize);

  /**
   * @brief 开启/关闭 电源计划锁定
   * @param {bool} isLock
   * @return {bool} 是否设置成功
   */
  bool setPowerPlanLock(bool isLock);

  /**
   * @brief 开启/关闭 限制后台活动
   * @param {bool} isLimit
   * @return {bool} 是否设置成功
   */
  bool setBackgroundActivityLimit(bool isLimit);

  /**
   * @brief 开启/关闭 优化网络延迟
   * @param {bool} isOptimize 是否优化
   * @return {bool} 是否设置成功
   */
  bool setOptimizeNetworkDelay(bool isOptimize);

  /**
   * @brief 开启/关闭 系统调度优化
   * @param {bool} isOptimize 是否优化
   * @return {bool} 是否设置成功
   */
  bool setSystemSchedulerOptimization(bool isOptimize);

  /**
   * @brief 开启/关闭 系统服务优化
   * @param {bool} isOptimize 是否优化
   * @return {bool} 是否设置成功
   */
  bool setSystemServiceOptimization(bool isOptimize);

  // getters and setters

  /**
   * @brief 获取当前配置 / get current config
   * @return AppConfig 当前配置 / current config
   * @note 该函数返回当前配置的副本，修改不会影响原配置
   */
  AppConfig getCurrentConfig() const;

  // 状态获取
  bool isOptimizing() const { return m_isOptimizing; }
  ConfigManager &getConfigManager() { return *m_configManager; }

private:
  AppConfig m_currentConfig{DefaultConfig::Get()};
  std::unique_ptr<ConfigManager> m_configManager{nullptr};
  std::unique_ptr<Optimizer> m_optimizer;
  std::atomic<bool> m_isOptimizing{false};
  QSystemTrayIcon *m_trayIcon = nullptr;
};
