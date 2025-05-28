/*
 * @Author: vdavidyang vdavidyang@gmail.com
 * @Date: 2025-04-21 22:40:40
 * @LastEditors: vdavidyang vdavidyang@gmail.com
 * @LastEditTime: 2025-05-11 00:08:01
 * @FilePath: \GameOptimizerPro\src\core\config_manager.cpp
 * @Description:
 * Copyright (c) 2025 by vdavidyang vdavidyang@gmail.com, All Rights Reserved.
 */
#include "core/config_manager.h"

// 构造函数：初始化路径等
ConfigManager::ConfigManager(const std::wstring &configPath)
{
  // 获取默认配置路径
  m_configPath = configPath.empty() ? getConfigPath() : configPath;
  LOG_INFO(L"配置管理器初始化，配置文件路径: " + m_configPath);
  loadConfig(m_configPath);
}

ConfigManager::~ConfigManager()
{
  saveConfig(m_configPath);
}

// 从环境变量或命令行参数读取配置路径
std::wstring ConfigManager::getConfigPath() const
{
  wchar_t path[MAX_PATH];
  if (GetEnvironmentVariableW(L"GAME_OPTIMIZER_CONFIG", path, MAX_PATH) > 0)
  {
    return path;
  }
  else
  {
    // 尝试获取可执行文件目录
    wchar_t exePath[MAX_PATH];
    if (GetModuleFileNameW(NULL, exePath, MAX_PATH) > 0)
    {
      try
      {
        return (std::filesystem::path(exePath).parent_path() / L"config/config.json").wstring();
      }
      catch (...)
      {
        // 文件系统路径可能抛异常 出错则使用相对路径
        return L"config/config.json";
      }
    }
    else
    {
      // 最终回退到相对路径
      return L"config/config.json";
    }
  }
}

// 验证配置文件
bool ConfigManager::validateConfig(const AppConfig &config) const
{

  // 检查游戏进程名和反作弊进程名合法性

  for (const auto &process : config.processConfig.gameProcessList)
  {
    for (const auto &processName : process.processList)
    {
      if (!isValidProcessName(processName))
      {
        LOG_ERROR(L"无效的进程名: " + MultiByteToWide(processName, CP_ACP));
        return false;
      }
    }
  }

  for (const auto &process : config.processConfig.antiCheatProcessList)
  {
    for (const auto &processName : process.processList)
    {
      if (!isValidProcessName(processName))
      {
        LOG_ERROR(L"无效的进程名: " + MultiByteToWide(processName, CP_ACP));
        return false;
      }
    }
  }

  return true;
}

