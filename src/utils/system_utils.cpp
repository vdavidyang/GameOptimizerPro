/*
 * @Author: vdavidyang vdavidyang@gmail.com
 * @Date: 2025-04-21 20:35:01
 * @LastEditors: vdavidyang vdavidyang@gmail.com
 * @LastEditTime: 2025-05-27 18:12:51
 * @FilePath: \GameOptimizerPro\src\utils\system_utils.cpp
 * @Description:
 * Copyright (c) 2025 by vdavidyang vdavidyang@gmail.com, All Rights Reserved.
 */

#include "utils/system_utils.h"

bool isValidProcessName(const std::string &processName)
{
  if (processName.empty() || processName.length() > MAX_PATH)
  {
    return false;
  }
  // 检查是否包含无效字符
  return processName.find_first_of("\\/:*?\"<>|") == std::string::npos;
}

std::wstring MultiByteToWide(const std::string &str, UINT codePage)
{
  // 注意：这里假设源是 UTF-8，如果不是，需要修改 CodePage (CP_UTF8)
  // 如果是来自 exception::what()，它通常是系统默认 ANSI 编码页 (CP_ACP)

  if (str.empty())
  {
    return std::wstring();
  }
  int size_needed = MultiByteToWideChar(codePage, 0, &str[0], (int)str.size(), nullptr, 0);
  std::wstring wstrTo(size_needed, 0);
  MultiByteToWideChar(codePage, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
  return wstrTo;
}

std::string WideToMultiByte(const std::wstring &wstr, UINT codePage)
{
  if (wstr.empty())
  {
    return std::string();
  }
  int size_needed = WideCharToMultiByte(codePage, 0, &wstr[0], (int)wstr.size(), nullptr, 0, nullptr, nullptr);
  std::string strTo(size_needed, 0);
  WideCharToMultiByte(codePage, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, nullptr, nullptr);
  return strTo;
}

std::wstring AnsiToWide(const std::string &str)
{
  int size_needed = MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.size(), nullptr, 0);
  std::wstring wstr(size_needed, 0);
  MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.size(), &wstr[0], size_needed);
  return wstr;
}

bool isAdmin()
{
  BOOL isAdmin = FALSE;
  SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
  PSID AdminGroup;
  if (AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &AdminGroup))
  {
    if (!CheckTokenMembership(NULL, AdminGroup, &isAdmin))
    {
      isAdmin = FALSE;
    }
    FreeSid(AdminGroup);
  }
  return isAdmin;
}

void requestAdminPrivileges()
{
  if (isAdmin())
  {
    LOG_INFO(L"已具备管理员权限");
    return;
  }
  LOG_WARN(L"请求提权");
  wchar_t path[MAX_PATH];
  if (GetModuleFileNameW(NULL, path, MAX_PATH))
  {
    SHELLEXECUTEINFOW sei = {sizeof(sei)};
    sei.lpVerb = L"runas";
    sei.lpFile = path;
    sei.hwnd = NULL;
    sei.nShow = SW_NORMAL;
    if (!ShellExecuteExW(&sei))
    {
      LOG_ERROR(L"提权失败: " + std::to_wstring(GetLastError()));
    }
  }
}

bool isAutoStartup()
{
  HKEY hKey;
  if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
  {
    wchar_t path[MAX_PATH];
    DWORD pathSize = static_cast<DWORD>(sizeof(path));
    if (RegQueryValueExW(hKey, L"GameOptimizerPro", NULL, NULL, (LPBYTE)path, &pathSize) == ERROR_SUCCESS)
    {
      RegCloseKey(hKey);
      return true;
    }
    RegCloseKey(hKey);
  }
  return false;
}

std::string GuidToString(const GUID *guid)
{
  // "{XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX}\\0"
  char buffer[39];
  sprintf_s(buffer, "{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
            guid->Data1, guid->Data2, guid->Data3,
            guid->Data4[0], guid->Data4[1], guid->Data4[2], guid->Data4[3],
            guid->Data4[4], guid->Data4[5], guid->Data4[6], guid->Data4[7]);
  return std::string(buffer);
}

GUID StringToGuid(const std::string &guidString)
{
  // 例如："8c5e7fda-e8bf-4a96-9a85-a6e23a8c635c"
  GUID guid;
  if (sscanf_s(guidString.c_str(), "{%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}",
               &guid.Data1, &guid.Data2, &guid.Data3,
               &guid.Data4[0], &guid.Data4[1], &guid.Data4[2], &guid.Data4[3],
               &guid.Data4[4], &guid.Data4[5], &guid.Data4[6], &guid.Data4[7]) == 11)
  {
    return guid;
  }
  return GUID_NULL;
}

