/*
 * @Author: vdavidyang vdavidyang@gmail.com
 * @Date: 2025-05-04 16:01:15
 * @LastEditors: vdavidyang vdavidyang@gmail.com
 * @LastEditTime: 2025-05-04 20:33:44
 * @FilePath: \GameOptimizerPro\include\utils\registry_key.h
 * @Description:
 * Copyright (c) 2025 by vdavidyang vdavidyang@gmail.com, All Rights Reserved.
 */

#pragma once

#include <string>
#include <windows.h>
#include <variant>
#include <vector>

/**
 * @class KeyValue
 * @brief 注册表键值类，负责存储注册表键值的名称和值
 * @note 该类使用 std::variant 来支持不同类型的值
 * @note 支持 DWORD 和 std::string 类型的值
 */
class KeyValue
{
public:
  std::string key;
  std::variant<DWORD, std::string> originValue;
  std::variant<DWORD, std::string> optimizeValue;

  KeyValue() : key(""), originValue(), optimizeValue() {};
  KeyValue(const std::string &valueName, std::variant<DWORD, std::string> originValue, std::variant<DWORD, std::string> optimizeValue)
      : key(valueName), originValue(std::move(originValue)), optimizeValue(std::move(optimizeValue)) {};
};

/**
 * @class RegistryKey
 * @brief 注册表键类，负责存储注册表键的根键、子键和键值列表
 * @note 该类使用 std::vector 来存储多个 KeyValue 对象
 */
class RegistryKey
{
public:
  HKEY hRoot;
  std::string subKey;
  std::vector<KeyValue> keyValueList;

  RegistryKey() : hRoot(nullptr), subKey(""), keyValueList() {}

  RegistryKey(HKEY hRoot, const std::string &subKey, std::vector<KeyValue> valueKeyList)
      : hRoot(hRoot), subKey(subKey), keyValueList(valueKeyList) {}
};