bool ConfigManager::loadConfig(const std::wstring &configPath)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  // 如果传入了配置路径，则使用传入的路径
  std::wstring pathToLoad = configPath.empty() ? m_configPath : configPath;

  try
  {
    // 以二进制模式打开文件
    std::ifstream configFile(pathToLoad, std::ios::binary);
    if (!configFile.is_open())
    {
      LOG_ERROR(L"无法打开配置文件: " + pathToLoad);
      // 加载默认配置
      m_appConfig = DefaultConfig::Get();
      LOG_INFO(L"加载默认配置");
      return false;
    }

    // 直接从二进制文件流解析 JSON

    nlohmann::json configJson;
    try
    {
      configJson = nlohmann::json::parse(configFile);
    }
    catch (const nlohmann::json::parse_error &e)
    {
      LOG_ERROR(L"配置文件解析错误: " + MultiByteToWide(e.what(), CP_ACP) +
                L" at byte " + std::to_wstring(e.byte));
      m_appConfig = DefaultConfig::Get();
      return false;
    }
    configFile.close();

    // 检查appConfig是否存在
    if (!configJson.contains("appConfig"))
    {
      LOG_ERROR(L"appConfig not found in config");
      m_appConfig = DefaultConfig::Get();
      return false;
    }

    auto appConfigJson = configJson["appConfig"];
    AppConfig tempConfig = DefaultConfig::Get();

    // 加载版本信息
    if (appConfigJson.contains("version"))
    {
      tempConfig.version = appConfigJson["version"].get<std::string>();
    }

    // 加载系统信息
    if (appConfigJson.contains("systemInfo"))
    {
      auto &systemInfoJson = appConfigJson["systemInfo"];
      if (systemInfoJson.contains("os"))
      {
        tempConfig.systemInfo.os = systemInfoJson["os"].get<std::string>();
      }
      if (systemInfoJson.contains("arch"))
      {
        tempConfig.systemInfo.arch = systemInfoJson["arch"].get<std::string>();
      }
      if (systemInfoJson.contains("cpu"))
      {
        tempConfig.systemInfo.cpu = systemInfoJson["cpu"].get<std::string>();
      }
    }

    // 加载优化配置
    if (appConfigJson.contains("optimismConfig"))
    {
      auto &optimismConfigJson = appConfigJson["optimismConfig"];

      // 使用模板函数安全获取值
      auto safeGetBool = [](const nlohmann::json &j, const std::string &key, bool defaultValue)
      {
        return j.contains(key) ? j[key].get<bool>() : defaultValue;
      };

      tempConfig.optimismConfig.autoStartUp = safeGetBool(optimismConfigJson, "autoStartUp", false);
      tempConfig.optimismConfig.autoLimitAntiCheat = safeGetBool(optimismConfigJson, "autoLimitAntiCheat", false);

      // 电源计划配置
      if (optimismConfigJson.contains("powerPlan"))
      {
        auto &powerPlanJson = optimismConfigJson["powerPlan"];
        tempConfig.optimismConfig.powerPlan.optimizePowerPlan = safeGetBool(powerPlanJson, "optimizePowerPlan", false);
        tempConfig.optimismConfig.powerPlan.lockPowerPlan = safeGetBool(powerPlanJson, "lockPowerPlan", false);

        if (powerPlanJson.contains("powerPlanGuid"))
        {
          auto guid = powerPlanJson["powerPlanGuid"].get<std::string>();
          // 简单验证GUID格式
          if (!guid.empty() && guid.front() == '{' && guid.back() == '}')
          {
            tempConfig.optimismConfig.powerPlan.powerPlanGuid = guid;
          }
        }
      }

      tempConfig.optimismConfig.limitBackgroundActivity = safeGetBool(optimismConfigJson, "limitBackgroundActivity", false);
      tempConfig.optimismConfig.optimizeNetworkDelay = safeGetBool(optimismConfigJson, "optimizeNetworkDelay", false);
      tempConfig.optimismConfig.optimizeSystemScheduling = safeGetBool(optimismConfigJson, "optimizeSystemScheduling", false);
      tempConfig.optimismConfig.optimizeSystemService = safeGetBool(optimismConfigJson, "optimizeSystemService", false);
    }

    // 加载进程配置
    if (appConfigJson.contains("processConfig"))
    {
      auto &processConfigJson = appConfigJson["processConfig"];

      // 加载游戏进程
      if (processConfigJson.contains("gameProcessList"))
      {
        tempConfig.processConfig.gameProcessList.clear();
        for (const auto &processInfoJson : processConfigJson["gameProcessList"])
        {
          ProcessInfo processInfo;
          if (processInfoJson.contains("name"))
          {
            processInfo.name = processInfoJson["name"].get<std::string>();
          }
          processInfo.status = processInfoJson.value("status", false);

          if (processInfoJson.contains("processList"))
          {
            for (const auto &process : processInfoJson["processList"])
            {
              if (isValidProcessName(process.get<std::string>()))
              {
                processInfo.processList.push_back(process.get<std::string>());
              }
            }
          }

          if (!processInfo.name.empty() && !processInfo.processList.empty())
          {
            tempConfig.processConfig.gameProcessList.push_back(processInfo);
          }
        }
      }

      // 加载反作弊进程
      if (processConfigJson.contains("antiCheatProcessList"))
      {
        tempConfig.processConfig.antiCheatProcessList.clear();
        for (const auto &processInfoJson : processConfigJson["antiCheatProcessList"])
        {
          ProcessInfo processInfo;
          if (processInfoJson.contains("name"))
          {
            processInfo.name = processInfoJson["name"].get<std::string>();
          }
          processInfo.status = processInfoJson.value("status", false);

          if (processInfoJson.contains("processList"))
          {
            for (const auto &process : processInfoJson["processList"])
            {
              if (isValidProcessName(process.get<std::string>()))
              {
                processInfo.processList.push_back(process.get<std::string>());
              }
            }
          }

          if (!processInfo.name.empty() && !processInfo.processList.empty())
          {
            tempConfig.processConfig.antiCheatProcessList.push_back(processInfo);
          }
        }
      }
    }

    // 验证并应用新配置
    if (validateConfig(tempConfig))
    {
      m_appConfig = std::move(tempConfig);
      if (!configPath.empty())
      {
        m_configPath = configPath;
      }
      LOG_INFO(L"配置文件加载成功: " + pathToLoad);
      return true;
    }
    else
    {
      LOG_ERROR(L"配置文件验证失败");
      m_appConfig = DefaultConfig::Get();
      return false;
    }
  }
  catch (const std::exception &e)
  {
    LOG_ERROR(L"配置文件加载异常: " + MultiByteToWide(e.what(), CP_ACP));
    m_appConfig = DefaultConfig::Get();
    return false;
  }
}

