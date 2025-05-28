/*
 * @Author: vdavidyang vdavidyang@gmail.com
 * @Date: 2025-04-26 23:19:20
 * @LastEditors: vdavidyang vdavidyang@gmail.com
 * @LastEditTime: 2025-05-28 00:56:49
 * @FilePath: \GameOptimizerPro\src\core\optimizer.cpp
 * @Description:
 * Copyright (c) 2025 by vdavidyang vdavidyang@gmail.com, All Rights Reserved.
 */

#include "core/optimizer.h"

Optimizer::Optimizer(QSystemTrayIcon *trayIcon)
    : m_trayIcon(trayIcon)
{
  // 初始化进程、注册表、电源、服务管理器
  try
  {
    m_Module.Init(NULL, GetModuleHandleW(NULL));
    m_processManager = std::make_unique<ProcessManager>();
    m_registryManager = std::make_unique<RegistryManager>();
    m_powerManager = std::make_unique<PowerManager>();
    m_serviceManager = std::make_unique<ServiceManager>();
  }
  catch (const std::exception &e)
  {
    LOG_ERROR("初始化优化管理器失败: " + std::string(e.what()));
  }
}

Optimizer::~Optimizer()
{
  m_Module.Term();
}

bool Optimizer::setAutoStartup(bool isAutoStartup)
{
  char path[MAX_PATH];
  if (!GetModuleFileNameA(NULL, path, MAX_PATH))
  {
    LOG_ERROR("获取当前运行路径失败");
    return false;
  }

  bool result = false;

  HKEY hRoot = m_registryKeys["AutoStartup"].hRoot;
  std::string subKey = m_registryKeys["AutoStartup"].subKey;

  if (isAutoStartup)
  {
    for (const auto &valueKey : m_registryKeys["AutoStartup"].keyValueList)
    {
      std::string valueName = valueKey.key;
      result = m_registryManager->setRegistryStringValue(hRoot, subKey, valueName, path);
      if (!result)
      {
        LOG_ERROR("设置注册表项失败");
        return false;
      }
    }
  }
  else
  {
    for (const auto &valueKey : m_registryKeys["AutoStartup"].keyValueList)
    {
      std::string valueName = valueKey.key;
      result = m_registryManager->deleteRegistryValue(hRoot, subKey, valueName);
      if (!result)
      {
        LOG_ERROR("删除注册表项失败");
        return false;
      }
    }
  }
  return true;
}

bool Optimizer::setAutoLimitAntiCheat(bool isAutoLimit, const std::vector<std::string> &processNames)
{
  if (isAutoLimit)
  {
    try
    {
      // 设置回调函数
      setListenerCallback();
      if (m_processManager->startListening(processNames))
      {
        std::cout << "Listening started successfully. Open/close monitored processes." << std::endl;
        LOG_INFO("监听进程创建和销毁事件成功");
        return true;
      }
      else
      {
        std::cerr << "Failed to start listening." << std::endl;
        LOG_ERROR("监听进程创建和销毁事件失败");
        return false;
      }
    }
    catch (const _com_error &e)
    {
      std::wcerr << L"A critical COM/WMI error occurred during initialization: " << e.ErrorMessage() << std::endl;
      // LOG_INFO("A critical COM/WMI error occurred during initialization: " + std::string(e.ErrorMessage()));
      return false;
    }
    catch (const std::exception &e)
    {
      std::cerr << "An unexpected standard exception occurred: " << e.what() << std::endl;
      return false;
    }
    catch (...)
    {
      std::cerr << "An unknown error occurred." << std::endl;
      return false;
    }
  }
  else
  {
    try
    {
      if (m_processManager->stopListening())
      {
        std::cout << "Listener stopped successfully." << std::endl;
        LOG_INFO("停止监听进程创建和销毁事件成功");
        return true;
      }
      else
      {
        std::cerr << "Failed to stop listener." << std::endl;
        LOG_ERROR("停止监听进程创建和销毁事件失败");
        return false;
      }
    }
    catch (const std::exception &e)
    {
      std::cerr << "Error stopping listener: " << e.what() << std::endl;
      LOG_ERROR("停止监听进程创建和销毁事件失败: " + std::string(e.what()));
      return false;
    }
    catch (...)
    {
      std::cerr << "Unknown error stopping listener." << std::endl;
      LOG_ERROR("停止监听进程创建和销毁事件失败");
      return false;
    }
  }
  return false;
}

