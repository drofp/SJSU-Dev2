#pragma once

#include <FreeRTOS.h>
#include <task.h>
#include "L0_LowLevel/LPC40xx.h"

#include <cinttypes>
#include <iterator>

#include "L0_LowLevel/interrupt.hpp"
#include "L0_LowLevel/ram.hpp"
#include "L0_LowLevel/system_controller.hpp"
#include "L3_Application/commandline.hpp"
#include "utility/log.hpp"

// Print RTOS runtime stats of FreeRTOS.
class RtosCommand : public Command
{
 public:
  static constexpr const char kDescription[] =
      "Display FreeRTOS runtime stats.";
  static constexpr const char kDivider[] =
      "+------------------+-----------+------+------------+-------------+";
  static constexpr const char kHeader[] =
      "|    Task Name     |   State   | CPU% | Stack Left |   Priority  |\n"
      "|                  |           |      |  in words  | Base : Curr |";

  constexpr RtosCommand() : Command("rtos", kDescription) {}
  static const char * RtosStateToString(eTaskState state)
  {
    switch (state)
    {
      case eTaskState::eBlocked: return "BLOCKED";
      case eTaskState::eDeleted: return "DELETED";
      case eTaskState::eInvalid: return "INVALID";
      case eTaskState::eReady: return "READY";
      case eTaskState::eRunning: return "RUNNING";
      case eTaskState::eSuspended: return "SUSPENDED";
      default: return "UNKNOWN";
    }
  }

  int Program(int, const char * const[]) override final
  {
    TaskStatus_t task_status[32];
    uint32_t ulTotalRunTime;
    // Generate raw status information about each task.
    UBaseType_t number_of_tasks = uxTaskGetSystemState(
        task_status, std::size(task_status), &ulTotalRunTime);
    // For percentage calculations.
    ulTotalRunTime /= 100UL;
    // Avoid divide by zero errors.
    if (ulTotalRunTime > 0)
    {
      puts(kDivider);
      puts(kHeader);
      // For each populated position in the task_status array,
      // format the raw data as human readable ASCII data.
      for (size_t x = 0; x < number_of_tasks; x++)
      {
        // What percentage of the total run time has the task used?
        // This will always be rounded down to the nearest integer.
        // ulTotalRunTimeDiv / 100 has already been divided by 100.
        uint32_t ulStatsAsPercentage =
            task_status[x].ulRunTimeCounter / ulTotalRunTime;
        puts(kDivider);
        printf("| %16.16s | %9.9s | %3" PRIu32 "%% | %10" PRIu16
               " | %4lu : %-4lu |\n",
               task_status[x].pcTaskName,
               RtosStateToString(task_status[x].eCurrentState),
               ulStatsAsPercentage, task_status[x].usStackHighWaterMark,
               task_status[x].uxBasePriority, task_status[x].uxCurrentPriority);
      }
      puts(kDivider);
    }
    return 0;
  }
};
