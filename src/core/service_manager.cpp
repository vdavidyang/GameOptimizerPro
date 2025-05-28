/*
 * @Author: vdavidyang vdavidyang@gmail.com
 * @Date: 2025-05-11 21:08:23
 * @LastEditors: vdavidyang vdavidyang@gmail.com
 * @LastEditTime: 2025-05-12 01:45:13
 * @FilePath: \GameOptimizerPro\src\core\service_manager.cpp
 * @Description:
 * Copyright (c) 2025 by vdavidyang vdavidyang@gmail.com, All Rights Reserved.
 */

#include "core/service_manager.h"

bool ServiceManager::startService(const std::wstring &serviceName)
{
  SC_HANDLE scm = OpenSCManagerW(
      nullptr,
      nullptr,
      SC_MANAGER_CONNECT);

  if (!scm)
  {
    LOG_HRESULT(L"无法打开服务控制管理器", GetLastError());
    return false;
  }

  SC_HANDLE service = OpenServiceW(
      scm,
      serviceName.c_str(),
      SERVICE_START);

  if (!service)
  {
    DWORD error = GetLastError();
    if (error == ERROR_SERVICE_DOES_NOT_EXIST)
    {
      LOG_HRESULT(L"服务不存在: " + serviceName, error);
    }
    else
    {
      LOG_HRESULT(L"无法打开服务: " + serviceName, error);
    }
    CloseServiceHandle(scm);
    return false;
  }

  if (!::StartService(service, 0, nullptr))
  {
    DWORD error = GetLastError();
    if (error == ERROR_SERVICE_ALREADY_RUNNING)
    {
      LOG_HRESULT(L"服务已在运行: " + serviceName, error);
    }
    else
    {
      LOG_HRESULT(L"服务启动失败: " + serviceName, error);
      CloseServiceHandle(service);
      CloseServiceHandle(scm);
      return false;
    }
  }
  else
  {
    LOG_INFO(L"服务启动成功: " + serviceName);
  }

  CloseServiceHandle(service);
  CloseServiceHandle(scm);
  return true;
}

bool ServiceManager::stopService(const std::wstring &serviceName)
{
  SC_HANDLE scm = OpenSCManagerW(
      nullptr,              // 本地计算机
      nullptr,              // 默认数据库
      SC_MANAGER_ALL_ACCESS // 需要完全控制权限
  );

  if (!scm)
  {
    LOG_HRESULT(L"无法打开服务控制管理器", GetLastError());
    return false;
  }

  SC_HANDLE service = OpenServiceW(
      scm,
      serviceName.c_str(),
      SERVICE_STOP | SERVICE_QUERY_STATUS);

  if (!service)
  {
    DWORD error = GetLastError();
    if (error == ERROR_SERVICE_DOES_NOT_EXIST)
    {
      LOG_HRESULT(L"服务不存在: " + serviceName, error);
    }
    else
    {
      LOG_HRESULT(L"无法打开服务: " + serviceName, error);
    }
    CloseServiceHandle(scm);
    return false;
  }

  SERVICE_STATUS status;
  if (!ControlService(service, SERVICE_CONTROL_STOP, &status))
  {
    LOG_HRESULT(L"服务停止失败: " + serviceName, GetLastError());
    CloseServiceHandle(service);
    CloseServiceHandle(scm);
    return false;
  }

  LOG_INFO(L"服务停止成功: " + serviceName);
  CloseServiceHandle(service);
  CloseServiceHandle(scm);
  return true;
}

