/*
 * @Author: vdavidyang vdavidyang@gmail.com
 * @Date: 2025-04-30 21:46:24
 * @LastEditors: vdavidyang vdavidyang@gmail.com
 * @LastEditTime: 2025-05-04 19:30:21
 * @FilePath: \GameOptimizerPro\include\core\power_manager.h
 * @Description:
 * Copyright (c) 2025 by vdavidyang vdavidyang@gmail.com, All Rights Reserved.
 */

#pragma once

#include <windows.h>
#include <powrprof.h>
#include <tchar.h>
#include <vector>

#include "log/logging.h"

#pragma comment(lib, "powrprof.lib")

/**
 * @class PowerManager
 * @brief 电源管理器类，负责创建、优化和删除电源计划，以及设置活动电源计划等功能
 * @note 该类使用单例模式实现，确保全局只有一个实例
 */
class PowerManager
{
public:
    PowerManager();
    ~PowerManager();

    /**
     * @brief 创建电源计划
     * @param {GUID*} newPlanGuid 新的电源计划GUID
     * @param {const TCHAR*} planName 电源计划名称
     * @param {const TCHAR*} planDescription 电源计划描述
     * @return {bool} 创建成功返回true，否则返回false
     * @attention 创建电源计划时，需要电脑有高性能电源计划
     */
    bool createPowerPlan(GUID *newPlanGuid, const TCHAR *planName, const TCHAR *planDescription);

    /**
     * @brief 优化电源计划
     * @param {GUID*} powerPlanGuid 电源计划GUID
     * @return {bool} 优化成功返回true，否则返回false
     * @attention 优化电源计划时，需要先删除电源计划下的所有子组
     */
    bool optimizePowerPlan(GUID *powerPlanGuid);

    /**
     * @brief 删除电源计划
     * @param {GUID*} powerPlanGuid 电源计划GUID
     * @return {bool} 删除成功返回true，否则返回false
     * @attention 删除电源计划时，需要先删除电源计划下的所有子组
     */
    bool deletePowerPlan(GUID *powerPlanGuid);

    /**
     * @brief 设置活动电源计划
     * @param {GUID*} powerPlanGuid 电源计划GUID
     * @return {bool} 设置成功返回true，否则返回false
     */
    bool setPowerPlanActive(GUID *powerPlanGuid);

private:
    // 高性能电源计划 GUID 8c5e7fda-e8bf-4a96-9a85-a6e23a8c635c
    const GUID m_GUID_HIGH_PERFORMANCE = {0x8c5e7fda, 0xe8bf, 0x4a96, {0x9a, 0x85, 0xa6, 0xe2, 0x3a, 0x8c, 0x63, 0x5c}};

    // 平衡电源计划 GUID 381b4222-f694-41f0-9685-ff5bb260df2e
    const GUID m_GUID_BALANCE = {0x381b4222, 0xf694, 0x41f0, {0x96, 0x85, 0xff, 0x5b, 0xb2, 0x60, 0xdf, 0x2e}};

    // 修改电源计划设置
    // USB 设置
    // USB 子组 GUID 2a737441-1930-4402-8d77-b2bebba308a3
    const GUID m_GUID_SUBGROUP_USB_DEVICE = {0x2a737441, 0x1930, 0x4402, {0x8d, 0x77, 0xb2, 0xbe, 0xbb, 0xa3, 0x08, 0xa3}};
    // 1. 禁用USB选择性暂停 GUID 48e6b7a6-50f5-4782-a5d4-53bb8f07e226 设置值:1. AC: 0, 2. DC: 0
    const std::pair<GUID, DWORD> m_GUID_SUBGROUP_USB_DEVICE_SELECTIVE_SUSPEND = {
        {0x48e6b7a6, 0x50f5, 0x4782, {0xa5, 0xd4, 0x53, 0xbb, 0x8f, 0x07, 0xe2, 0x26}}, 0};

    // 2. USB 3 Link Power Management GUID d4e98f31-5ffe-4ce1-be31-1b38b384c009 设置值:1. AC: 0, 2. DC: 0
    const std::pair<GUID, DWORD> m_GUID_SUBGROUP_USB_DEVICE_LINK_POWER_MANAGEMENT = {
        {0xd4e98f31, 0x5ffe, 0x4ce1, {0xbe, 0x31, 0x1b, 0x38, 0xb3, 0x84, 0xc0, 0x09}},
        0};