// TODO: 待完善
void Optimizer::setListenerCallback()
{
  // --- 设置回调函数 ---
  // 使用 lambda 表达式作为回调
  m_processManager->setOnProcessCreatedCallback(
      [this](const std::wstring &processName, DWORD processId)
      {
    LOG_INFO(L"[Callback] Process Started: '" + processName + L" PID: " + std::to_wstring(processId) + L" 启动");

    std::thread([this, processName, processId]()
                {
                      std::this_thread::sleep_for(std::chrono::seconds(5));
                      if (!m_processManager->restrictAntiCheatProcessPS(processName, processId))
                      {
                        LOG_ERROR(L"限制进程 " + processName + L" 失败");
                        return;
                      }
                      m_trayIcon->showMessage(
                        "鱼腥味的游戏优化工具箱",
                        "限制进程 " + QString::fromStdWString(processName) + " 成功",
                        QSystemTrayIcon::Information,
                        5000
                      );})
            .detach(); });

  m_processManager->setOnProcessDestroyedCallback(
      [this](const std::wstring &processName, DWORD processId)
      {
        LOG_INFO(L"[Callback] Process Destroyed: '" + processName + L" PID: " + std::to_wstring(processId) + L" 销毁");
      });

  m_processManager->setOnErrorCallback(
      [this](long errorCode)
      {
        LOG_HRESULT(L"WMI Failure.", errorCode);
      });
}

bool Optimizer::setGameOptimizePowerPlan(GUID *PowerPlanGuid, bool isOptimize)
{
  bool result = false;
  if (isOptimize)
  {
    result = m_powerManager->createPowerPlan(PowerPlanGuid, _T("高性能游戏电源计划"), _T("鱼腥味的高性能游戏电源计划"));
    if (!result)
    {
      LOG_ERROR("创建高性能游戏电源计划失败");
      return result;
    }
    result = m_powerManager->optimizePowerPlan(PowerPlanGuid);
    if (!result)
    {
      LOG_ERROR("优化高性能游戏电源计划失败");
      return result;
    }
    result = m_powerManager->setPowerPlanActive(PowerPlanGuid);
    if (!result)
    {
      LOG_ERROR("设置高性能游戏电源计划为活动电源计划失败");
      return result;
    }
    return true;
  }
  else
  {
    result = m_powerManager->deletePowerPlan(PowerPlanGuid);
    if (!result)
    {
      LOG_ERROR("删除高性能游戏电源计划失败");
      return result;
    }
    return true;
  }
}

// TODO: 设置电源计划锁定
bool Optimizer::setPowerPlanLock(bool isLock)
{
  return isLock;
}

bool Optimizer::setBackgroundActivityLimit(bool isLimit)
{
  //  // 假设 key 是一个 RegistryKey 对象
  //   if (std::holds_alternative<DWORD>(key.value)) {
  //       DWORD dwordValue = std::get<DWORD>(key.value);
  //       // ... 使用 dwordValue ...
  //   } else if (std::holds_alternative<std::string>(key.value)) {
  //       std::string stringValue = std::get<std::string>(key.value);
  //       // ... 处理字符串情况 ...
  //   }
  // 设置注册表路径：HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Multimedia\SystemProfile
  // 设置值：NetworkThrottlingIndex 优化前为十六进制 0xffffffff 十进制 4294967295，优化后为十六进制 0xffffffff 十进制 4294967295
  // 设置值：WSystemResponsiveness 优化前为十六进制 0x00000014 十进制 20，优化后为十六进制 0x00000005 十进制 5
  HKEY hRoot = m_registryKeys["OptimizeBackgroundActivity"].hRoot;
  std::string subKey = m_registryKeys["OptimizeBackgroundActivity"].subKey;

  if (isLimit)
  {
    for (const auto &valueKey : m_registryKeys["OptimizeBackgroundActivity"].keyValueList)
    {
      std::string valueName = valueKey.key;
      DWORD value = std::get<DWORD>(valueKey.optimizeValue);
      if (m_registryManager->setRegistryDWORDValue(hRoot, subKey, valueName, value))
      {
        LOG_INFO("设置后台活动限制成功，" + valueName + ": " + std::to_string(value));
      }
      else
      {
        LOG_ERROR("设置后台活动限制失败，" + valueName + ": " + std::to_string(value));
        return false;
      }
    }
  }
  else
  {
    for (const auto &valueKey : m_registryKeys["OptimizeBackgroundActivity"].keyValueList)
    {
      std::string valueName = valueKey.key;
      DWORD value = std::get<DWORD>(valueKey.originValue);
      if (m_registryManager->setRegistryDWORDValue(hRoot, subKey, valueName, value))
      {
        LOG_INFO("取消后台活动限制成功，" + valueName + ": " + std::to_string(value));
      }
      else
      {
        LOG_ERROR("取消后台活动限制失败，" + valueName + ": " + std::to_string(value));
        return false;
      }
    }
  }
  return true;
}

