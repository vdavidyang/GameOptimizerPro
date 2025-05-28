/*
 * @Author: vdavidyang vdavidyang@gmail.com
 * @Date: 2025-04-29 21:17:01
 * @LastEditors: vdavidyang vdavidyang@gmail.com
 * @LastEditTime: 2025-05-28 01:30:59
 * @FilePath: \GameOptimizerPro\src\core\registry_manager.cpp
 * @Description:
 * Copyright (c) 2025 by vdavidyang vdavidyang@gmail.com, All Rights Reserved.
 */
#include "core/registry_manager.h"

RegistryManager::RegistryManager()
{
}

RegistryManager::~RegistryManager()
{
}

bool RegistryManager::backupRegistryKey(HKEY hRoot, const std::string &subKey, const std::string &backupFile)
{
  HKEY hKey;
  LONG result = RegOpenKeyExA(hRoot, subKey.c_str(), 0, KEY_READ, &hKey);
  if (result != ERROR_SUCCESS)
  {
    LOG_HRESULT("打开注册表项失败: " + subKey + " 错误: ", HRESULT_FROM_WIN32(result));
    return false;
  }

  result = RegSaveKeyA(hKey, backupFile.c_str(), NULL);
  RegCloseKey(hKey);

  if (result != ERROR_SUCCESS)
  {
    LOG_HRESULT("备份注册表失败: " + subKey + " 错误: ", HRESULT_FROM_WIN32(result));
    return false;
  }
  return true;
}

bool RegistryManager::restoreRegistryKey(HKEY hRoot, const std::string &subKey, const std::string &backupFile)
{
  HKEY hKey;
  LONG result = RegOpenKeyExA(hRoot, subKey.c_str(), 0, KEY_WRITE, &hKey);
  if (result != ERROR_SUCCESS)
  {
    LOG_HRESULT("打开注册表项失败: " + subKey + " 错误: ", HRESULT_FROM_WIN32(result));
    return false;
  }

  result = RegRestoreKeyA(hKey, backupFile.c_str(), 0);
  RegCloseKey(hKey);

  if (result != ERROR_SUCCESS)
  {
    LOG_HRESULT("恢复注册表失败: " + backupFile + " 错误: ", HRESULT_FROM_WIN32(result));
    return false;
  }
  return true;
}

bool RegistryManager::createRegistryKey(HKEY hRoot, const std::string &subKey, const std::string &keyName)
{
  HKEY hKey;
  LONG result = RegOpenKeyExA(hRoot, subKey.c_str(), 0, KEY_WRITE, &hKey);
  if (result != ERROR_SUCCESS)
  {
    LOG_HRESULT("打开父项失败: " + subKey + " 错误: ", HRESULT_FROM_WIN32(result));
    return false;
  }
  std::string keyPath = subKey + "\\" + keyName;
  result = RegCreateKeyExA(hRoot, keyPath.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);
  RegCloseKey(hKey);
  if (result != ERROR_SUCCESS)
  {
    LOG_HRESULT("创建注册表项失败: " + keyPath + " 错误: ", HRESULT_FROM_WIN32(result));
    return false;
  }
  return true;
}

bool RegistryManager::setRegistryDWORDValue(HKEY hRoot, const std::string &subKey, const std::string &valueName, DWORD value)
{
  HKEY hKey;
  LONG result = RegOpenKeyExA(hRoot, subKey.c_str(), 0, KEY_WRITE, &hKey);
  if (result != ERROR_SUCCESS)
  {
    LOG_HRESULT("打开注册表项失败: " + subKey + " 错误: ", HRESULT_FROM_WIN32(result));
    return false;
  }

  result = RegSetValueExA(hKey, valueName.c_str(), 0, REG_DWORD, reinterpret_cast<const BYTE *>(&value), sizeof(value));
  RegCloseKey(hKey);

  if (result != ERROR_SUCCESS)
  {
    LOG_HRESULT("设置注册表值失败: " + valueName + " 错误: ", HRESULT_FROM_WIN32(result));
    return false;
  }
  return true;
}

bool RegistryManager::setRegistryStringValue(HKEY hRoot, const std::string &subKey, const std::string &valueName, const std::string &value)
{
  HKEY hKey;
  LONG result = RegOpenKeyExA(hRoot, subKey.c_str(), 0, KEY_WRITE, &hKey);
  if (result != ERROR_SUCCESS)
  {
    LOG_HRESULT("打开注册表项失败: " + subKey + " 错误: ", HRESULT_FROM_WIN32(result));
    return false;
  }

  result = RegSetValueExA(hKey, valueName.c_str(), 0, REG_SZ, reinterpret_cast<const BYTE *>(value.c_str()), static_cast<DWORD>(value.size()));
  RegCloseKey(hKey);

  if (result != ERROR_SUCCESS)
  {
    LOG_HRESULT("设置注册表值失败: " + valueName + " 错误: ", HRESULT_FROM_WIN32(result));
    return false;
  }
  return true;
}

