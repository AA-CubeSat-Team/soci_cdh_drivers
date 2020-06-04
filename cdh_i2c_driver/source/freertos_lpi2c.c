/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Standard C Included Files */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

/* Freescale includes. */
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_lpi2c.h"
#include "fsl_lpi2c_freertos.h"

#include "pin_mux.h"
#include "clock_config.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_I2C_MASTER_BASE (LPI2C1_BASE)
#define EXAMPLE_I2C_MASTER_IRQN (LPI2C1_IRQn)
#define EXAMPLE_I2C_SLAVE_BASE (LPI2C3_BASE)
#define EXAMPLE_I2C_SLAVE_IRQN (LPI2C3_IRQn)

/* Select USB1 PLL (480 MHz) as master lpi2c clock source */
#define LPI2C_CLOCK_SOURCE_SELECT (0U)
/* Clock divider for master lpi2c clock source */
#define LPI2C_CLOCK_SOURCE_DIVIDER (5U)
/* Get frequency of lpi2c clock */
#define LPI2C_CLOCK_FREQUENCY ((CLOCK_GetFreq(kCLOCK_Usb1PllClk) / 8) / (LPI2C_CLOCK_SOURCE_DIVIDER + 1U))

/* clang-format off */
/*
The example support single board communication(one instance works as master, another
instance works as slave) or board to board communication. Default is single board mode.
*/
#define SINGLE_BOARD 0
#define BOARD_TO_BOARD 1
#ifndef EXAMPLE_CONNECT_I2C
#define EXAMPLE_CONNECT_I2C SINGLE_BOARD
#endif /* EXAMPLE_CONNECT_I2C */
#if (EXAMPLE_CONNECT_I2C == BOARD_TO_BOARD)
#define isMASTER 0
#define isSLAVE 1
#ifndef I2C_MASTER_SLAVE
#define I2C_MASTER_SLAVE isMASTER
#endif /* I2C_MASTER_SLAVE */
#endif /* EXAMPLE_CONNECT_I2C */

#define EXAMPLE_I2C_MASTER   ((LPI2C_Type *)EXAMPLE_I2C_MASTER_BASE)
#define EXAMPLE_I2C_SLAVE   ((LPI2C_Type *)EXAMPLE_I2C_SLAVE_BASE)

#define I2C_MASTER_SLAVE_ADDR_7BIT (0x7EU)
#define I2C_BAUDRATE (100000) /* 100K */
#define I2C_DATA_LENGTH (6) /* MAX is 256 */

#define HMC5983_ADDR7BIT (0x1EU)
//#define HMC5983_WHOAMI (0x0AU)
#define HMC5983_STATUS (0x09U)
#define HMC5983_CRA (0x00U)
#define HMC5983_CRB (0x01U)
#define HMC5983_MODE (0x02U)
#define HMC5983_XMSB (0x03U)
#define HMC5983_XLSB (0x04U)
#define HMC5983_ZMSB (0x05U)
#define HMC5983_ZLSB (0x06U)
#define HMC5983_YMSB (0x07U)
#define HMC5983_YLSB (0x08U)
#define HMC5983_IRA  (0x0AU)

/* Task priorities. */
#define lpi2c_task_PRIORITY (configMAX_PRIORITIES - 1)
/* clang-format on */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
uint8_t g_slave_buff[1];
uint8_t g_master_buff[I2C_DATA_LENGTH];

#if ((I2C_MASTER_SLAVE == isSLAVE) || (EXAMPLE_CONNECT_I2C == SINGLE_BOARD))
lpi2c_slave_handle_t g_s_handle;
#endif
volatile uint32_t timeout = 100000;

SemaphoreHandle_t lpi2c_sem;
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Task priorities. */
#define slave_task_PRIORITY (configMAX_PRIORITIES - 2)
#define master_task_PRIORITY (configMAX_PRIORITIES - 1)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void slave_task(void *pvParameters);
#if ((I2C_MASTER_SLAVE == isMaster) || (EXAMPLE_CONNECT_I2C == SINGLE_BOARD))
static void master_task(void *pvParameters);
static bool I2C_read_write_lp(lpi2c_rtos_handle_t* master_rtos_handle, status_t* status, uint8_t device_addr, uint8_t direction, uint8_t reg_addr, uint8_t* value, size_t datasize);
#endif
/*******************************************************************************
 * Code
 ******************************************************************************/
