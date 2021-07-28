/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2019 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "fsl_common.h"
#include "power_mode_switch.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "lpm.h"
#include "fsl_gpt.h"
#include "fsl_lpuart.h"
#include "specific.h"
#include "peripherals.h"

#include "pin_mux.h"
#include "clock_config.h"

#include "semc_sdram.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define CPU_NAME "iMXRT1021"


/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static lpm_power_mode_t s_targetPowerMode;
static lpm_power_mode_t s_curRunMode = LPM_PowerModeOverRun;

static const char *s_modeNames[] = {"Over RUN",    "Full Run",       "Low Speed Run", "Low Power Run",
                                    "System Idle", "Low Power Idle", "Suspend"};

/*******************************************************************************
 * Code
 ******************************************************************************/


lpm_power_mode_t APP_GetRunMode(void)
{
    return s_curRunMode;
}

void APP_SetRunMode(lpm_power_mode_t powerMode)
{
    s_curRunMode = powerMode;
}

/*
 * Check whether could switch to target power mode from current mode.
 * Return true if could switch, return false if could not switch.
 */
bool APP_CheckPowerMode(lpm_power_mode_t originPowerMode, lpm_power_mode_t targetPowerMode)
{
    bool modeValid = true;

    /* If current mode is Lowpower run mode, the target mode should not be system idle mode. */
    if ((originPowerMode == LPM_PowerModeLowPowerRun) && (targetPowerMode == LPM_PowerModeSysIdle))
    {
        PRINTF("Low Power Run mode can't enter System Idle mode.\r\n");
        modeValid = false;
    }

    /* Don't need to change power mode if current mode is already the target mode. */
    if (originPowerMode == targetPowerMode)
    {
        PRINTF("Already in the target power mode.\r\n");
        modeValid = false;
    }

    return modeValid;
}

lpm_power_mode_t APP_GetLPMPowerMode(void)
{
    return s_targetPowerMode;
}

static void APP_ShowPowerMode(lpm_power_mode_t powerMode)
{
    if (powerMode <= LPM_PowerModeEnd)
    {
        PRINTF("    Power mode: %s\r\n", s_modeNames[powerMode]);
        APP_PrintRunFrequency(1);
    }
    else
    {
        assert(0);
    }
}

/*!
 * @brief main task.
 */
static void PowerModeSwitchTask(void *pvParameters)
{
    uint8_t ch;
    uint32_t freq;

    while (1)
    {
        freq = CLOCK_GetFreq(kCLOCK_CpuClk);

        PRINTF("\r\n########## Power Mode Switch Demo (build %s) ###########\n\r\n", __DATE__);
        PRINTF("    Core Clock = %dHz \r\n", freq);

        APP_ShowPowerMode(s_curRunMode);

        PRINTF("\r\nSelect the desired operation \n\r\n");
        PRINTF("Press  %c for enter: Over RUN       - System Over Run mode\r\n",
               (uint8_t)'A' + (uint8_t)LPM_PowerModeOverRun);
        PRINTF("Press  %c for enter: Full RUN       - System Full Run mode\r\n",
               (uint8_t)'A' + (uint8_t)LPM_PowerModeFullRun);
        PRINTF("Press  %c for enter: Low Speed RUN  - System Low Speed Run mode\r\n",
               (uint8_t)'A' + (uint8_t)LPM_PowerModeLowSpeedRun);
        PRINTF("Press  %c for enter: Low Power RUN  - System Low Power Run mode\r\n",
               (uint8_t)'A' + (uint8_t)LPM_PowerModeLowPowerRun);
        PRINTF("Press  %c for enter: System Idle    - System Wait mode\r\n",
               (uint8_t)'A' + (uint8_t)LPM_PowerModeSysIdle);
        PRINTF("Press  %c for enter: Low Power Idle - Low Power Idle mode\r\n",
               (uint8_t)'A' + (uint8_t)LPM_PowerModeLPIdle);
        PRINTF("Press  %c for enter: Suspend        - Suspend mode\r\n", (uint8_t)'A' + (uint8_t)LPM_PowerModeSuspend);
#if (HAS_WAKEUP_PIN)
        PRINTF("Press  %c for enter: SNVS           - Shutdown the system\r\n",
               (uint8_t)'A' + (uint8_t)LPM_PowerModeSNVS);
#endif
        PRINTF("\r\nWaiting for power mode select...\r\n\r\n");

        /* Wait for user response */
        ch = GETCHAR();

        if ((ch >= 'a') && (ch <= 'z'))
        {
            ch -= 'a' - 'A';
        }

        s_targetPowerMode = (lpm_power_mode_t)(ch - 'A');

        if (s_targetPowerMode <= LPM_PowerModeEnd)
        {
            /* If could not set the target power mode, loop continue. */
            if (!APP_CheckPowerMode(s_curRunMode, s_targetPowerMode))
            {
                continue;
            }

            if (!LPM_SetPowerMode(s_targetPowerMode))
            {
                PRINTF("Some task doesn't allow to enter mode %s\r\n", s_modeNames[s_targetPowerMode]);
            }
            else
            {
                if (s_targetPowerMode <= LPM_PowerModeRunEnd)
                {
                    switch (s_targetPowerMode)
                    {
                        case LPM_PowerModeOverRun:
                            LPM_OverDriveRun();
                            break;
                        case LPM_PowerModeFullRun:
                            LPM_FullSpeedRun();
                            break;
                        case LPM_PowerModeLowSpeedRun:
                            LPM_LowSpeedRun();
                            break;
                        case LPM_PowerModeLowPowerRun:
                            LPM_LowPowerRun();
                            break;
                        default:
                            break;
                    }
                    APP_SetRunMode(s_targetPowerMode);
                    continue;
                }
                LPM_SetPowerMode(s_curRunMode);
            }
        }
        PRINTF("\r\nNext loop\r\n");
    }
}

