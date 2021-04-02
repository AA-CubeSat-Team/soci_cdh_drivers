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


/* Select USB1 PLL (480 MHz) as master lpi2c clock source */
#define LPI2C_CLOCK_SOURCE_SELECT (0U)
/* Clock divider for master lpi2c clock source */
#define LPI2C_CLOCK_SOURCE_DIVIDER (5U)
/* Get frequency of lpi2c clock */
#define LPI2C_CLOCK_FREQUENCY ((CLOCK_GetFreq(kCLOCK_Usb1PllClk) / 8) / (LPI2C_CLOCK_SOURCE_DIVIDER + 1U))

#define I2C_MASTER_SLAVE_ADDR_7BIT (0x7EU)
#define I2C_BAUDRATE (100000) /* 100K */
#define I2C_DATA_LENGTH (32) /* MAX is 256 */

/* Task priorities. */
#define lpi2c_task_PRIORITY (configMAX_PRIORITIES - 1)

/* clang-format on */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
uint8_t g_slave_buff[I2C_DATA_LENGTH];
uint8_t i2c1_tx_buff[I2C_DATA_LENGTH];
uint8_t i2c1_rx_buff[I2C_DATA_LENGTH];

volatile uint32_t timeout = 100000;

SemaphoreHandle_t lpi2c_sem;
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Task priorities. */
//#define slave_task_PRIORITY  (configMAX_PRIORITIES - 2)
#define master_task_PRIORITY (configMAX_PRIORITIES - 1)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static void master_task(void *pvParameters);

/*******************************************************************************
 * Code
 ******************************************************************************/
int main(void)
{
    uint32_t i = 0;

    /* Init board hardware. */
    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /*Clock setting for LPI2C*/
    CLOCK_SetMux(kCLOCK_Lpi2cMux, LPI2C_CLOCK_SOURCE_SELECT);
    CLOCK_SetDiv(kCLOCK_Lpi2cDiv, LPI2C_CLOCK_SOURCE_DIVIDER);

    /* Set IRQ priority for freertos_lpi2c */
    NVIC_SetPriority(LPI2C1_IRQn, 3);
    //NVIC_SetPriority(EXAMPLE_I2C_SLAVE_IRQN, 2);
    PRINTF("\r\nLPI2C example -- MasterInterrupt_SlaveInterrupt.\r\n");

    /* Set up i2c master to send data to slave */
    for (i = 0; i < I2C_DATA_LENGTH; i++)
    {
    	i2c1_tx_buff[i] = i;
    }

    if (xTaskCreate(master_task, "Master_task", configMINIMAL_STACK_SIZE + 100, NULL, master_task_PRIORITY, NULL) !=
            pdPASS)
        {
            vTaskSuspend(NULL);
            PRINTF("Failed to create master task");
        }

    vTaskStartScheduler();

    while (1)
    {
    }
}


void I2C_send(lpi2c_rtos_handle_t * handle, uint16_t slave_address, uint8_t * tx_buffer, size_t tx_size)
{
	lpi2c_master_transfer_t masterXfer;
	status_t status;

	PRINTF("Master will send data :");
	int i=0;
	for (i = 0; i < tx_size; i++)
	{
		if (i % 8 == 0)
		{
			PRINTF("\r\n");
		}
		PRINTF("0x%2x  ", tx_buffer[i]);
	}
	PRINTF("\r\n\r\n");


	memset(&masterXfer, 0, sizeof(masterXfer));
	masterXfer.slaveAddress   = slave_address;
	masterXfer.direction      = kLPI2C_Write;
	masterXfer.subaddress     = 0;
	masterXfer.subaddressSize = 0;
	masterXfer.data           = tx_buffer;
	masterXfer.dataSize       = tx_size;
	masterXfer.flags          = kLPI2C_TransferDefaultFlag;

	status = LPI2C_RTOS_Transfer(handle, &masterXfer);
	if (status == kStatus_Success)
	{
		PRINTF("I2C master transfer completed successfully.\r\n");

	}
	else
	{
		PRINTF("I2C master transfer completed with ERROR!\r\n");
	}

}

void I2C_request(lpi2c_rtos_handle_t * handle, uint16_t slave_address, uint8_t * tx_buffer, size_t tx_size, uint8_t * rx_buffer, size_t rx_size)
{
	lpi2c_master_transfer_t masterXfer;
	status_t status;

	PRINTF("Master will request data\r\n");

	memset(&masterXfer, 0, sizeof(masterXfer));
	masterXfer.slaveAddress   = slave_address;
	masterXfer.direction      = kLPI2C_Read;
	masterXfer.subaddress     = 0;
	masterXfer.subaddressSize = 0;
	masterXfer.data           = rx_buffer;
	masterXfer.dataSize       = rx_size;
	masterXfer.flags          = kLPI2C_TransferDefaultFlag;

	status = LPI2C_RTOS_Transfer(handle, &masterXfer);
	if (status == kStatus_Success)
	{
		PRINTF("Received data :\r\n");
		int i;
		for (i = 0; i < rx_size; i++)
		{
			if (i % 8 == 0)
			{
				PRINTF("\r\n");
			}
			PRINTF("0x%2x  ", (rx_buffer)[i]);
		}
		PRINTF("\r\n\r\n");
	}
	else {
		PRINTF("Failed receive!\r\n");
	}
}

static void master_task(void *pvParameters)
{
    lpi2c_master_config_t masterConfig;
    lpi2c_rtos_handle_t master_rtos_handle;

    status_t status;

	const TickType_t xDelayms = pdMS_TO_TICKS( 2000 );

	/*
	 * masterConfig.debugEnable = false;
	 * masterConfig.ignoreAck = false;
	 * masterConfig.pinConfig = kLPI2C_2PinOpenDrain;
	 * masterConfig.baudRate_Hz = 100000U;
	 * masterConfig.busIdleTimeout_ns = 0;
	 * masterConfig.pinLowTimeout_ns = 0;
	 * masterConfig.sdaGlitchFilterWidth_ns = 0;
	 * masterConfig.sclGlitchFilterWidth_ns = 0;
	 */
	LPI2C_MasterGetDefaultConfig(&masterConfig);
	masterConfig.baudRate_Hz = I2C_BAUDRATE;

	status = LPI2C_RTOS_Init(&master_rtos_handle, (LPI2C_Type *)LPI2C1_BASE, &masterConfig, LPI2C_CLOCK_FREQUENCY);
	if (status != kStatus_Success)
	{
		PRINTF("LPI2C master: Error initializing LPI2C!\r\n");
		vTaskSuspend(NULL);
	}

	//LPI2C_MasterEnableInterrupts((LPI2C_Type *)LPI2C1_BASE, kLPI2C_MasterTxReadyFlag);
    for(;;){
		TickType_t xLastWakeTime = xTaskGetTickCount();

		I2C_send(&master_rtos_handle, 0x7E, i2c1_tx_buff, I2C_DATA_LENGTH);
		I2C_request(&master_rtos_handle, 0x7E, i2c1_tx_buff, I2C_DATA_LENGTH, i2c1_rx_buff, I2C_DATA_LENGTH);

		vTaskDelayUntil(&xLastWakeTime, xDelayms);
    }
}