int main(void)
{
    uint32_t i = 0;

    /* Init board hardware. */
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    /*Clock setting for LPI2C*/
    CLOCK_SetMux(kCLOCK_Lpi2cMux, LPI2C_CLOCK_SOURCE_SELECT);
    CLOCK_SetDiv(kCLOCK_Lpi2cDiv, LPI2C_CLOCK_SOURCE_DIVIDER);

    /* Set IRQ priority for freertos_lpi2c */
    NVIC_SetPriority(EXAMPLE_I2C_MASTER_IRQN, 3);
    NVIC_SetPriority(EXAMPLE_I2C_SLAVE_IRQN, 2);
    PRINTF("\r\nLPI2C example -- MasterInterrupt_SlaveInterrupt.\r\n");

    /* Set up i2c master to send data to slave */
    g_master_buff[0] = 0x70U; //8-average, 15Hz rate
    g_master_buff[1] = 0xA0U; // Gain = 5
    g_master_buff[2] = 0x01U; //Single measurement mode



#if ((I2C_MASTER_SLAVE == isMASTER) || (EXAMPLE_CONNECT_I2C == SINGLE_BOARD))
    if (xTaskCreate(master_task, "Master_task", configMINIMAL_STACK_SIZE + 64, NULL, master_task_PRIORITY, NULL) !=
        pdPASS)
    {
        vTaskSuspend(NULL);
        PRINTF("Failed to create master task");
    }
#endif

    vTaskStartScheduler();


    while (1)
    {
    }
}

/*!
 * @brief Data structure and callback function for slave I2C communication.
 */

typedef struct _callback_message_t
{
    status_t async_status;
    SemaphoreHandle_t sem;
} callback_message_t;

#if ((I2C_MASTER_SLAVE == isSLAVE) || (EXAMPLE_CONNECT_I2C == SINGLE_BOARD))
static void lpi2c_slave_callback(LPI2C_Type *base, lpi2c_slave_transfer_t *xfer, void *userData)
{
    callback_message_t *cb_msg = (callback_message_t *)userData;
    BaseType_t reschedule      = 0;

    switch (xfer->event)
    {
        case kLPI2C_SlaveReceiveEvent:
            xfer->data     = g_slave_buff;
            xfer->dataSize = I2C_DATA_LENGTH;
            break;
        case kLPI2C_SlaveCompletionEvent:
            cb_msg->async_status = xfer->completionStatus;
            xSemaphoreGiveFromISR(cb_msg->sem, &reschedule);
            portYIELD_FROM_ISR(reschedule);
            break;
        default:
            break;
    }
}
#endif

/*!
 * @brief Task responsible for slave I2C communication.
 */

static void slave_task(void *pvParameters)
{
    callback_message_t cb_msg;

    cb_msg.sem = xSemaphoreCreateBinary();
    lpi2c_sem  = cb_msg.sem;
    if (cb_msg.sem == NULL)
    {
        PRINTF("I2C slave: Error creating semaphore\r\n");
        vTaskSuspend(NULL);
    }

#if ((I2C_MASTER_SLAVE == isSLAVE) || (EXAMPLE_CONNECT_I2C == SINGLE_BOARD))
    /* Set up i2c slave first */
    /*
     * slaveConfig.address0 = 0U;
     * slaveConfig.address1 = 0U;
     * slaveConfig.addressMatchMode = kLPI2C_MatchAddress0;
     * slaveConfig.filterDozeEnable = true;
     * slaveConfig.filterEnable = true;
     * slaveConfig.enableGeneralCall = false;
     * slaveConfig.ignoreAck = false;
     * slaveConfig.enableReceivedAddressRead = false;
     * slaveConfig.sdaGlitchFilterWidth_ns = 0;
     * slaveConfig.sclGlitchFilterWidth_ns = 0;
     * slaveConfig.dataValidDelay_ns = 0;
     * slaveConfig.clockHoldTime_ns = 0;
     */
    lpi2c_slave_config_t slaveConfig;
    LPI2C_SlaveGetDefaultConfig(&slaveConfig);

    slaveConfig.address0 = I2C_MASTER_SLAVE_ADDR_7BIT;
    slaveConfig.address1 = 0;
    LPI2C_SlaveInit(EXAMPLE_I2C_SLAVE, &slaveConfig, LPI2C_CLOCK_FREQUENCY);

    memset(&g_s_handle, 0, sizeof(g_s_handle));
    memset(&g_slave_buff, 0, sizeof(g_slave_buff));

    LPI2C_SlaveTransferCreateHandle(EXAMPLE_I2C_SLAVE, &g_s_handle, lpi2c_slave_callback, &cb_msg);
    LPI2C_SlaveTransferNonBlocking(EXAMPLE_I2C_SLAVE, &g_s_handle,
                                   kLPI2C_SlaveReceiveEvent | kLPI2C_SlaveCompletionEvent);
#endif
#if ((I2C_MASTER_SLAVE == isMASTER) || (EXAMPLE_CONNECT_I2C == SINGLE_BOARD))
    if (xTaskCreate(master_task, "Master_task", configMINIMAL_STACK_SIZE + 64, NULL, master_task_PRIORITY, NULL) !=
        pdPASS)
    {
        vTaskSuspend(NULL);
        PRINTF("Failed to create master task");
    }
#endif

    /* Wait for transfer to finish */
    if (xSemaphoreTake(cb_msg.sem, portMAX_DELAY) != pdTRUE)
    {
        PRINTF("Failed to take semaphore.\r\n");
    }

#if ((I2C_MASTER_SLAVE == isSLAVE) || (EXAMPLE_CONNECT_I2C == SINGLE_BOARD))
    if (cb_msg.async_status == kStatus_Success)
    {
        PRINTF("I2C slave transfer completed successfully. \r\n\r\n");
    }
    else
    {
        PRINTF("I2C slave transfer completed with error. \r\n\r\n");
    }

    int i;
    /* Transfer completed. Check the data. */
    for (i = 0; i < I2C_DATA_LENGTH; i++)
    {
        if (g_slave_buff[i] != g_master_buff[i])
        {
            PRINTF("\r\nError occurred in this transfer ! \r\n");
            break;
        }
    }

    if (i == 32)
    {
        PRINTF("\r\n Transfer successfully!\r\n ");
    }

    PRINTF("\r\nSlave received data :");
    for (i = 0; i < I2C_DATA_LENGTH; i++)
    {
        if (i % 8 == 0)
        {
            PRINTF("\r\n");
        }
        PRINTF("0x%2x  ", g_slave_buff[i]);
    }
    PRINTF("\r\n\r\n");
#endif

    vTaskSuspend(NULL);
}