bool Optimizer::setOptimizeNetworkDelay(bool isOptimize)
{
  // 设置注册表路径：HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Tcpip\Parameters\Interfaces\{NIC-id}
  // 设置值：TcpAckFrequency 优化前无该值，直接删除即可，优化后为十六进制 0x00000001 十进制 1
  // 设置值：TCPNoDelay 优化前无该值，直接删除即可，优化后为十六进制 0x00000001 十进制 1
  // 设置值：TcpDelAckTicks 优化前无该值，直接删除即可，优化后为十六进制 0x00000000 十进制 0

  // 先获取所有网络接口卡ID
  std::vector<std::string> nicIds;
  HKEY hRoot = m_registryKeys["NetworkInterfaceCardIds"].hRoot;
  std::string subKey = m_registryKeys["NetworkInterfaceCardIds"].subKey;
  std::string valueName = m_registryKeys["NetworkInterfaceCardIds"].keyValueList[0].key;

  if (!m_registryManager->GetNetworkInterfaceCardIds(hRoot, subKey, valueName, nicIds))
  {
    LOG_ERROR("获取网络接口卡ID失败");
    return false;
  }

  hRoot = m_registryKeys["OptimizeNetWorkDelay"].hRoot;
  subKey = m_registryKeys["OptimizeNetWorkDelay"].subKey;

  // 过滤掉无效的ID
  if (!m_registryManager->filterInvalidNetworkInterfaceCardIds(hRoot, subKey, nicIds))
  {
    LOG_ERROR("过滤无效的网络接口卡ID失败");
    return false;
  }

  if (nicIds.empty())
  {
    LOG_ERROR("没有有效的网络接口卡ID");
    return false;
  }

  if (isOptimize)
  {
    // 设置注册表项
    for (const auto &nicId : nicIds)
    {
      std::string keyPath = subKey + nicId;
      for (const auto &valueKey : m_registryKeys["OptimizeNetWorkDelay"].keyValueList)
      {
        DWORD value = std::get<DWORD>(valueKey.optimizeValue);
        if (!m_registryManager->setRegistryDWORDValue(hRoot, keyPath, valueKey.key, value))
        {
          LOG_ERROR("设置网络延迟优化失败，" + keyPath + ": " + std::to_string(value));
          return false;
        }
      }
    }
  }
  else
  {
    // 删除注册表项
    for (const auto &nicId : nicIds)
    {
      std::string keyPath = subKey + nicId;
      for (const auto &valueKey : m_registryKeys["OptimizeNetWorkDelay"].keyValueList)
      {
        if (!m_registryManager->deleteRegistryValue(hRoot, keyPath, valueKey.key))
        {
          LOG_ERROR("删除网络延迟优化失败，" + keyPath + ": " + valueKey.key);
          return false;
        }
      }
    }
  }

  return true;
}

bool Optimizer::setSystemSchedulerOptimization(bool isOptimize)
{
  // 设置注册表路径：HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\PriorityControl
  // 设置值：Win32PrioritySeparation 优化前为十六进制 0x00000026 十进制 38，优化后为十六进制 0x00000028 十进制 40
  HKEY hRoot = m_registryKeys["OptimizeSystemScheduler"].hRoot;
  std::string subKey = m_registryKeys["OptimizeSystemScheduler"].subKey;

  if (isOptimize)
  {
    for (const auto &valueKey : m_registryKeys["OptimizeSystemScheduler"].keyValueList)
    {
      std::string valueName = valueKey.key;
      DWORD value = std::get<DWORD>(valueKey.optimizeValue);
      if (m_registryManager->setRegistryDWORDValue(hRoot, subKey, valueName, value))
      {
        LOG_INFO("设置系统调度优化成功，" + valueName + ": " + std::to_string(value));
      }
      else
      {
        LOG_ERROR("设置系统调度优化失败，" + valueName + ": " + std::to_string(value));
        return false;
      }
    }
  }
  else
  {
    for (const auto &valueKey : m_registryKeys["OptimizeSystemScheduler"].keyValueList)
    {
      std::string valueName = valueKey.key;
      DWORD value = std::get<DWORD>(valueKey.originValue);
      if (m_registryManager->setRegistryDWORDValue(hRoot, subKey, valueName, value))
      {
        LOG_INFO("取消系统调度优化成功，" + valueName + ": " + std::to_string(value));
      }
      else
      {
        LOG_ERROR("取消系统调度优化失败，" + valueName + ": " + std::to_string(value));
        return false;
      }
    }
  }
  return true;
}

