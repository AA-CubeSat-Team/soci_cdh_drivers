/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


/*
 *
 * Testing Setup with Arduino Uno 5V (Uses SPI3 with chip select CS0)
 * (Arduino program located in the driver repo)
 *
 * RT1020                      Arduino
 * J18-3 <--Level Converter--> 13 (SCK)
 * J18-4 <--Level Converter--> 10 (CS0)
 * J18-5 <--Level Converter--> 12 (MISO)
 * J18-6 <--Level Converter--> 11 (MOSI)
 * GNDs connected from both boards onto level converter GND ports
 * 5V and 3.3V supplied to level converter (HV and LV) from
 * respective boards or from a single boards 5V and 3.3V pins
 *
 *
 */


/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

/* Freescale includes. */
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "fsl_lpspi.h"
#include "fsl_lpspi_freertos.h"
#include "pin_mux.h"
#include "board.h"

#include "fsl_common.h"
#if ((defined FSL_FEATURE_SOC_INTMUX_COUNT) && (FSL_FEATURE_SOC_INTMUX_COUNT))
#include "fsl_intmux.h"
#endif
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Master related */
#define EXAMPLE_LPSPI_MASTER_BASEADDR (LPSPI3)
#define EXAMPLE_LPSPI_MASTER_IRQN     LPSPI3_IRQn

#define EXAMPLE_LPSPI_MASTER_PCS_FOR_INIT     (kLPSPI_Pcs0)
#define EXAMPLE_LPSPI_MASTER_PCS_FOR_TRANSFER (kLPSPI_MasterPcs0)

/* Select USB1 PLL PFD0 (720 MHz) as lpspi clock source */
#define EXAMPLE_LPSPI_CLOCK_SOURCE_SELECT (1U)
/* Clock divider for master lpspi clock source */
#define EXAMPLE_LPSPI_CLOCK_SOURCE_DIVIDER (7U)

#define LPSPI_MASTER_CLK_FREQ (CLOCK_GetFreq(kCLOCK_Usb1PllPfd0Clk) / (EXAMPLE_LPSPI_CLOCK_SOURCE_DIVIDER + 1U))
#define TRANSFER_SIZE     (7U)//(64U)    /*! Transfer dataSize.*/
#define TRANSFER_BAUDRATE (500000U) /*! Transfer baudrate - 500k */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/* LPSPI user callback */
extern uint32_t LPSPI_GetInstance(LPSPI_Type *base);

/*******************************************************************************
 * Variables
 ******************************************************************************/
lpspi_slave_handle_t g_s_handle;

uint8_t masterReceiveBuffer[TRANSFER_SIZE] = {0};
uint8_t masterSendBuffer[TRANSFER_SIZE]    = {0};
uint8_t slaveSendBuffer[TRANSFER_SIZE]     = {0};

SemaphoreHandle_t lpspi_sem;
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Task priorities. */
#define master_task_PRIORITY (configMAX_PRIORITIES - 1)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void master_task(void *pvParameters);

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Application entry point.
 */
int main(void)
{
    int i;

    /* Init board hardware. */
    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /*Set clock source for LPSPI*/
    CLOCK_SetMux(kCLOCK_LpspiMux, EXAMPLE_LPSPI_CLOCK_SOURCE_SELECT);
    CLOCK_SetDiv(kCLOCK_LpspiDiv, EXAMPLE_LPSPI_CLOCK_SOURCE_DIVIDER);

    NVIC_SetPriority(EXAMPLE_LPSPI_MASTER_IRQN, 3);

    PRINTF("FreeRTOS LPSPI master example starts.\r\n");

    PRINTF("This example uses two boards to connect with one as master and anohter as slave.\r\n");

    PRINTF("Master and slave are both use interrupt way.\r\n");
    PRINTF("Please make sure you make the correct line connection. Basically, the connection is:\r\n");
    PRINTF("LPSPI_master -- LPSPI_slave\r\n");
    PRINTF("    CLK      --    CLK\r\n");
    PRINTF("    PCS      --    PCS\r\n");
    PRINTF("    SOUT     --    SIN\r\n");
    PRINTF("    SIN      --    SOUT\r\n");
    PRINTF("\r\n");

    /* Initialize data in transfer buffers */
    for (i = 0; i < TRANSFER_SIZE; i++)
    {
        masterSendBuffer[i]    = i % 256;
        //masterReceiveBuffer[i] = 0;

        slaveSendBuffer[i] = ~masterSendBuffer[i];//checks match with slave response
    }

    if (xTaskCreate(master_task, "Master_task", configMINIMAL_STACK_SIZE + 64, NULL, master_task_PRIORITY, NULL) !=
        pdPASS)
    {
        PRINTF("Task creation failed!.\r\n");
        while (1)
            ;
    }
    vTaskStartScheduler();
    for (;;)
        ;
}