#if ((I2C_MASTER_SLAVE == isMaster) || (EXAMPLE_CONNECT_I2C == SINGLE_BOARD))
static void master_task(void *pvParameters)
{
    lpi2c_master_config_t masterConfig;
    lpi2c_rtos_handle_t master_rtos_handle;
    //lpi2c_master_transfer_t masterXfer;
    status_t status;
    uint32_t i = 0;

    PRINTF("Master will send data :");
    for (i = 0; i < I2C_DATA_LENGTH; i++)
    {
        if (i % 8 == 0)
        {
            PRINTF("\r\n");
        }
        PRINTF("0x%2x  ", g_master_buff[i]);
    }
    PRINTF("\r\n\r\n");


     masterConfig.debugEnable = true;
     masterConfig.ignoreAck = false;
     masterConfig.pinConfig = kLPI2C_2PinOpenDrain;
     masterConfig.baudRate_Hz = 100000U;
     masterConfig.busIdleTimeout_ns = 0;
     masterConfig.pinLowTimeout_ns = 0;
     masterConfig.sdaGlitchFilterWidth_ns = 0;
     masterConfig.sclGlitchFilterWidth_ns = 0;

    LPI2C_MasterGetDefaultConfig(&masterConfig);
    masterConfig.baudRate_Hz = I2C_BAUDRATE;
    status = LPI2C_RTOS_Init(&master_rtos_handle, EXAMPLE_I2C_MASTER, &masterConfig, LPI2C_CLOCK_FREQUENCY);
    if (status != kStatus_Success)
    {
        PRINTF("LPI2C master: Error initializing LPI2C!\r\n");
        vTaskSuspend(NULL);
    }

    I2C_read_write_lp(&master_rtos_handle, &status, HMC5983_ADDR7BIT, kLPI2C_Write, HMC5983_CRA, g_master_buff[0], 1U);
    I2C_read_write_lp(&master_rtos_handle, &status, HMC5983_ADDR7BIT, kLPI2C_Write, HMC5983_CRB, g_master_buff[1], 1U);
    I2C_read_write_lp(&master_rtos_handle, &status, HMC5983_ADDR7BIT, kLPI2C_Write, HMC5983_MODE, g_master_buff[2], 1U);
    for (int i = 0; i < 5000; i++)
    {
    	// delay 6ms ish
    }
    I2C_read_write_lp(&master_rtos_handle, &status, HMC5983_ADDR7BIT, kLPI2C_Read, HMC5983_IRA, g_slave_buff, 1);
    for (i = 0; i < 1; i++)
    {
        if (i % 8 == 0)
        {
            PRINTF("\r\n");
        }
        PRINTF("0x%2x  ", g_slave_buff[i]);
    }
    PRINTF("\r\n\r\n");

    vTaskSuspend(NULL);
}

static bool I2C_read_write_lp(lpi2c_rtos_handle_t* master_rtos_handle, status_t* status, uint8_t device_addr, uint8_t direction, uint8_t reg_addr, uint8_t* value, size_t datasize)
{
	lpi2c_master_transfer_t masterXfer;

    memset(&masterXfer, 0, sizeof(masterXfer));
    masterXfer.slaveAddress   = device_addr;
    masterXfer.direction      = direction;
    masterXfer.subaddress     = reg_addr;
    masterXfer.subaddressSize = 1U;
    masterXfer.data           = value;
    masterXfer.dataSize       = datasize;
    masterXfer.flags          = kLPI2C_TransferDefaultFlag;

    status = LPI2C_RTOS_Transfer(master_rtos_handle, &masterXfer);
    if (status == kStatus_Success)
    {
#if (EXAMPLE_CONNECT_I2C == BOARD_TO_BOARD)
        xSemaphoreGive(lpi2c_sem);
#endif
        PRINTF("I2C master transfer completed successfully.\r\n");
        return true;
    }
    else
    {
        PRINTF("I2C master transfer completed with error!\r\n");
        return false;
    }
}

#endif