bool ServiceManager::SetServiceSecurityWrapper(SC_HANDLE service)
{
  EXPLICIT_ACCESS_W ea = {0};
  SECURITY_DESCRIPTOR sd = {0};
  PACL oldDacl = nullptr;
  PACL newDacl = nullptr;

  // 创建管理员组SID
  PSID adminSid = nullptr;
  SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
  if (!AllocateAndInitializeSid(
          &ntAuthority,
          2,
          SECURITY_BUILTIN_DOMAIN_RID,
          DOMAIN_ALIAS_RID_ADMINS,
          0, 0, 0, 0, 0, 0,
          &adminSid))
  {
    return false;
  }

  // 获取原有DACL
  if (GetSecurityInfo(
          service,
          SE_SERVICE,
          DACL_SECURITY_INFORMATION,
          nullptr,
          nullptr,
          &oldDacl,
          nullptr,
          nullptr) != ERROR_SUCCESS)
  {
    return false;
  }

  // 设置新权限
  ea.grfAccessPermissions = SERVICE_ALL_ACCESS;
  ea.grfAccessMode = GRANT_ACCESS;
  ea.grfInheritance = NO_INHERITANCE;
  ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
  ea.Trustee.TrusteeType = TRUSTEE_IS_GROUP;
  ea.Trustee.ptstrName = (LPWSTR)adminSid;

  // 合并ACL
  if (SetEntriesInAclW(1, &ea, oldDacl, &newDacl) != ERROR_SUCCESS)
  {
    LocalFree(oldDacl);
    return false;
  }

  // 应用新DACL
  bool result = true;
  if (SetSecurityInfo(
          service,
          SE_SERVICE,
          DACL_SECURITY_INFORMATION,
          nullptr,
          nullptr,
          newDacl,
          nullptr) != ERROR_SUCCESS)
  {
    result = false;
  }

  // 清理资源
  LocalFree(oldDacl);
  LocalFree(newDacl);
  return result;
}

bool ServiceManager::setServiceStartType(const std::wstring &serviceName, DWORD dwStartType)
{
  // 检查启动类型是否合法
  if (dwStartType < 0 || dwStartType > 3)
  {
    LOG_ERROR(L"无效的启动类型: " + std::to_wstring(dwStartType));
    return false;
  }

  // 打开服务控制管理器
  SC_HANDLE scm = OpenSCManagerW(
      nullptr,
      nullptr,
      SC_MANAGER_ALL_ACCESS);

  if (!scm)
  {
    LOG_HRESULT(L"无法打开服务控制管理器", GetLastError());
    return false;
  }

  SC_HANDLE service = OpenServiceW(
      scm,
      serviceName.c_str(),
      SERVICE_CHANGE_CONFIG | WRITE_DAC);

  if (!service)
  {

    DWORD error = GetLastError();
    // 详细的权限错误处理
    if (error == ERROR_ACCESS_DENIED)
    {
      // 尝试使用系统命令绕过API限制
      LOG_INFO(L"尝试使用系统命令修改基础启动类型");
      std::wstring command = L"sc config \"" + serviceName +
                             L"\" start= " + m_startTypeList[dwStartType].second;
      LOG_INFO(L"执行命令：" + command);
      if (runCommandFromCP(command))
      {
        LOG_INFO(L"服务启动类型设置成功: " + serviceName + L"-" + m_startTypeList[dwStartType].second);
        CloseServiceHandle(service);
        CloseServiceHandle(scm);
        return true;
      }
      LOG_HRESULT(L"操作被拒绝，请以管理员权限运行程序", error);
    }
    else if (error == ERROR_SERVICE_DOES_NOT_EXIST)
    {
      LOG_HRESULT(L"服务不存在: " + serviceName, error);
    }
    else
    {
      LOG_HRESULT(L"无法打开服务: " + serviceName, error);
    }
    CloseServiceHandle(scm);
    return false;
  }

  // 安全描述符修改（针对受保护服务）
  if (!SetServiceSecurityWrapper(service))
  {
    LOG_WARN(L"安全描述符修改失败，继续尝试设置启动类型...");
  }

  // 第一步：设置基础启动类型
  if (!ChangeServiceConfigW(
          service,
          SERVICE_NO_CHANGE,
          m_startTypeList[dwStartType].first,
          SERVICE_NO_CHANGE,
          nullptr,
          nullptr,
          nullptr,
          nullptr,
          nullptr,
          nullptr,
          nullptr))
  {
    LOG_HRESULT(L"修改基础启动类型失败: " + serviceName, GetLastError());
    // 尝试使用系统命令绕过API限制
    LOG_INFO(L"尝试使用系统命令修改基础启动类型");
    std::wstring command = L"sc config \"" + serviceName +
                           L"\" start= " + std::to_wstring(dwStartType);

    if (runCommandFromCP(command))
    {
      LOG_INFO(L"服务启动类型设置成功: " + serviceName + L"-" + m_startTypeList[dwStartType].second);
      CloseServiceHandle(service);
      CloseServiceHandle(scm);
      return true;
    }
    CloseServiceHandle(service);
    CloseServiceHandle(scm);
    return false;
  }

  // 第二步：如果是自动启动，需要额外设置
  if (dwStartType == 0)
  {
    // 设置不延迟启动
    SERVICE_DELAYED_AUTO_START_INFO delayInfo = {FALSE};
    if (!ChangeServiceConfig2(
            service,
            SERVICE_CONFIG_DELAYED_AUTO_START_INFO,
            &delayInfo))
    {
      DWORD err = GetLastError();
      if (err == ERROR_ACCESS_DENIED)
      {
        LOG_HRESULT(L"需要管理员权限", err);
      }
      else if (err == ERROR_INVALID_LEVEL)
      {
        LOG_HRESULT(L"系统不支持延迟启动", err);
      }
      else
      {
        LOG_HRESULT(L"设置延迟启动失败", err);
      }
      CloseServiceHandle(service);
      CloseServiceHandle(scm);
      return false;
    }
  }
  // 第三步：如果是延迟启动，需要额外设置
  else if (dwStartType == 1)
  {
    // 设置延迟启动
    SERVICE_DELAYED_AUTO_START_INFO delayInfo = {TRUE};
    if (!ChangeServiceConfig2(
            service,
            SERVICE_CONFIG_DELAYED_AUTO_START_INFO,
            &delayInfo))
    {
      DWORD err = GetLastError();
      if (err == ERROR_ACCESS_DENIED)
      {
        LOG_HRESULT(L"需要管理员权限", err);
      }
      else if (err == ERROR_INVALID_LEVEL)
      {
        LOG_HRESULT(L"系统不支持延迟启动", err);
      }
      else
      {
        LOG_HRESULT(L"设置延迟启动失败", err);
      }
      CloseServiceHandle(service);
      CloseServiceHandle(scm);
      return false;
    }
  }

  LOG_INFO(L"服务启动类型设置成功: " + serviceName + L"-" + m_startTypeList[dwStartType].second);
  CloseServiceHandle(service);
  CloseServiceHandle(scm);
  return true;
}

