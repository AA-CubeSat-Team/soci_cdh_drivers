/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */




/*
 *
 *
 * Works on RT1020 J17 1&2 pins as RX and TX
 *
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
#include "board.h"

#include "fsl_lpuart_freertos.h"
#include "fsl_lpuart.h"

#include "pin_mux.h"
#include "clock_config.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* Task priorities. */
#define uart_task_PRIORITY (configMAX_PRIORITIES - 1)
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void uart_task(void *pvParameters);

/*******************************************************************************
 * Code
 ******************************************************************************/
const char *to_send               = "FreeRTOSFreeRTOS";
const char *send_ring_overrun     = "\r\nRing buffer overrun!FreeRTOS\r\n";
const char *send_hardware_overrun = "\r\nHardware buffer overrun!FreeRTOS\r\n";
uint8_t background_buffer[500];
uint8_t recv_buffer[100];

lpuart_rtos_handle_t handle;
struct _lpuart_handle t_handle;

lpuart_rtos_config_t lpuart_config = {
    .baudrate    = 74880,
    .parity      = kLPUART_ParityDisabled,
    .stopbits    = kLPUART_OneStopBit,
    .buffer      = background_buffer,
    .buffer_size = sizeof(background_buffer),
	.base        = LPUART2,
};

/*!
 * @brief Application entry point.
 */
int main(void)
{
    /* Init board hardware. */
    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();// equivalent to BOARD_BootClockRUN()

    NVIC_SetPriority(LPUART2_IRQn, 5);
    if (xTaskCreate(uart_task, "Uart_task", configMINIMAL_STACK_SIZE + 100, NULL, uart_task_PRIORITY, NULL) != pdPASS)
    {
        PRINTF("Task creation failed!.\r\n");
        while (1)
            ;
    }
    vTaskStartScheduler();
    for (;;)
        ;
}


static int uart_send(lpuart_rtos_handle_t *handle, uint8_t *buffer, uint32_t length){
	for (int i = 0; i < length; i++){
		if (kStatus_Success != LPUART_RTOS_Send(handle, &buffer[i], 1))
		{
			PRINTF("failed to send the %dth byte, terminating \r\n", i);
			return kStatus_Fail;
		} else {
			PRINTF("successfully sent %c \r\n", buffer[i]);
		}
	}
	return kStatus_Success;
}

static int uart_request(lpuart_rtos_handle_t *handle, uint8_t *buffer, uint32_t length, size_t *n){
	for (int i = 0; i < length; i++){
		size_t small_n = 0;
		int error = LPUART_RTOS_Receive(handle, &buffer[i], 1, &small_n);
		if (error != kStatus_Success)
		{
			PRINTF("failed to receive the %dth byte, terminating \r\n", i);
			return error;
		} else {
			PRINTF("successfully received %c at %d byte\r\n", buffer[i], i);
			*n += small_n;
		}
	}
	return kStatus_Success;
}

/*!
 * @brief Task responsible for loopback.
 */
static void uart_task(void *pvParameters)
{
    int error;
    size_t n = 0;

    lpuart_config.srcclk = BOARD_DebugConsoleSrcFreq(); //80000000UL ?
    lpuart_config.base   = LPUART2;

    if (kStatus_Success != LPUART_RTOS_Init(&handle, &t_handle, &lpuart_config))
    {
        vTaskSuspend(NULL);
    }

    /* Send introduction message. */
    if (kStatus_Success != uart_send(&handle, (uint8_t *)to_send, strlen(to_send))){
    	vTaskSuspend( NULL );
    }
    PRINTF("message sent\n");

    /* Receive user input and send it back to terminal. */
    do
    {
    	n = 0;
        error = uart_request(&handle, recv_buffer, 16, &n);

        PRINTF("n = %d\n", n);
        if (error == kStatus_LPUART_RxHardwareOverrun)
        {
            /* Notify about hardware buffer overrun */
            if (kStatus_Success !=
				uart_send(&handle, (uint8_t *)send_hardware_overrun, strlen(send_hardware_overrun)))
            {
                vTaskSuspend(NULL);
            }
        }
        if (error == kStatus_LPUART_RxRingBufferOverrun)
        {
            /* Notify about ring buffer overrun */
            if (kStatus_Success != uart_send(&handle, (uint8_t *)send_ring_overrun, strlen(send_ring_overrun)))
            {
                vTaskSuspend(NULL);
            }
        }
        if (n > 0)
        {
            /* send back the received data */
            if (kStatus_Success != uart_send(&handle, (uint8_t *)recv_buffer, 16))
            {
                vTaskSuspend(NULL);
            }
        }
//        vTaskDelay(1000);
    } while (kStatus_Success == error);

    LPUART_RTOS_Deinit(&handle);
    vTaskSuspend(NULL);
}
