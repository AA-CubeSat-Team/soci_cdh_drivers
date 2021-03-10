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
#define EXAMPLE_I2C_MASTER_BASE (LPI2C1_BASE)
#define EXAMPLE_I2C_MASTER_IRQN (LPI2C1_IRQn)
#define EXAMPLE_I2C_SLAVE_BASE  (LPI2C2_BASE)
#define EXAMPLE_I2C_SLAVE_IRQN  (LPI2C2_IRQn)

/* Select USB1 PLL (480 MHz) as master lpi2c clock source */
#define LPI2C_CLOCK_SOURCE_SELECT (0U)
/* Clock divider for master lpi2c clock source */
#define LPI2C_CLOCK_SOURCE_DIVIDER (5U)
/* Get frequency of lpi2c clock */
#define LPI2C_CLOCK_FREQUENCY ((CLOCK_GetFreq(kCLOCK_Usb1PllClk) / 8) / (LPI2C_CLOCK_SOURCE_DIVIDER + 1U))


#define EXAMPLE_I2C_MASTER   ((LPI2C_Type *)EXAMPLE_I2C_MASTER_BASE)
#define EXAMPLE_I2C_SLAVE   ((LPI2C_Type *)EXAMPLE_I2C_SLAVE_BASE)

#define I2C_MASTER_SLAVE_ADDR_7BIT (0x7EU)
#define I2C_BAUDRATE (100000) /* 100K */
#define I2C_DATA_LENGTH (32) /* MAX is 256 */



#define HMC5983_ADDR7BIT (0x1E)
//#define HMC5983_WHOAMI (0x0A)

#define HMC5983_CONF_A		0x00
#define HMC5983_CONF_B		0x01
#define HMC5983_MODE		0x02
#define HMC5983_OUT_X_MSB	0x03
#define HMC5983_OUT_X_LSB	0x04
#define HMC5983_OUT_Z_MSB	0x05
#define HMC5983_OUT_Z_LSB	0x06
#define HMC5983_OUT_Y_MSB	0x07
#define HMC5983_OUT_Y_LSB	0x08
#define HMC5983_STATUS		0x09
#define HMC5983_ID_A		0x0A
#define HMC5983_ID_B		0x0B
#define HMC5983_ID_C		0x0C
#define HMC5983_TEMP_OUT_MSB	0x31
#define HMC5983_TEMP_OUT_LSB	0x32

// I2C COMMANDS
#define HMC5983_WRITE 	  	0x3C // includes the address (1E) plus read/write bit
#define HMC5983_READ   	 	0x3D


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
#define slave_task_PRIORITY  (configMAX_PRIORITIES - 2)
#define master_task_PRIORITY (configMAX_PRIORITIES - 1)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static void master_task(void *pvParameters);



/*
 * HMC5983
 *
 * 8-bit read address 0x3D
 *
 * 8-bit write address 0x3C
 * 3400 kHz
 *
 *
 *
Hysteresis of Schmitt trigger inputs on SCL
and SDA - Fall (VDDIO=1.8V)
Rise (VDDIO=1.8V)
0.2*VDDIO
0.8*VDDIO

example use: https://github.com/arduino/HMC5983/blob/master/HMC5983.h
 *
 *
 *
 *
 *
 *
All bus transactions begin with the master device issuing the start sequence followed by the slave address byte. The
address byte contains the slave address; the upper 7 bits (bits7-1), and the Least Significant bit (LSb). The LSb of the
address byte designates if the operation is a read (LSb=1) or a write (LSb=0). At the 9
th clock pulse, the receiving slave
device will issue the ACK (or NACK). Following these bus events, the master will send data bytes for a write operation, or
the slave will clock out data with a read operation. All bus transactions are terminated with the master issuing a stop
sequence.

Below is an example of a (power-on) initialization process for “single-measurement mode” via I²C interface:
1. Write CRA (00) – send 0x3C 0x00 0x70 (8-average, 15 Hz default or any other rate, normal measurement)
2. Write CRB (01) – send 0x3C 0x01 0xA0 (Gain=5, or any other desired gain)
3. For each measurement query:
Write Mode (02) – send 0x3C 0x02 0x01 (Single-measurement mode)
Wait 6 ms or monitor status register or DRDY hardware interrupt pin
Send 0x3D 0x06 (Read all 6 bytes. If gain is changed then this data set is using previous gain)
Convert three 16-bit 2’s compliment hex values to decimal values and assign to X, Z, Y, respectively.
 */



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
    NVIC_SetPriority(EXAMPLE_I2C_MASTER_IRQN, 3);
    NVIC_SetPriority(EXAMPLE_I2C_SLAVE_IRQN, 2);
    PRINTF("\r\nLPI2C example -- MasterInterrupt_SlaveInterrupt.\r\n");


    //http://c1170156.r56.cf3.rackcdn.com/UK_HMP_HMC5983_DS.pdf
