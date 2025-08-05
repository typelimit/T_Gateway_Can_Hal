#ifndef __APP_TASK_H__
#define __APP_TASH_H__
#include "FreeRTOS.h"
#include "task.h"
#include "Common_Debug.h"
#include "Inf_W5500.h"
#include "cJSON.h"
#include "Driver_Can.h"


#define MOTOR_STATUS_INDEX 0
#define TARGET_SPEED_INDEX 0
#define DIR_INDEX 0
#define CURRENT_SPEED_INDEX 0

void App_Task_Start(void);

#endif
