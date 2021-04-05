/*
 * gyroscope_task.h
 *
 *  Created on: Mar 3, 2020
 *      Author: Hirotoshi Maeda
 */

#ifndef GYROSCOPE_TASK_H_
#define GYROSCOPE_TASK_H_

#include "peripherals.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "fsl_lpspi.h"
#include "fsl_lpspi_freertos.h"
#include "assert.h"

#define PRINT_GYRO_STATUS 1

/*
 * Task for gyroscope
 */
void gyroscope_task(void* pvParameters);

#endif /* GYROSCOPE_TASK_H_ */