//    /* Set up i2c master to send data to slave */
//        g_master_buff[0] = 0x70U; // Config Reg A: 8-average, 15Hz rate
//        g_master_buff[1] = 0xA0U; // Config Reg B: ( Default=0x20 ) Gain = 5
//        g_master_buff[2] = 0x01U; // Mode Reg: Single measurement mode
    // Output: DXRA (MSB) and DXRB (LSB) of X measurement

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


void I2C_Master_Transfer(lpi2c_rtos_handle_t handle, uint16_t slave_address, void * tx_buffer, size_t tx_size, void * rx_buffer, size_t rx_size)
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
		PRINTF("0x%2x  ", ((int *)tx_buffer)[i]);
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

	status = LPI2C_RTOS_Transfer(&handle, &masterXfer);
	if (status == kStatus_Success)
	{
		PRINTF("I2C master transfer completed successfully.\r\n");

		memset(&masterXfer, 0, sizeof(masterXfer));
		masterXfer.slaveAddress   = slave_address;
		masterXfer.direction      = kLPI2C_Read;
		masterXfer.subaddress     = 0;
		masterXfer.subaddressSize = 0;
		masterXfer.data           = rx_buffer;
		masterXfer.dataSize       = rx_size;
		masterXfer.flags          = kLPI2C_TransferDefaultFlag;

		status = LPI2C_RTOS_Transfer(&handle, &masterXfer);
		PRINTF("Received data :");
		for (i = 0; i < rx_size; i++)
		{
			if (i % 8 == 0)
			{
				PRINTF("\r\n");
			}
			PRINTF("0x%2x  ", ((int *)rx_buffer)[i]);
		}
		PRINTF("\r\n\r\n");
	}
	else
	{
		PRINTF("I2C master transfer completed with ERROR!\r\n");
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

	status = LPI2C_RTOS_Init(&master_rtos_handle, EXAMPLE_I2C_MASTER, &masterConfig, LPI2C_CLOCK_FREQUENCY);
	if (status != kStatus_Success)
	{
		PRINTF("LPI2C master: Error initializing LPI2C!\r\n");
		vTaskSuspend(NULL);
	}

    for(;;){
		TickType_t xLastWakeTime = xTaskGetTickCount();

		I2C_Master_Transfer(master_rtos_handle, I2C_MASTER_SLAVE_ADDR_7BIT, i2c1_tx_buff, I2C_DATA_LENGTH, i2c1_rx_buff, I2C_DATA_LENGTH);

		vTaskDelayUntil(&xLastWakeTime, xDelayms);
    }
}



/*
 *
  	memset(&masterXfer, 0, sizeof(masterXfer));
	masterXfer.slaveAddress   = I2C_MASTER_SLAVE_ADDR_7BIT;
	masterXfer.direction      = kLPI2C_Write;
	masterXfer.subaddress     = 0;
	masterXfer.subaddressSize = 0;
	masterXfer.data           = i2c1_tx_buff;
	masterXfer.dataSize       = I2C_DATA_LENGTH;
	masterXfer.flags          = kLPI2C_TransferDefaultFlag;

	status = LPI2C_RTOS_Transfer(&master_rtos_handle, &masterXfer);
	if (status == kStatus_Success)
	{
		PRINTF("I2C master transfer completed successfully.\r\n");

		memset(&masterXfer, 0, sizeof(masterXfer));
		masterXfer.slaveAddress   = I2C_MASTER_SLAVE_ADDR_7BIT;
		masterXfer.direction      = kLPI2C_Read;
		masterXfer.subaddress     = 0;
		masterXfer.subaddressSize = 0;
		masterXfer.data           = i2c1_rx_buff;
		masterXfer.dataSize       = I2C_DATA_LENGTH;
		masterXfer.flags          = kLPI2C_TransferDefaultFlag;

		status = LPI2C_RTOS_Transfer(&master_rtos_handle, &masterXfer);

		PRINTF("Received data :");
		for (i = 0; i < I2C_DATA_LENGTH; i++)
		{
			if (i % 8 == 0)
			{
				PRINTF("\r\n");
			}
			PRINTF("0x%2x  ", i2c1_rx_buff[i]);
		}
		PRINTF("\r\n\r\n");
	}
	else
	{
		PRINTF("I2C master transfer completed with ERROR!\r\n");
	}

 *
 *
 *
 *
 * */
