/*
 * @Author: vdavidyang vdavidyang@gmail.com
 * @Date: 2025-04-29 21:15:54
 * @LastEditors: vdavidyang vdavidyang@gmail.com
 * @LastEditTime: 2025-05-11 15:14:41
 * @FilePath: \GameOptimizerPro\include\core\registry_manager.h
 * @Description:
 * Copyright (c) 2025 by vdavidyang vdavidyang@gmail.com, All Rights Reserved.
 */

#pragma once

#include <string>
#include <windows.h>

#include "log/logging.h"

/**
 * @class RegistryManager
 * @brief 注册表管理器类，该类负责对注册表的基本操作，包括备份、恢复、创建子键、设置值、删除键和检查键是否存在等
 * @note 该类使用单例模式实现，确保全局只有一个实例
 */
class RegistryManager
{
public:
  RegistryManager();
  ~RegistryManager();

  /**
   * @brief 备份注册表项
   * @param {HKEY} hRoot 根键
   * @param {string} &subKey 子键
   * @param {string} &backupFile 备份文件路径
   * @return {bool} 备份成功返回true，否则返回false
   */
  bool backupRegistryKey(HKEY hRoot, const std::string &subKey, const std::string &backupFile);

  /**
   * @brief 恢复注册表项
   * @param {HKEY} hRoot 根键
   * @param {string} &subKey 子键
   * @param {string} &backupFile 备份文件路径
   * @return {bool} 恢复成功返回true，否则返回false
   */
  bool restoreRegistryKey(HKEY hRoot, const std::string &subKey, const std::string &backupFile);

  /**
   * @brief 创建子注册表项
   * @param {HKEY} hRoot 根键
   * @param {string} &subKey 子键
   * @param {string} &keyName 键名
   * @return {bool} 创建成功返回true，否则返回false
   */
  bool createRegistryKey(HKEY hRoot, const std::string &subKey, const std::string &keyName);

  /**
   * @brief 设置注册表项的DWORD值
   * @param {HKEY} hRoot 根键
   * @param {string} &subKey 子键
   * @param {string} &valueName 值名称
   * @param {DWORD} value 值
   * @return {bool} 设置成功返回true，否则返回false
   */
  bool setRegistryDWORDValue(HKEY hRoot, const std::string &subKey, const std::string &valueName, DWORD value);

  /**
   * @brief 设置注册表项的字符串值
   * @param {HKEY} hRoot 根键
   * @param {string} &subKey 子键
   * @param {string} &valueName 值名称
   * @param {string} &value 值
   * @return {bool} 设置成功返回true，否则返回false
   */
  bool setRegistryStringValue(HKEY hRoot, const std::string &subKey, const std::string &valueName, const std::string &value);

  /**
   * @brief 删除注册表项
   * @param {HKEY} hRoot 根键
   * @param {string} &subKey 子键
   * @param {string} &keyName 键名
   * @return {bool} 删除成功返回true，否则返回false
   */
  bool deleteRegistryKey(HKEY hRoot, const std::string &subKey, const std::string &keyName);

  /**
   * @brief 删除注册表项的值
   * @param {HKEY} hRoot 根键
   * @param {string} &subKey 子键
   * @param {string} &valueName 值名称
   * @return {bool} 删除成功返回true，否则返回false
   */
  bool deleteRegistryValue(HKEY hRoot, const std::string &subKey, const std::string &valueName);

  /**
   * @brief 检查注册表项是否存在
   * @param {HKEY} hRoot 根键
   * @param {string} &subKey 子键
   * @param {string} &keyName 键名
   * @return {bool} 存在返回true，否则返回false
   */
  bool checkRegistryKey(HKEY hRoot, const std::string &subKey, const std::string &keyName);

  /**
   * @brief 获取所有网络接口卡ID
   * @param hRoot 注册表根键
   * @param subKey 注册表子键
   * @param valueName 注册表值名称
   * @param nicIds 存储网络接口卡ID的字符串向量
   * @return 成功时返回true，失败时返回false
   */
  bool GetNetworkInterfaceCardIds(HKEY hRoot, const std::string &subKey, const std::string &valueName, std::vector<std::string> &nicIds);

  /**
   * @brief 在指定的注册表路径下创建DWORD值
   * @param nicId 网络接口卡ID
   * @return 成功时返回true，失败时返回false
   */
  bool CreateRegistryValues(const std::string &nicId);

  /**
   * @brief: 过滤掉无效的网络接口卡ID
   * @param HKEY hRoot 注册表根键
   * @param string &subKey 注册表子键
   * @param vector<std::string> &nicIds 网络接口卡ID数组
   * @return 成功时返回true，失败时返回false
   */
  bool filterInvalidNetworkInterfaceCardIds(HKEY hRoot, const std::string &subKey, std::vector<std::string> &nicIds);
};
