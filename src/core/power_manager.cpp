/*
 * @Author: vdavidyang vdavidyang@gmail.com
 * @Date: 2025-04-30 21:46:06
 * @LastEditors: vdavidyang vdavidyang@gmail.com
 * @LastEditTime: 2025-05-02 00:43:37
 * @FilePath: \GameOptimizerPro\src\core\power_manager.cpp
 * @Description:
 * Copyright (c) 2025 by vdavidyang vdavidyang@gmail.com, All Rights Reserved.
 */
#include "core/power_manager.h"

PowerManager::PowerManager()
{
}

PowerManager::~PowerManager()
{
}

bool PowerManager::createPowerPlan(GUID *newPowerPlanGuid, const TCHAR *planName, const TCHAR *planDescription)
{
  // 复制Windows高性能电源计划作为基础
  if (PowerDuplicateScheme(NULL, &m_GUID_HIGH_PERFORMANCE, &newPowerPlanGuid) != ERROR_SUCCESS)
  {
    LOG_ERROR("Failed to duplicate power scheme.");
    return false;
  }

  // 设置电源计划名称和描述
  if (PowerWriteFriendlyName(NULL, newPowerPlanGuid, NULL, NULL, (BYTE *)planName, (DWORD)(_tcslen(planName) + 1) * sizeof(TCHAR)) != ERROR_SUCCESS)
  {
    LOG_ERROR("Failed to set power plan name.");
    PowerDeleteScheme(NULL, newPowerPlanGuid);
    return false;
  }

  if (PowerWriteDescription(NULL, newPowerPlanGuid, NULL, NULL, (BYTE *)planDescription, (DWORD)(_tcslen(planDescription) + 1) * sizeof(TCHAR)) != ERROR_SUCCESS)
  {
    LOG_ERROR("Failed to set power plan description.");
    PowerDeleteScheme(NULL, newPowerPlanGuid);
    return false;
  }
  return true;
}

bool PowerManager::deletePowerPlan(GUID *powerPlanGuid)
{
  // 先将活动电源计划设置为平衡，再删除
  if (PowerSetActiveScheme(NULL, &m_GUID_BALANCE) != ERROR_SUCCESS)
  {
    LOG_ERROR("Failed to set active power scheme.");
    return false;
  }
  return PowerDeleteScheme(NULL, powerPlanGuid) == ERROR_SUCCESS;
}

bool PowerManager::optimizePowerPlan(GUID *powerPlanGuid)
{
  for (auto &powerPlanSubGroup : m_powerPlanSubGroupArray)
  {
    for (auto &powerPlanSubGroupItem : powerPlanSubGroup.second)
    {
      // 设置交流电（插电）
      if (PowerWriteACValueIndex(NULL, powerPlanGuid, &powerPlanSubGroup.first, &powerPlanSubGroupItem.first, powerPlanSubGroupItem.second) != ERROR_SUCCESS)
      {
        LOG_ERROR("Failed to set AC power plan value: SubGroup=" + GuidToString(&powerPlanSubGroup.first) + " Setting=" + GuidToString(&powerPlanSubGroupItem.first) + "!");
        return false;
      }

      // 设置直流电（电池）
      if (PowerWriteDCValueIndex(NULL, powerPlanGuid, &powerPlanSubGroup.first, &powerPlanSubGroupItem.first, powerPlanSubGroupItem.second) != ERROR_SUCCESS)
      {
        LOG_ERROR("Failed to set DC power plan value: SubGroup=" + GuidToString(&powerPlanSubGroup.first) + " Setting=" + GuidToString(&powerPlanSubGroupItem.first) + "!");
        return false;
      }
    }
  }
  return true;
}

bool PowerManager::setPowerPlanActive(GUID *powerPlanGuid)
{
  // 设置活动电源计划
  DWORD result = PowerSetActiveScheme(NULL, powerPlanGuid);
  if (result != ERROR_SUCCESS)
  {
    LOG_ERROR("Failed to set active power scheme.");
    return false;
  }
  return true;
}