bool Optimizer::setSystemServiceOptimization(bool isOptimize)
{
  if (isOptimize)
  {
    // 遍历需要优化的服务列表
    for (const auto &service : m_serviceList)
    {
      // 先检查当前服务是否正在运行
      bool status = false;
      DWORD startType = 4;
      if (!m_serviceManager->queryServiceStatus(service.first, status, startType))
      {
        LOG_ERROR(L"查询服务状态失败：" + service.first);
        return false;
      }

      // 服务正在运行 先停止服务
      if (status)
      {
        if (!m_serviceManager->stopService(service.first))
        {
          LOG_ERROR(L"服务停止失败：" + service.first);
          return false;
        }
      }

      // 设置服务启动类型
      if (!m_serviceManager->setServiceStartType(service.first, service.second))
      {
        LOG_ERROR(L"设置服务启动类型失败：" + service.first);
        return false;
      }
    }
  }
  else
  {
  }

  return true;
}

bool Optimizer::checkGameProcessRegistry(const std::vector<std::string> &processNames)
{
  for (const auto &processName : processNames)
  {
    if (!m_registryManager->checkRegistryKey(m_registryKeys["GameProcessRegistry"].hRoot, m_registryKeys["GameProcessRegistry"].subKey, processName))
    {
      // Image File Execution Options 下不存在processNames项
      return false;
    }
  }
  return true;
}

bool Optimizer::setGameProcessRegistry(const std::vector<std::string> &processNames, bool isOptimize)
{
  bool result = false;

  if (isOptimize)
  {
    // 设置游戏进程注册表
    try
    {
      for (const auto &processName : processNames)
      {
        HKEY hRoot = m_registryKeys["GameProcessRegistry"].hRoot;
        // 创建 processName 键
        std::string regPath = m_registryKeys["GameProcessRegistry"].subKey;
        result = m_registryManager->createRegistryKey(hRoot, regPath, processName);
        if (!result)
        {
          LOG_ERROR("注册表键创建失败: " + regPath + processName);
        }

        // 创建PerfOptions键
        regPath += processName;
        result = m_registryManager->createRegistryKey(hRoot, regPath, "PerfOptions");
        if (!result)
        {
          LOG_ERROR("注册表键创建失败: " + regPath + "\\PerfOptions");
        }

        // 设置CPU\IO\Memory为高优先级
        DWORD cpuPriorityValue = 3;
        DWORD ioPriorityValue = 3;
        DWORD memoryPriorityValue = 3;

        // 设置CpuPriorityClass
        regPath += "\\PerfOptions";
        result = m_registryManager->setRegistryDWORDValue(hRoot, regPath, "CpuPriorityClass", cpuPriorityValue);
        if (!result)
        {
          // 不一定是致命错误，继续尝试设置 IO
          LOG_ERROR("注册表值设置失败: " + regPath + "\\CpuPriorityClass");
        }

        // 设置IoPriority
        result = m_registryManager->setRegistryDWORDValue(hRoot, regPath, "IoPriority", ioPriorityValue);
        if (!result)
        {
          // 不一定是致命错误
          LOG_ERROR("注册表值设置失败: " + regPath + "\\IoPriority");
        }

        // 设置MemoryPriority
        result = m_registryManager->setRegistryDWORDValue(hRoot, regPath, "MemoryPriority", memoryPriorityValue);
        if (!result)
        {
          // 不一定是致命错误
          LOG_ERROR("注册表值设置失败: " + regPath + "\\MemoryPriority");
        }

        // 提示设置成功 至少部分成功
        LOG_INFO("注册表性能选项设置成功: " + processName);
        return true;
      }
    }
    catch (const std::exception &e)
    {
      LOG_ERROR("设置注册表时发生异常: " + std::string(e.what()));
      return false;
    }
    catch (...)
    {
      LOG_ERROR("设置注册表时发生未知异常");
      return false;
    }
  }
  else
  {
    // 删除游戏进程注册表
    try
    {
      HKEY hRoot = m_registryKeys["GameProcessRegistry"].hRoot;
      std::string regPath = m_registryKeys["GameProcessRegistry"].subKey;
      for (const auto &processName : processNames)
      {
        if (m_registryManager->checkRegistryKey(hRoot, regPath, processName))
        {
          result = m_registryManager->deleteRegistryKey(hRoot, regPath, processName);
          if (!result)
          {
            LOG_ERROR("注册表性能选项删除失败: " + processName);
            return false;
          }
          LOG_INFO("注册表性能选项删除成功: " + processName);
          return true;
        }
      }
    }
    catch (const std::exception &e)
    {
      LOG_ERROR("删除注册表时发生异常: " + std::string(e.what()));
      return false;
    }
    catch (...)
    {
      LOG_ERROR("删除注册表时发生未知异常");
      return false;
    }
  }

  return false;
}
