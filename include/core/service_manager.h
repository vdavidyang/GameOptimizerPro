/*
 * @Author: vdavidyang vdavidyang@gmail.com
 * @Date: 2025-05-11 21:08:36
 * @LastEditors: vdavidyang vdavidyang@gmail.com
 * @LastEditTime: 2025-05-12 00:08:49
 * @FilePath: \GameOptimizerPro\include\core\service_manager.h
 * @Description:
 * Copyright (c) 2025 by vdavidyang vdavidyang@gmail.com, All Rights Reserved.
 */

#pragma once

#include <windows.h>
#include <string>
#include <map>
#include <aclapi.h> // For EXPLICIT_ACCESS_W and related functions

#include "utils/system_utils.h"
#include "log/logging.h"

class ServiceManager
{
public:
  /**
   * @brief 启动指定Windows服务
   * @param serviceName 需要启动的服务名称
   * @return 操作成功返回true，失败返回false
   */
  // 启动指定Windows服务
  bool startService(const std::wstring &serviceName);

  /**
   * @brief 停止指定的Windows服务
   * @param serviceName 需要停止的服务名称
   * @return 操作成功返回true，失败返回false
   */
  bool stopService(const std::wstring &serviceName);

  /**
   * @brief 设置指定Windows服务的启动类型
   * @param serviceName 服务名称
   * @param startType 启动类型 (0-自动启动, 1-自动（延迟启动）, 2-手动启动, 3-禁用)
   * @return 操作成功返回true，失败返回false
   */
  bool setServiceStartType(const std::wstring &serviceName, DWORD startType);

  /**
   * @brief 查询指定Windows服务的状态和启动类型
   * @param serviceName 需要查询的服务名称
   * @param currentStatus 当前服务状态 (true-运行中, false-已停止)
   * @param startType 服务启动类型 (0-自动启动, 1-自动（延迟启动）, 2-手动启动, 3-禁用)
   * @return 操作成功返回true，失败返回false
   */
  bool queryServiceStatus(const std::wstring &serviceName, bool &currentStatus, DWORD &startType);

private:
  // 服务启动类型映射表
  const std::vector<std::pair<DWORD, std::wstring>> m_startTypeList = {
      {SERVICE_AUTO_START, L"auto"},
      {SERVICE_AUTO_START, L"auto-delayed"},
      {SERVICE_DEMAND_START, L"demand"},
      {SERVICE_DISABLED, L"disabled"}};

  /**
   * @brief 修改服务的安全描述符以获取访问权限
   * @param service 已打开的服务句柄
   * @return 修改成功返回true，失败返回false
   */
  bool SetServiceSecurityWrapper(SC_HANDLE service);
};