bool ServiceManager::queryServiceStatus(const std::wstring &serviceName, bool &currentStatus, DWORD &startType)
{
  // 打开服务控制管理器
  // SC_MANAGER_CONNECT 连接到服务控制管理器
  SC_HANDLE scm = OpenSCManagerW(
      nullptr,
      nullptr,
      SC_MANAGER_CONNECT);

  if (!scm)
  {
    LOG_HRESULT(L"无法打开服务控制管理器", GetLastError());
    return false;
  }

  SC_HANDLE service = OpenServiceW(
      scm,
      serviceName.c_str(),
      SERVICE_QUERY_STATUS | SERVICE_QUERY_CONFIG);

  if (!service)
  {
    LOG_HRESULT(L"无法打开服务: " + serviceName, GetLastError());
    CloseServiceHandle(scm);
    return false;
  }

  // 查询服务状态
  SERVICE_STATUS status;
  if (QueryServiceStatus(service, &status))
  {
    currentStatus = (status.dwCurrentState == SERVICE_RUNNING);
    LOG_INFO(L"服务状态: " + serviceName + L"-" +
             (status.dwCurrentState == SERVICE_RUNNING ? L"运行中" : L"已停止"));
  }

  // 查询启动类型
  LPQUERY_SERVICE_CONFIG config = {};
  DWORD bytesNeeded = 0;
  QueryServiceConfig(service, nullptr, 0, &bytesNeeded);
  config = (LPQUERY_SERVICE_CONFIG)LocalAlloc(LPTR, bytesNeeded);

  if (!QueryServiceConfig(service, config, bytesNeeded, &bytesNeeded))
  {
    LOG_HRESULT(L"查询服务启动类型失败: " + serviceName, GetLastError());
    return false;
  }

  if (config->dwStartType == SERVICE_AUTO_START)
  {
    startType = 0;
  }
  else if (config->dwStartType == SERVICE_AUTO_START)
  {
    startType = 1;
  }
  else if (config->dwStartType == SERVICE_DEMAND_START)
  {
    startType = 2;
  }
  else if (config->dwStartType == SERVICE_DISABLED)
  {
    startType = 3;
  }
  else
  {
    LOG_HRESULT(L"未知的服务启动类型: " + serviceName, GetLastError());
    return false;
  }

  LocalFree(config);
  CloseServiceHandle(service);
  CloseServiceHandle(scm);
  return true;
}