bool ConfigManager::saveConfig(const std::wstring &configPath)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  std::wstring pathToSave = configPath.empty() ? m_configPath : configPath;

  try
  {
    // 创建目录（如果不存在） / create directories if not exist
    auto dirPath = std::filesystem::path(pathToSave).parent_path();
    if (!dirPath.empty() && !std::filesystem::exists(dirPath))
    {
      std::filesystem::create_directories(dirPath);
    }

    // 备份旧文件（如果存在） / backup old file if exists
    if (std::filesystem::exists(pathToSave))
    {
      std::wstring backupPath = pathToSave + L".bak";
      try
      {
        std::filesystem::copy_file(pathToSave, backupPath,
                                   std::filesystem::copy_options::overwrite_existing);
        LOG_INFO(L"Backup configuration file successfully: " + backupPath);
      }
      catch (const std::exception &e)
      {
        LOG_WARN(L"Configuration file backup failed: " + backupPath + L". ErrorMessage: " +
                 MultiByteToWide(e.what(), CP_ACP));
      }
    }

    // 使用 ordered_json 保存配置
    nlohmann::ordered_json appConfigJson;
    nlohmann::ordered_json tmpConfigJson;

    // 保存 version
    tmpConfigJson["version"] = m_appConfig.version;

    // 保存系统信息
    nlohmann::json systemInfoJson;
    systemInfoJson["os"] = m_appConfig.systemInfo.os;
    systemInfoJson["arch"] = m_appConfig.systemInfo.arch;
    systemInfoJson["cpu"] = m_appConfig.systemInfo.cpu;
    tmpConfigJson["systemInfo"] = systemInfoJson;

    // 保存优化配置
    nlohmann::json optimismConfigJson;
    optimismConfigJson["autoStartUp"] = m_appConfig.optimismConfig.autoStartUp;
    optimismConfigJson["autoLimitAntiCheat"] = m_appConfig.optimismConfig.autoLimitAntiCheat;
    // 保存电源计划
    nlohmann::json powerPlanJson;
    powerPlanJson["optimizePowerPlan"] = m_appConfig.optimismConfig.powerPlan.optimizePowerPlan;
    powerPlanJson["lockPowerPlan"] = m_appConfig.optimismConfig.powerPlan.lockPowerPlan;
    powerPlanJson["powerPlanGuid"] = m_appConfig.optimismConfig.powerPlan.powerPlanGuid;
    optimismConfigJson["powerPlan"] = powerPlanJson;

    optimismConfigJson["limitBackgroundActivity"] = m_appConfig.optimismConfig.limitBackgroundActivity;
    optimismConfigJson["optimizeNetworkDelay"] = m_appConfig.optimismConfig.optimizeNetworkDelay;
    optimismConfigJson["optimizeSystemScheduling"] = m_appConfig.optimismConfig.optimizeSystemScheduling;
    optimismConfigJson["optimizeSystemService"] = m_appConfig.optimismConfig.optimizeSystemService;
    tmpConfigJson["optimismConfig"] = optimismConfigJson;

    // 保存进程配置
    nlohmann::ordered_json processConfigJson;

    // 保存游戏进程
    nlohmann::ordered_json gameProcessListJson = nlohmann::ordered_json::array();
    for (const auto &processInfo : m_appConfig.processConfig.gameProcessList)
    {
      nlohmann::json processInfoJson;
      processInfoJson["name"] = processInfo.name;
      processInfoJson["status"] = processInfo.status;
      nlohmann::json processListJson = nlohmann::json::array();
      for (const auto &process : processInfo.processList)
      {
        processListJson.push_back(process);
      }
      processInfoJson["processList"] = processListJson;
      gameProcessListJson.push_back(processInfoJson);
    }
    processConfigJson["gameProcessList"] = gameProcessListJson;

    // 保存反作弊进程
    nlohmann::ordered_json antiCheatProcessListJson = nlohmann::ordered_json::array();
    for (const auto &processInfo : m_appConfig.processConfig.antiCheatProcessList)
    {
      nlohmann::json processInfoJson;
      processInfoJson["name"] = processInfo.name;
      processInfoJson["status"] = processInfo.status;
      nlohmann::json processListJson = nlohmann::json::array();
      for (const auto &process : processInfo.processList)
      {
        processListJson.push_back(process);
      }
      processInfoJson["processList"] = processListJson;
      antiCheatProcessListJson.push_back(processInfoJson);
    }
    processConfigJson["antiCheatProcessList"] = antiCheatProcessListJson;

    tmpConfigJson["processConfig"] = processConfigJson;

    // 将 appConfig 嵌套在外层 JSON 中
    appConfigJson["appConfig"] = tmpConfigJson;

    // 保存 appConfig

    // 以二进制模式打开文件以进行写入
    std::ofstream configFile(pathToSave, std::ios::binary);
    if (!configFile.is_open())
    {
      LOG_ERROR(L"无法打开配置文件以保存: " + pathToSave);
      return false;
    }

    // 向文件中写入 UTF-8 编码的 JSON 数据
    configFile << appConfigJson.dump(4);
    configFile.close();

    if (!configPath.empty())
    {
      m_configPath = configPath;
    }

    LOG_INFO(L"Configuration file saving successfully: " + pathToSave);
    return true;
  }
  catch (const std::exception &e)
  {
    LOG_ERROR(L"Configuration file saving failed: " + MultiByteToWide(e.what(), CP_ACP));
    return false;
  }
}

// 重新加载配置文件
void ConfigManager::reloadConfig()
{
  try
  {
    loadConfig(m_configPath);
  }
  catch (const std::exception &e)
  {
    LOG_ERROR(L"配置文件重新加载异常: " + MultiByteToWide(e.what(), CP_ACP));
  }
}

AppConfig ConfigManager::getConfig()
{
  // 加锁保护 m_appConfig，取决于并发访问模式
  std::lock_guard<std::mutex> lock(m_mutex);
  // 返回配置的副本
  return m_appConfig;
}

bool ConfigManager::setConfig(const AppConfig &config)
{
  // 加锁保护 m_appConfig，取决于并发访问模式
  // std::lock_guard<std::mutex> lock(m_mutex);
  m_appConfig = config;
  if (saveConfig(m_configPath))
  {
    LOG_INFO(L"配置已更新");
    return true;
  }
  else
  {
    LOG_ERROR(L"配置更新失败");
    return false;
  }
}