static void SPI_transfer(lpspi_rtos_handle_t * handler, uint8_t * txBuffer, uint8_t * rxBuffer, size_t transferSize)
{
	lpspi_transfer_t masterXfer;
	status_t status;

	//Start master transfer
	masterXfer.txData      = txBuffer;
	masterXfer.rxData      = rxBuffer;
	masterXfer.dataSize    = transferSize;
	masterXfer.configFlags = EXAMPLE_LPSPI_MASTER_PCS_FOR_TRANSFER | kLPSPI_MasterPcsContinuous | kLPSPI_SlaveByteSwap;

	status = LPSPI_RTOS_Transfer(handler, &masterXfer);
	if (status == kStatus_Success)
	{
		PRINTF("LPSPI master transfer completed successfully.\r\n");
	}
	else
	{
		PRINTF("LPSPI master transfer completed with error.\r\n");
	}
}

static void SPI_transfer2(lpspi_rtos_handle_t * handler, uint8_t * txBuffer, uint8_t * rxBuffer, size_t transferSize)
{
	lpspi_transfer_t masterXfer;
	status_t status;
	transferSize++;
	uint8_t newtxBuffer[transferSize];
	uint8_t newrxBuffer[transferSize];
	memcpy(newtxBuffer, txBuffer, transferSize - 1);
	newtxBuffer[transferSize - 1] = 0;

	//Start master transfer
	masterXfer.txData      = newtxBuffer;
	masterXfer.rxData      = newrxBuffer;
	masterXfer.dataSize    = transferSize;
	masterXfer.configFlags = EXAMPLE_LPSPI_MASTER_PCS_FOR_TRANSFER | kLPSPI_MasterPcsContinuous | kLPSPI_SlaveByteSwap;

	status = LPSPI_RTOS_Transfer(handler, &masterXfer);
	if (status == kStatus_Success)
	{
		PRINTF("LPSPI master transfer completed successfully.\r\n");
		memcpy(rxBuffer, &newrxBuffer[1], transferSize - 1);
	}
	else
	{
		PRINTF("LPSPI master transfer completed with error.\r\n");
	}
}

void SPI_send(lpspi_rtos_handle_t * handler, uint8_t * txBuffer, size_t transferSize)
{
	lpspi_transfer_t masterXfer;
	status_t status;
	transferSize++;

	/*Start master transfer*/
	uint8_t rxBuffer[transferSize];
	uint8_t newtxBuffer[transferSize];
	memcpy(&newtxBuffer[1], txBuffer, transferSize - 1);
	newtxBuffer[0] = 0;
	masterXfer.txData      = newtxBuffer;
	masterXfer.rxData      = rxBuffer;
	masterXfer.dataSize    = transferSize;
	masterXfer.configFlags = EXAMPLE_LPSPI_MASTER_PCS_FOR_TRANSFER | kLPSPI_MasterPcsContinuous | kLPSPI_SlaveByteSwap;

	status = LPSPI_RTOS_Transfer(handler, &masterXfer);
	//sanity check
	PRINTF("SPI SEND");
	PRINTF("Send Buffer: ");
	for(int i = 0; i < sizeof(newtxBuffer); i++) {
		PRINTF("%d ", newtxBuffer[i]);
	}
	PRINTF("\n");
	PRINTF("Receive Buffer: ");
	for(int i = 0; i < sizeof(rxBuffer); i++) {
		PRINTF("%d ", rxBuffer[i]);
	}
	PRINTF("\n");
	memset(rxBuffer, 0, sizeof(rxBuffer));
	if (status == kStatus_Success)
	{
		PRINTF("LPSPI master send completed successfully.\r\n");
	}
	else
	{
		PRINTF("LPSPI master send completed with error.\r\n");
	}
}