bool runCommandFromSE(const std::wstring &command)
{
  SHELLEXECUTEINFOW sei = {sizeof(sei)};
  sei.lpVerb = L"runas";
  sei.lpFile = L"cmd.exe";
  sei.lpParameters = (L"/c " + command).c_str();
  sei.nShow = SW_HIDE;

  if (!ShellExecuteExW(&sei))
  {
    DWORD err = GetLastError();
    if (err == ERROR_CANCELLED)
    {
      LOG_WARN(L"用户拒绝了 UAC 提权请求");
    }
    else
    {
      LOG_HRESULT(L"Shell执行失败: ", err);
    }
    return false;
  }
  return true;
}

bool runCommandFromCP(const std::wstring &command)
{
  STARTUPINFOW si = {sizeof(STARTUPINFOW)};
  PROCESS_INFORMATION pi;

  if (!CreateProcessW(
          NULL,                                // 不指定可执行文件（使用命令行）
          const_cast<LPWSTR>(command.c_str()), // 要执行的命令
          NULL,                                // 进程安全属性
          NULL,                                // 线程安全属性
          FALSE,                               // 不继承句柄
          CREATE_NO_WINDOW,                    // 不显示控制台窗口（静默执行）
          NULL,                                // 使用当前环境变量
          NULL,                                // 使用当前目录
          &si,                                 // 启动信息
          &pi                                  // 进程信息
          ))
  {
    LOG_ERROR(L"Failed to run shell command as admin.");
    return false;
  }

  // 等待命令执行完成
  WaitForSingleObject(pi.hProcess, INFINITE);

  // 关闭句柄
  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);
  return true;
}

std::wstring RunPowerShellCommand(const std::wstring &command)
{
  std::wstring result;
  SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
  HANDLE hReadPipe, hWritePipe;

  if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0))
  {
    return L"Error creating pipe";
  }

  STARTUPINFOW si = {sizeof(STARTUPINFOW)};
  si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
  si.wShowWindow = SW_HIDE;
  si.hStdOutput = hWritePipe;
  si.hStdError = hWritePipe;

  PROCESS_INFORMATION pi;
  std::wstring fullCommand = L"powershell.exe -NoProfile -ExecutionPolicy Bypass -Command \"" + command + L"\"";

  if (!CreateProcessW(NULL, const_cast<LPWSTR>(fullCommand.c_str()), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
  {
    CloseHandle(hReadPipe);
    CloseHandle(hWritePipe);
    return L"Error creating process";
  }

  CloseHandle(hWritePipe);

  char buffer[4096];
  DWORD bytesRead;
  while (true)
  {
    if (!ReadFile(hReadPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL) || bytesRead == 0)
    {
      break;
    }
    buffer[bytesRead] = '\0';
    int wideLen = MultiByteToWideChar(CP_UTF8, 0, buffer, -1, NULL, 0);
    std::vector<wchar_t> wideBuffer(wideLen);
    MultiByteToWideChar(CP_UTF8, 0, buffer, -1, wideBuffer.data(), wideLen);
    result += wideBuffer.data();
  }

  WaitForSingleObject(pi.hProcess, INFINITE);
  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);
  CloseHandle(hReadPipe);

  return result;
}

bool SetProcessPriorityAndAffinity(const std::wstring &processName, const std::wstring &priority, DWORD_PTR affinityMask)
{
  // 修改PowerShell脚本以包含更多调试信息
  std::wstring command = L"function SetProcessPriorityAndAffinity { ";
  command += L"param($processName, $priority, $affinityMask); ";
  command += L"try { ";
  command += L"$processes = Get-Process -Name $processName -ErrorAction Stop; ";
  command += L"foreach ($p in $processes) { ";
  command += L"$p.PriorityClass = [System.Diagnostics.ProcessPriorityClass]::$priority; ";
  command += L"$p.ProcessorAffinity = $affinityMask; ";
  command += L"} return 'True';";
  command += L"} catch { return 'Error: ' + $_.Exception.Message; }}";
  command += L" SetProcessPriorityAndAffinity -processName " + processName + L" -priority " + priority + L" -affinityMask " + std::to_wstring(affinityMask) + L";";

  std::wstring result = RunPowerShellCommand(command);
  if (result.find(L"True") != std::wstring::npos)
  {
    return true;
  }
  else
  {
    LOG_ERROR(L"Failed to set process priority and affinity: " + result);
    return false;
  }
}