/*
 * @Author: vdavidyang vdavidyang@gmail.com
 * @Date: 2025-04-26 23:18:06
 * @LastEditors: vdavidyang vdavidyang@gmail.com
 * @LastEditTime: 2025-05-28 00:17:23
 * @FilePath: \GameOptimizerPro\include\core\optimizer.h
 * @Description:
 * Copyright (c) 2025 by vdavidyang vdavidyang@gmail.com, All Rights Reserved.
 */

#pragma once

#include <vector>
#include <string>
#include <memory>
#include <windows.h>
#include <QSystemTrayIcon>

#include "log/logging.h"

#include "core/process_manager.h"
#include "core/registry_manager.h"
#include "core/power_manager.h"
#include "core/service_manager.h"

#include "utils/registry_key.h"

/**
 * @class Optimizer
 * @brief 优化器类，负责游戏优化相关的操作
 * 包括设置开机自启动、限制反作弊进程、优化电源计划等功能
 */
class Optimizer
{
public:
    /**
     * @brief 构造函数
     * @details 初始化进程、注册表和电源管理器
     */
    explicit Optimizer(QSystemTrayIcon *trayIcon = nullptr);
    ~Optimizer();

    // 主要功能

    /**
     * @brief 设置开机自启动
     * @param bool isAutoStartup 是否开机自启动
     * @return bool 是否设置成功
     */
    bool setAutoStartup(bool isAutoStartup);

    /**
     * @brief: 开启/关闭 自动限制反作弊进程
     * @param bool isAutoLimit 是否自动限制
     * @param vector<std::string> &processNames 要监听的进程名称
     * @return bool 是否设置成功
     */
    bool setAutoLimitAntiCheat(bool isAutoLimit, const std::vector<std::string> &processNames = {});

    /**
     * @brief 设置游戏优化电源计划
     * @param GUID *PowerPlanGuid 电源计划GUID
     * @param bool isOptimize 是否优化
     * @return bool 是否设置成功
     */
    bool setGameOptimizePowerPlan(GUID *PowerPlanGuid, bool isOptimize);

    /**
     * @brief 设置电源计划锁定
     * @param bool isLock 是否锁定
     * @return bool 是否设置成功
     */
    bool setPowerPlanLock(bool isLock);

    /**
     * @brief 设置后台活动限制
     * @param bool isLimit 是否限制
     * @return bool 是否设置成功
     */
    bool setBackgroundActivityLimit(bool isLimit);

    /**
     * @brief 设置优化网络延迟
     * @param bool isOptimize 是否优化
     * @return bool 是否设置成功
     */
    bool setOptimizeNetworkDelay(bool isOptimize);

    /**
     * @brief 设置系统调度优化
     * @param bool isOptimize 是否优化
     * @return bool 是否设置成功
     */
    bool setSystemSchedulerOptimization(bool isOptimize);

    /**
     * @brief 设置系统服务优化
     * @param bool isOptimize 是否优化
     * @return bool 是否设置成功
     */
    bool setSystemServiceOptimization(bool isOptimize);

    /**
     * @brief 检查游戏进程注册表
     * @param vector<std::string> &processNames 游戏进程名称
     * @return bool 是否检查成功
     */
    bool checkGameProcessRegistry(const std::vector<std::string> &processNames);

    /**
     * @brief 设置/删除游戏进程注册表
     * @param vector<std::string> &processNames 游戏进程名称
     * @return bool 是否设置成功
     */
    bool setGameProcessRegistry(const std::vector<std::string> &processNames, bool isOptimize);

private:
    // ATL Module Instance - Required for CComObject, etc.
    CComModule m_Module;

    std::unique_ptr<ProcessManager> m_processManager{nullptr};
    std::unique_ptr<PowerManager> m_powerManager{nullptr};
    std::unique_ptr<RegistryManager> m_registryManager{nullptr};
    std::unique_ptr<ServiceManager> m_serviceManager{nullptr};
    QSystemTrayIcon *m_trayIcon{nullptr};

    // 储存注册表项的map
    std::map<std::string, RegistryKey> m_registryKeys = {
        {"AutoStartup",
         RegistryKey(
             HKEY_CURRENT_USER,
             "Software\\Microsoft\\Windows\\CurrentVersion\\Run\\",
             {KeyValue("GameOptimizerPro", "GameOptimizerPro", "GameOptimizerPro")})},
        {"OptimizeBackgroundActivity",
         RegistryKey(
             HKEY_LOCAL_MACHINE,
             "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Multimedia\\SystemProfile\\",
             {KeyValue("NetworkThrottlingIndex", 4294967295, 4294967295),
              KeyValue("SystemResponsiveness", 20, 5)})},
        {"OptimizeNetWorkDelay",
         RegistryKey(
             HKEY_LOCAL_MACHINE,
             "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces\\",
             {
                 KeyValue("TcpAckFrequency", 1, 1),
                 KeyValue("TcpNoDelay", 1, 1),
                 KeyValue("TcpDelAckTicks", 0, 0),
             })},
        {"OptimizeSystemScheduler",
         RegistryKey(
             HKEY_LOCAL_MACHINE,
             "SYSTEM\\CurrentControlSet\\Control\\PriorityControl\\",
             {KeyValue("Win32PrioritySeparation", 38, 40)})},
        {"GameProcessRegistry",
         RegistryKey(
             HKEY_LOCAL_MACHINE,
             "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\",
             {KeyValue("", "", "")})},
        {"NetworkInterfaceCardIds",
         RegistryKey(
             HKEY_LOCAL_MACHINE,
             "SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}",
             {KeyValue("NetCfgInstanceId", "", "")})}};

    /**
     * 储存优化服务的vector
     * 其中包含pair first: serviceName second: startType
     * 包括 Windows诊断策略服务、Windows Search、Program Compatibility Assistant、SysMain
     */
    std::vector<std::pair<std::wstring, DWORD>> m_serviceList = {
        {L"diagsvc", 3},
        {L"DPS", 3},
        {L"WdiServiceHost", 3},
        {L"WdiSystemHost", 3},
        {L"WSearch", 3},
        {L"PcaSvc", 3},
        {L"SysMain", 3}};

    /**
     * @brief 检查是否开机自启动
     * @return bool 是否开机自启动
     */
    bool isStartupEnabled();

    /**
     * @brief 设置监听器回调函数
     */
    void setListenerCallback();
};