/*!
 * @brief simulating working task.
 */
static void WorkingTask(void *pvParameters)
{
    //LPM_RegisterPowerListener(APP_LpmListener, pvParameters);
	memset(sdram_writeBuffer, 0xFF, sizeof(sdram_writeBuffer));
	SEMC_SDRAM_Write(0, 10, 1);
	SEMC_SDRAM_Read(0, 10, 1);
	for (int i = 0; i < 10; i++) {
		PRINTF("%2x ", sdram_readBuffer[i]);
	}
    for (;;)
    {
        /* Use App task logic to replace vTaskDelay */
        PRINTF("Task %d is working now\r\n", (uint32_t)pvParameters);
        vTaskDelay(portMAX_DELAY);
    }
}

/*!
 * @brief main demo function.
 */
int main(void)
{
    BaseType_t xReturn;

    /* Init board hardware. */
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_InitBootClocks();


    BOARD_InitDebugConsole();
    BOARD_InitBootPeripherals();

    PRINTF("\r\nCPU wakeup source 0x%x...\r\n", SRC->SRSR);

    PRINTF("\r\n***********************************************************\r\n");
    PRINTF("\tPower Mode Switch Demo for %s\r\n", CPU_NAME);
    PRINTF("***********************************************************\r\n");
    APP_PrintRunFrequency(0);

    if (true != LPM_Init(s_curRunMode))
    {
        PRINTF("LPM Init Failed!\r\n");
        return -1;
    }

    if (BOARD_InitSEMC() != kStatus_Success)
	{
		PRINTF("\r\n SEMC SDRAM Init Failed\r\n");
	}

    xReturn = xTaskCreate(PowerModeSwitchTask, "Power Mode Switch Task", configMINIMAL_STACK_SIZE + 512, NULL,
                          tskIDLE_PRIORITY + 1U, NULL);
    configASSERT(xReturn);

    xReturn =
        xTaskCreate(WorkingTask, "Working Task1", configMINIMAL_STACK_SIZE, (void *)1, tskIDLE_PRIORITY + 2U, NULL);
    configASSERT(xReturn);

    xReturn =
        xTaskCreate(WorkingTask, "Working Task2", configMINIMAL_STACK_SIZE, (void *)2, tskIDLE_PRIORITY + 3U, NULL);
    configASSERT(xReturn);

    vTaskStartScheduler();

    /* Application should never reach this point. */
    for (;;)
    {
    }
}
