/*
 * @Author: vdavidyang vdavidyang@gmail.com
 * @Date: 2025-04-26 22:49:30
 * @LastEditors: vdavidyang vdavidyang@gmail.com
 * @LastEditTime: 2025-05-28 01:25:06
 * @FilePath: \GameOptimizerPro\src\core\application.cpp
 * @Description:
 * Copyright (c) 2025 by vdavidyang vdavidyang@gmail.com, All Rights Reserved.
 */

#include "core/application.h"

Application::Application(const std::wstring &configPath, QSystemTrayIcon *trayIcon)
    : m_trayIcon(trayIcon)
{
  // 初始化核心模块
  try
  {
    m_optimizer = std::make_unique<Optimizer>(trayIcon);
    m_configManager = std::make_unique<ConfigManager>(configPath);

    m_currentConfig = m_configManager->getConfig();
    LOG_INFO("Application初始化成功");
    // 启动配置热更新监听
    // m_configManager->StartConfigMonitor();
    // m_configManager->RegisterConfigReloadCallback([this]() {
    //     LOG_INFO(L"检测到配置更新，重新加载设置");
    //     ReloadConfiguration();
    // });
  }
  catch (const std::exception &e)
  {
    LOG_ERROR("初始化失败: " + std::string(e.what()));
  }
}

Application::~Application()
{
}

AppConfig Application::getCurrentConfig() const
{
  return m_configManager->getConfig();
}

bool Application::setAutoStartup(bool isAutoStartup)
{
  if (m_optimizer->setAutoStartup(isAutoStartup))
  {
    m_currentConfig.optimismConfig.autoStartUp = isAutoStartup;
    m_configManager->setConfig(m_currentConfig);
    LOG_INFO("设置自动启动成功");
    return true;
  }
  else
  {
    LOG_ERROR("设置自动启动失败");
    return false;
  }
}

bool Application::setOptimizeGameProcess(int gameIndex, bool isOptimize)
{
  // 根据 gameIndex 从 m_currentConfig 中获取进程名数组
  ProcessInfo processInfo = m_currentConfig.processConfig.gameProcessList[gameIndex];

  if (isOptimize)
  {
    // 设置游戏优化
    if (m_optimizer->setGameProcessRegistry(processInfo.processList, isOptimize))
    {
      m_currentConfig.processConfig.gameProcessList[gameIndex].status = isOptimize;
      m_configManager->setConfig(m_currentConfig);
      LOG_INFO("设置游戏优化成功: " + processInfo.name);
      return true;
    }
    else
    {
      LOG_ERROR("设置游戏优化失败: " + processInfo.name);
      return false;
    }
  }
  else
  {
    // 取消游戏优化
    if (m_optimizer->setGameProcessRegistry(processInfo.processList, isOptimize))
    {
      m_currentConfig.processConfig.gameProcessList[gameIndex].status = isOptimize;
      m_configManager->setConfig(m_currentConfig);
      LOG_INFO("取消游戏优化成功: " + processInfo.name);
      return true;
    }
    else
    {
      LOG_ERROR("取消游戏优化失败: " + processInfo.name);
      return false;
    }
  }
  return false;
}

bool Application::setAutoLimitAntiCheat(bool checked, bool isQuit)
{

  if (m_optimizer->setAutoLimitAntiCheat(checked, m_currentConfig.processConfig.antiCheatProcessList[0].processList))
  {
    // 如果是退出状态，则不需要保存配置
    if (!isQuit)
    {
      // 更新配置
      m_currentConfig.optimismConfig.autoLimitAntiCheat = checked;
      m_configManager->setConfig(m_currentConfig);
    }
    LOG_INFO("设置自动限制反作弊成功");
    return true;
  }
  else
  {
    LOG_ERROR("设置自动限制反作弊失败");
    return false;
  }

  return false;
}

bool Application::setGameOptimizePowerPlan(bool isOptimize)
{
  GUID PowerPlanGuid = StringToGuid(m_currentConfig.optimismConfig.powerPlan.powerPlanGuid);
  if (isOptimize)
  {
    if (m_optimizer->setGameOptimizePowerPlan(&PowerPlanGuid, isOptimize))
    {
      m_currentConfig.optimismConfig.powerPlan.optimizePowerPlan = true;
      LOG_INFO("开启高性能游戏电源计划成功");
    }
    else
    {
      LOG_ERROR("开启高性能游戏电源计划失败");
      return false;
    }
  }
  else
  {
    if (m_optimizer->setGameOptimizePowerPlan(&PowerPlanGuid, isOptimize))
    {
      m_currentConfig.optimismConfig.powerPlan.optimizePowerPlan = false;
      LOG_INFO("关闭高性能游戏电源计划成功");
    }
    else
    {
      LOG_ERROR("关闭高性能游戏电源计划失败");
      return false;
    }
  }
  m_currentConfig.optimismConfig.powerPlan.powerPlanGuid = GuidToString(&PowerPlanGuid);
  m_configManager->setConfig(m_currentConfig);
  return true;
}

// TODO: 设置电源计划锁定
bool Application::setPowerPlanLock(bool checked)
{
  MessageBoxW(NULL, L"设置电源计划锁定", L"设置电源计划锁定", MB_OK);
  return checked;
}

bool Application::setBackgroundActivityLimit(bool checked)
{
  if (m_optimizer->setBackgroundActivityLimit(checked))
  {
    m_currentConfig.optimismConfig.limitBackgroundActivity = checked;
    m_configManager->setConfig(m_currentConfig);
    LOG_INFO("设置后台活动限制成功");
    return true;
  }
  else
  {
    LOG_ERROR("设置后台活动限制失败");
    return false;
  }
}

bool Application::setOptimizeNetworkDelay(bool checked)
{
  if (m_optimizer->setOptimizeNetworkDelay(checked))
  {
    m_currentConfig.optimismConfig.optimizeNetworkDelay = checked;
    m_configManager->setConfig(m_currentConfig);
    LOG_INFO("设置优化网络延迟成功");
    return true;
  }
  else
  {
    LOG_ERROR("设置优化网络延迟失败");
    return false;
  }
}

bool Application::setSystemSchedulerOptimization(bool checked)
{
  if (m_optimizer->setSystemSchedulerOptimization(checked))
  {
    m_currentConfig.optimismConfig.optimizeSystemScheduling = checked;
    m_configManager->setConfig(m_currentConfig);
    LOG_INFO("设置系统调度优化成功");
    return true;
  }
  else
  {
    LOG_ERROR("设置系统调度优化失败");
    return false;
  }
}

bool Application::setSystemServiceOptimization(bool checked)
{
  if (checked)
  {
    if (m_optimizer->setSystemServiceOptimization(checked))
    {
      LOG_ERROR("设置系统服务优化成功");
      return true;
    }
    else
    {
      LOG_ERROR("设置系统服务优化失败");
      return false;
    }
  }
  else
  {
    if (m_optimizer->setSystemServiceOptimization(checked))
    {
      LOG_ERROR("取消系统服务优化成功");
      return true;
    }
    else
    {
      LOG_ERROR("取消系统服务优化失败");
      return false;
    }
  }
}