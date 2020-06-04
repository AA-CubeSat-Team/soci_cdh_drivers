/* FreeRTOS kernel includes. */
#include <lpuart.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

/* Freescale includes. */
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "board.h"

#include "pin_mux.h"
#include "clock_config.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/


/*******************************************************************************
 * Global Variables
 ******************************************************************************/


/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Main function
 */

int main(void)
{
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole(); //this uses UART1 to print stuff back

    init_uart_tasks();
    vTaskStartScheduler();
    // The system should never reach here.

    for (;;);
}