void SPI_request(lpspi_rtos_handle_t * handler, uint8_t * rxBuffer, size_t transferSize)
{
	lpspi_transfer_t masterXfer;
	status_t status;
	transferSize++;

	uint8_t txBuffer[transferSize];
	uint8_t newrxBuffer[transferSize];
	memset(txBuffer, 0xFF, transferSize);
	/*Start master transfer*/
	masterXfer.txData      = txBuffer;
	masterXfer.rxData      = newrxBuffer;
	masterXfer.dataSize    = transferSize;
	masterXfer.configFlags = EXAMPLE_LPSPI_MASTER_PCS_FOR_TRANSFER | kLPSPI_MasterPcsContinuous | kLPSPI_SlaveByteSwap;

	status = LPSPI_RTOS_Transfer(handler, &masterXfer);

	//sanity check
	PRINTF("SPI SEND");
	PRINTF("Send Buffer: ");
	for(int i = 0; i < sizeof(txBuffer); i++) {
		PRINTF("%d ", txBuffer[i]);
	}
	PRINTF("\n");
	PRINTF("Receive Buffer: ");
	for(int i = 0; i < sizeof(newrxBuffer); i++) {
		PRINTF("%d ", newrxBuffer[i]);
	}
	PRINTF("\n");

	if (status == kStatus_Success)
	{
		PRINTF("LPSPI master request completed successfully.\r\n");
		memcpy(rxBuffer, &newrxBuffer[1], transferSize - 1);
	}
	else
	{
		PRINTF("LPSPI master request completed with error.\r\n");
	}
}

/*!
 * @brief Task responsible for master SPI communication.
 */
static void master_task(void *pvParameters)
{
    lpspi_transfer_t masterXfer;
    lpspi_rtos_handle_t master_rtos_handle;
    lpspi_master_config_t masterConfig;
    status_t status;

    LPSPI_MasterGetDefaultConfig(&masterConfig);
    masterConfig.baudRate = TRANSFER_BAUDRATE;
    masterConfig.whichPcs = EXAMPLE_LPSPI_MASTER_PCS_FOR_INIT;

    status = LPSPI_RTOS_Init(&master_rtos_handle, EXAMPLE_LPSPI_MASTER_BASEADDR, &masterConfig, LPSPI_MASTER_CLK_FREQ);
    if (status != kStatus_Success)
    {
        PRINTF("LPSPI master: error during initialization. \r\n");
        vTaskSuspend(NULL);
    }


    // SPI_transfer(&master_rtos_handle, masterSendBuffer, masterReceiveBuffer, TRANSFER_SIZE);
    SPI_send(&master_rtos_handle, masterSendBuffer, TRANSFER_SIZE);
    SPI_request(&master_rtos_handle, masterReceiveBuffer, TRANSFER_SIZE);

    uint32_t errorCount;
    uint32_t i;

    PRINTF("EXPECTED: \n");
    for (i = 0; i < TRANSFER_SIZE; i++)
    	{
    		/* Print 16 numbers in a line */
    		if ((i % 0x08U) == 0U)
    		{
    			PRINTF("\r\n");
    		}
    		PRINTF(" %02X", slaveSendBuffer[i]);
    	}
    	PRINTF("\r\n");

        PRINTF("RECEIVED: \n");

    for (i = 0; i < TRANSFER_SIZE; i++)
	{
		/* Print 16 numbers in a line */
		if ((i % 0x08U) == 0U)
		{
			PRINTF("\r\n");
		}
		PRINTF(" %02X", masterReceiveBuffer[i]);
	}
	PRINTF("\r\n");

    errorCount = 0;
    for (i = 0; i < TRANSFER_SIZE; i++)
    {
        if (slaveSendBuffer[i] != masterReceiveBuffer[i])
        {
            errorCount++;
        }
    }

    if (errorCount == 0)
    {
        PRINTF("LPSPI transfer all data matched !\r\n");
    }
    else
    {
        PRINTF("Error occurred in LPSPI transfer !\r\n");
    }

    vTaskSuspend(NULL);
}