    // PCI Express 设置
    // PCI Express 子组 GUID 501a4d13-42af-4429-9fd1-a8218c268e20
    const GUID m_GUID_SUBGROUP_PCI_EXPRESS = {0x501a4d13, 0x42af, 0x4429, {0x9f, 0xd1, 0xa8, 0x21, 0x8c, 0x26, 0x8e, 0x20}};
    // 1. PCI Express 链路状态电源管理 GUID ee12f906-d277-404b-b6da-e5fa1a576df5 设置值:1. AC: 0, 2. DC: 0
    const std::pair<GUID, DWORD> m_GUID_SUBGROUP_PCI_EXPRESS_LINK_STATE_POWER_MANAGEMENT = {
        {0xee12f906, 0xd277, 0x404b, {0xb6, 0xda, 0xe5, 0xfa, 0x1a, 0x57, 0x6d, 0xf5}}, 0};

    // 处理器电源管理
    // 处理器电源管理 子组 GUID 54533251-82be-4824-96c1-47b60b740d00
    const GUID m_GUID_SUBGROUP_PROCESSOR_POWER_MANAGEMENT = {0x54533251, 0x82be, 0x4824, {0x96, 0xc1, 0x47, 0xb6, 0x0b, 0x74, 0x0d, 0x00}};
    // 1. 处理器性能提高阈值 GUID 06cadf0e-64ed-448a-8927-ce7bf90eb35d 设置值:1. AC: 1, 2. DC: 1
    const std::pair<GUID, DWORD> m_GUID_SUBGROUP_PROCESSOR_PERFORMANCE_INCREASE_THRESHOLD = {
        {0x06cadf0e, 0x64ed, 0x448a, {0x89, 0x27, 0xce, 0x7b, 0xf9, 0x0e, 0xb3, 0x5d}},
        1};

    // 2. 针对第 1 类处理器电源效率的处理器性能提升阈值 GUID 06cadf0e-64ed-448a-8927-ce7bf90eb35e 设置值:1. AC: 1, 2. DC: 1
    const std::pair<GUID, DWORD> m_GUID_SUBGROUP_PROCESSOR_POWER_EFFICIENCY = {
        {0x06cadf0e, 0x64ed, 0x448a, {0x89, 0x27, 0xce, 0x7b, 0xf9, 0x0e, 0xb3, 0x5e}},
        1};

    // 3. 处理器性能核心放置最小核心数量 GUID 0cc5b647-c1df-4637-891a-dec35c318583 设置值:1. AC: 100, 2. DC: 100
    const std::pair<GUID, DWORD> m_GUID_SUBGROUP_PROCESSOR_CORE_PLACEMENT_MINIMUM = {
        {0x0cc5b647, 0xc1df, 0x4637, {0x89, 0x1a, 0xde, 0xc3, 0x5c, 0x31, 0x85, 0x83}},
        100};

    // 4. 针对第 1 类处理器电源效率的处理器性能核心放置最小核心数量 GUID 0cc5b647-c1df-4637-891a-dec35c318584 设置值:1. AC: 100, 2. DC: 100
    const std::pair<GUID, DWORD> m_GUID_SUBGROUP_PROCESSOR_POWER_EFFICIENCY_CORE_PLACEMENT_MINIMUM = {
        {0x0cc5b647, 0xc1df, 0x4637, {0x89, 0x1a, 0xde, 0xc3, 0x5c, 0x31, 0x85, 0x84}},
        100};

    // 5. 允许节流状态 GUID 3b04d4fd-1cc7-4f23-ab1c-d1337819c4bb 设置值:1. AC: 0, 2. DC: 0
    const std::pair<GUID, DWORD> m_GUID_SUBGROUP_PROCESSOR_ALLOW_THROTTLING = {
        {0x3b04d4fd, 0x1cc7, 0x4f23, {0xab, 0x1c, 0xd1, 0x33, 0x78, 0x19, 0xc4, 0xbb}},
        0};