bool RegistryManager::deleteRegistryKey(HKEY hRoot, const std::string &subKey, const std::string &keyName)
{
  // 打开父注册表项
  std::string keyPath = subKey + keyName;
  HKEY hParentKey;
  LONG result = RegOpenKeyExA(
      hRoot,
      keyPath.c_str(),
      0,
      KEY_ALL_ACCESS,
      &hParentKey);

  if (result != ERROR_SUCCESS)
  {
    LOG_HRESULT("打开注册表项失败: " + keyPath + " 错误: ", HRESULT_FROM_WIN32(result));
    return false;
  }

  // 递归删除子项（兼容旧版Windows）
  result = RegDeleteTreeA(hParentKey, nullptr);
  if (result != ERROR_SUCCESS)
  {
    RegCloseKey(hParentKey);
    LOG_HRESULT("删除注册表项失败: " + subKey + " 错误: ", HRESULT_FROM_WIN32(result));
    return false;
  }

  RegCloseKey(hParentKey);

  // 删除目标项自身
  result = RegDeleteKeyExA(
      hRoot,
      keyPath.c_str(),
      KEY_WOW64_64KEY, // 显式指定64位视图
      0);

  return (result == ERROR_SUCCESS);
}

bool RegistryManager::deleteRegistryValue(HKEY hRoot, const std::string &subKey, const std::string &valueName)
{
  HKEY hKey;
  LONG result = RegOpenKeyExA(hRoot, subKey.c_str(), 0, KEY_WRITE, &hKey);
  if (result != ERROR_SUCCESS)
  {
    LOG_HRESULT("打开注册表项失败: " + subKey + " 错误: ", HRESULT_FROM_WIN32(result));
    return false;
  }

  result = RegDeleteValueA(hKey, valueName.c_str());
  RegCloseKey(hKey);

  if (result != ERROR_SUCCESS)
  {
    LOG_HRESULT("删除注册表值失败: " + valueName + " 错误: ", HRESULT_FROM_WIN32(result));
    return false;
  }
  return true;
}

bool RegistryManager::checkRegistryKey(HKEY hRoot, const std::string &subKey, const std::string &keyName)
{
  HKEY hKey;
  std::string keyPath = subKey + keyName;
  LONG result = RegOpenKeyExA(hRoot, keyPath.c_str(), 0, KEY_READ, &hKey);
  RegCloseKey(hKey);
  if (result != ERROR_SUCCESS)
  {
    // LOG_HRESULT("打开注册表项失败: " + keyPath + " 错误: ", HRESULT_FROM_WIN32(result));
    return false;
  }
  return true;
}

bool RegistryManager::GetNetworkInterfaceCardIds(HKEY hRoot, const std::string &subKey, const std::string &valueName, std::vector<std::string> &nicIds)
{
  HKEY hKey;

  if (RegOpenKeyExA(hRoot, subKey.c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS)
  {
    LOG_ERROR("无法打开注册表项: " + subKey);
    return false;
  }

  DWORD index = 0;
  DWORD dataType;
  CHAR subKeyName[MAX_PATH];

  while (RegEnumKeyA(hKey, index++, subKeyName, MAX_PATH) == ERROR_SUCCESS)
  {
    HKEY hSubKey;
    std::string fullSubKeyPath = subKey + "\\" + subKeyName;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, fullSubKeyPath.c_str(), 0, KEY_READ, &hSubKey) == ERROR_SUCCESS)
    {
      CHAR valueData[MAX_PATH];
      DWORD dataSize = sizeof(valueData);
      if (RegQueryValueExA(hSubKey, valueName.c_str(), nullptr, &dataType, reinterpret_cast<LPBYTE>(valueData), &dataSize) == ERROR_SUCCESS)
      {
        nicIds.push_back(valueData);
      }
      RegCloseKey(hSubKey);
    }
  }

  RegCloseKey(hKey);

  if (nicIds.empty())
  {
    LOG_ERROR("未找到有效的网络接口ID");
    return false;
  }

  return true;
}

bool RegistryManager::filterInvalidNetworkInterfaceCardIds(HKEY hRoot, const std::string &subKey, std::vector<std::string> &nicIds)
{
  std::vector<std::string> validNicIds;
  for (const auto &nicId : nicIds)
  {
    if (checkRegistryKey(hRoot, subKey, nicId))
    {
      validNicIds.push_back(nicId);
    }
  }
  nicIds = std::move(validNicIds);
  return !nicIds.empty();
}
