/*
 * grycoscope.c
 *
 *  Created on: Mar 3, 2020
 *      Author: Hirotoshi Maeda
 */


#include "gyroscope_task.h"
#include "structDef.h"

// A dummy gyroscope task.
// TODO implement actual gyroscope task
void gyroscope_task(void* pvParameters){
	//lpspi_rtos_handle_t master_rtos_handle;
	//status_t status;

	// Get the handle from the main function
	extern QueueHandle_t xGyroQueue;
	extern int32_t GyroMaxCounter;

	// Make sure that the queue is not NULL
//	assert(xGyroQueue != NULL);
//	assert(pvParameters != NULL);

	/*  Defining which sensor's we're using
	 *  and the message we're going to send to queue.
	 */
	Data_t message;
	message.id = (SensorID) pvParameters;
	message.flag = true;

	const TickType_t xDelay200ms = pdMS_TO_TICKS( 200 );
	const TickType_t xBlockTime = pdMS_TO_TICKS( 50 );

	uint32_t counter = 0;

	float fXval;
	float fYval;
	float fZval;
	while(counter < GyroMaxCounter){
		if(PRINT_GYRO_STATUS)
			PRINTF("Gyroscope measurement %d\r\n", counter);

		fXval = counter+1;
		fYval = counter+2;
		fZval = counter +3;

		if(PRINT_GYRO_STATUS){
			PRINTF("Gyro%d: Sent gyroscope X = %f\r\n", message.id-2, fXval);
			PRINTF("Gyro%d: Sent gyroscope Y = %f\r\n", message.id-2, fYval);
			PRINTF("Gyro%d: Sent gyroscope Z = %f\r\n", message.id-2, fZval);
		}
		message.x = fXval;
		message.y = fYval;
		message.z = fZval;
		xQueueSendToBack(xGyroQueue, (void*)&message, xBlockTime );

		// Suspend self so that the Queue task could run.
		vTaskSuspend(NULL);

		// Add 200 ms delay
		vTaskDelay(xDelay200ms);
		counter++;
	}

	message.flag = false;
	xQueueSendToBack(xGyroQueue, (void*)&message, xBlockTime );

	// Deinitialize the LPSPI and related RTOS
	//LPSPI_RTOS_Deinit(&master_rtos_handle);

	PRINTF("Finished Gyroscope Task %d\r\n", message.id);
	//vTaskResume(QueTaskHandle);
	vTaskSuspend(NULL);
}