    // 6. 处理器闲置降级阈值 GUID 4b92d758-5a24-4851-a470-815d78aee119 设置值:1. AC: 100, 2. DC: 100
    const std::pair<GUID, DWORD> m_GUID_SUBGROUP_PROCESSOR_IDLE_STATE_DEGRADATION_THRESHOLD = {
        {0x4b92d758, 0x5a24, 0x4851, {0xa4, 0x70, 0x81, 0x5d, 0x78, 0xae, 0xe1, 0x19}},
        100};

    // 7. 处理器性能时间检查间隔 GUID 4d2b0152-7d5c-498b-88e2-34345392a2c5 设置值:1. AC: 5000, 2. DC: 5000
    const std::pair<GUID, DWORD> m_GUID_SUBGROUP_PROCESSOR_PERFORMANCE_TIME_CHECK_INTERVAL = {
        {0x4d2b0152, 0x7d5c, 0x498b, {0x88, 0xe2, 0x34, 0x34, 0x53, 0x92, 0xa2, 0xc5}},
        5000};

    // 8. 最大处理器状态 GUID bc5038f7-23e0-4960-96da-33abaf5935ec 设置值:1. AC: 100, 2. DC: 100
    const std::pair<GUID, DWORD> m_GUID_SUBGROUP_PROCESSOR_MAXIMUM_STATE = {
        {0xbc5038f7, 0x23e0, 0x4960, {0x96, 0xda, 0x33, 0xab, 0xaf, 0x59, 0x35, 0xec}},
        100};

    // 9. 最小处理器状态 GUID 893dee8e-2bef-41e0-89c6-b55d0929964c 设置值:1. AC: 100, 2. DC: 100
    const std::pair<GUID, DWORD> m_GUID_SUBGROUP_PROCESSOR_MINIMUM_STATE = {
        {0x893dee8e, 0x2bef, 0x41e0, {0x89, 0xc6, 0xb5, 0x5d, 0x09, 0x29, 0x96, 0x4c}},
        100};

    // 10. 异类线程调度策略 GUID 93b8b6dc-0698-4d1c-9ee4-0644e900c85d 设置值:1. AC: 0, 2. DC: 0
    const std::pair<GUID, DWORD> m_GUID_SUBGROUP_PROCESSOR_HETEROGENEOUS_THREAD_SCHEDULING_POLICY = {
        {0x93b8b6dc, 0x0698, 0x4d1c, {0x9e, 0xe4, 0x06, 0x44, 0xe9, 0x00, 0xc8, 0x5d}},
        0};

    // 创建一个二维数组，数组的第一个元素是自组GUID，第二个元素是对应子组下的GUID的数组和对应的值
    std::vector<std::pair<GUID, std::vector<std::pair<GUID, DWORD>>>> m_powerPlanSubGroupArray = {
        {m_GUID_SUBGROUP_USB_DEVICE,
         {m_GUID_SUBGROUP_USB_DEVICE_SELECTIVE_SUSPEND,
          m_GUID_SUBGROUP_USB_DEVICE_LINK_POWER_MANAGEMENT}},
        {m_GUID_SUBGROUP_PCI_EXPRESS,
         {m_GUID_SUBGROUP_PCI_EXPRESS_LINK_STATE_POWER_MANAGEMENT}},
        {m_GUID_SUBGROUP_PROCESSOR_POWER_MANAGEMENT,
         {m_GUID_SUBGROUP_PROCESSOR_PERFORMANCE_INCREASE_THRESHOLD,
          m_GUID_SUBGROUP_PROCESSOR_POWER_EFFICIENCY,
          m_GUID_SUBGROUP_PROCESSOR_CORE_PLACEMENT_MINIMUM,
          m_GUID_SUBGROUP_PROCESSOR_POWER_EFFICIENCY_CORE_PLACEMENT_MINIMUM,
          m_GUID_SUBGROUP_PROCESSOR_ALLOW_THROTTLING,
          m_GUID_SUBGROUP_PROCESSOR_IDLE_STATE_DEGRADATION_THRESHOLD,
          m_GUID_SUBGROUP_PROCESSOR_PERFORMANCE_TIME_CHECK_INTERVAL,
          m_GUID_SUBGROUP_PROCESSOR_MAXIMUM_STATE,
          m_GUID_SUBGROUP_PROCESSOR_MINIMUM_STATE,
          m_GUID_SUBGROUP_PROCESSOR_HETEROGENEOUS_THREAD_SCHEDULING_POLICY}},
    };
};
