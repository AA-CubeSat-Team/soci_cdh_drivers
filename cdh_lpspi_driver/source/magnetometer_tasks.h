#ifndef MAG_LPSPI_H
#define MAG_LPSPI_H

#include "peripherals.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "fsl_lpspi.h"
#include "fsl_lpspi_freertos.h"
#include "board.h"

#include "fsl_common.h"
#include "pin_mux.h"

#define LPSPI_MASTER_BASEADDR (LPSPI3)
#define LPSPI_MASTER_PCS_FOR_TRANSFER (kLPSPI_MasterPcs0)
#define MAG_BUFFER 10

#define HMC5983_CONFIG_A_REG 0x00
#define HMC5983_CONFIG_B_REG 0x01
#define HMC5983_MODE_REG     0x02
#define HMC5983_MSB_X_REG    0x03
#define HMC5983_LSB_X_REG    0x04
#define HMC5983_MSB_Z_REG    0x05
#define HMC5983_LSB_Z_REG    0x06
#define HMC5983_MSB_Y_REG    0x07
#define HMC5983_LSB_Y_REG    0x08
#define HMC5983_STATUS_REG   0x09
#define HMC5983_ID_A         0x0A
#define HMC5983_ID_B         0x0B
#define HMC5983_ID_C         0x0C
#define HMC5983_MSB_TEMP     0x31
#define HMC5983_LSB_TEMP     0x32

#define HMC5983Gain08   0x00
#define HMC5983Gain13   0x20
#define HMC5983Gain19   0x40
#define HMC5983Gain25   0x60
#define HMC5983Gain40   0x80
#define HMC5983Gain47   0xA0
#define HMC5983Gain56   0xC0
#define HMC5983Gain81   0xE0


/*
 * Task for magnetometer
 */
void magnetometer_task(void* pvParameters);


#endif
