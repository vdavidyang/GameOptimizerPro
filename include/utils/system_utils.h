/*
 * @Author: vdavidyang vdavidyang@gmail.com
 * @Date: 2025-04-21 20:34:54
 * @LastEditors: vdavidyang vdavidyang@gmail.com
 * @LastEditTime: 2025-05-27 17:57:39
 * @FilePath: \GameOptimizerPro\include\utils\system_utils.h
 * @Description:
 * Copyright (c) 2025 by vdavidyang vdavidyang@gmail.com, All Rights Reserved.
 */

#pragma once

#include <string>
#include <windows.h>

#include "log\logging.h"

/**
 * @brief 检查进程名是否合法
 * @param {wstring} &processName 进程名
 * @return {bool} 是否合法
 */
bool isValidProcessName(const std::string &processName);

/**
 * @brief 将 MultiByte (char*) 字符串转换为 WideChar (wchar_t*)
 * @param {string} &str 字符串
 * @param {UINT} codePage 编码
 * @return {wstring} 宽字符串
 */
std::wstring MultiByteToWide(const std::string &str, UINT codePage = CP_UTF8);

/**
 * @brief 将 WideChar (wchar_t*) 字符串转换为 MultiByte (char*)
 * @param {wstring} &wstr 宽字符串
 * @param {UINT} codePage 编码
 * @return {string} 字符串
 */
std::string WideToMultiByte(const std::wstring &wstr, UINT codePage = CP_UTF8);

/**
 * @brief 将 ANSI (char*) 字符串转换为 WideChar (wchar_t*)
 * @param {string} &str 字符串
 * @return {wstring} 宽字符串
 */
std::wstring AnsiToWide(const std::string &str);

/**
 * @brief 检查是否为管理员
 * @return {bool} 是否为管理员
 */
bool isAdmin();

/**
 * @brief 请求管理员权限
 */
void requestAdminPrivileges();

/**
 * @brief 检查是否开机自启动
 * @return {bool} 是否开机自启动
 */
bool isStartupEnabled();

/**
 * @brief 将GUID转换为字符串
 * @param {GUID*} guid GUID
 * @return {std::string} 字符串
 * @details 字符串格式为"{XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX}"
 */
std::string GuidToString(const GUID *guid);

/**
 * @brief 将字符串转换为GUID
 * @param {std::string} guidString GUID字符串
 * @return {GUID} GUID
 * @attention 字符串格式为"{XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX}"
 */
GUID StringToGuid(const std::string &guidString);

/**
 * @brief 通过 ShellExecute 运行命令
 * @param command 命令
 * @return bool 运行结果
 */
bool runCommandFormSE(const std::wstring &command);

/**
 * @brief 通过 CreateProcess 函数运行命令
 * @param command 命令
 * @return bool 运行结果
 */
bool runCommandFromCP(const std::wstring &command);

/**
 * @brief 通过 PowerShell 运行命令
 * @param command 命令
 * @return std::wstring 运行结果
 */
std::wstring RunPowerShellCommand(const std::wstring &command);

/**
 * @brief 设置进程的优先级和 CPU 亲和性
 * @param processName 进程名
 * @param priority 优先级
 * @param affinityMask CPU 亲和性掩码
 * @return bool 是否成功
 */
bool SetProcessPriorityAndAffinity(const std::wstring &processName, const std::wstring &priority, DWORD_PTR affinityMask);