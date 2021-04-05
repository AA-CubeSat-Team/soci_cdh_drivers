
#include "magnetometer_tasks.h"

// Forward declaration
float hmc5983_convert_hex_2_field(int16_t val, uint8_t hmc593GainVal);
uint8_t readConfigA(lpspi_rtos_handle_t* rtos_handle,  uint8_t* sendBuffer, uint8_t* receiveBuffer);
uint8_t readConfigB(lpspi_rtos_handle_t* rtos_handle,  uint8_t* sendBuffer, uint8_t* receiveBuffer);
uint8_t readMode(lpspi_rtos_handle_t* rtos_handle,  uint8_t* sendBuffer, uint8_t* receiveBuffer);
void writeMode(lpspi_rtos_handle_t* rtos_handle,  uint8_t* sendBuffer, uint8_t val);
uint8_t readMeasurement(lpspi_rtos_handle_t* rtos_handle,  uint8_t* sendBuffer, uint8_t* receiveBuffer);
uint8_t read_mag_x(lpspi_rtos_handle_t* rtos_handle, uint8_t* sendBuffer, uint8_t* receiveBuffer);
uint8_t read_mag_y(lpspi_rtos_handle_t* rtos_handle, uint8_t* sendBuffer, uint8_t* receiveBuffer);
uint8_t read_mag_z(lpspi_rtos_handle_t* rtos_handle, uint8_t* sendBuffer, uint8_t* receiveBuffer);
status_t sendMessage(lpspi_rtos_handle_t* master_rtos_handle, uint8_t* sendBuffer, uint8_t* receiveBuffer, uint8_t transferSize);


void magnetometer_task(void* pvParameters){
    lpspi_rtos_handle_t master_rtos_handle;
    status_t status;

    // Get the handle from the main function
    extern QueueHandle_t xQueue;
    extern TaskHandle_t QueTaskHandle;
    extern int32_t MagCounter;

    const TickType_t xDelay500ms = pdMS_TO_TICKS( 500 );
    const TickType_t xBlockTime = pdMS_TO_TICKS( 50 );

    PRINTF("Reading Configuration B of magnetometer.\r\n");

    status = LPSPI_RTOS_Init(&master_rtos_handle, LPSPI_MASTER_BASEADDR, &LPSPI_3_config, LPSPI_3_CLOCK_FREQ);
    if (status != kStatus_Success)
    {
        PRINTF("LPSPI master: error during initialization. \r\n");
        vTaskSuspend(NULL);
    }

    uint8_t sendBuffer[MAG_BUFFER] = {0};
    uint8_t receiveBuffer[MAG_BUFFER] = {0};

    readMode(&master_rtos_handle, sendBuffer, receiveBuffer);
    writeMode(&master_rtos_handle, sendBuffer,  0x00);
    readMode(&master_rtos_handle, sendBuffer, receiveBuffer);

    uint8_t gainVal = 0x00;

    uint8_t configBSize = readConfigB(&master_rtos_handle, sendBuffer, receiveBuffer);
    if(configBSize != 0){
    	gainVal = receiveBuffer[1];
    }else{
        vTaskSuspend(NULL);
    }

    uint32_t counter = 0;
    uint8_t xSize;
    uint8_t ySize;
    uint8_t zSize;

    int16_t xVal;
    int16_t yVal;
    int16_t zVal;

    float fXval;
    float fYval;
    float fZval;
    while(counter < MagCounter){
    	PRINTF("Measurement %d\r\n", counter);

    	// Read the x measurement of the mangetometer
    	xSize = read_mag_x(&master_rtos_handle, sendBuffer, receiveBuffer);
    	if(xSize != 0){
			xVal = receiveBuffer[1] << 8 | receiveBuffer[2];
    		fXval = hmc5983_convert_hex_2_field(xVal, gainVal);
    		//xQueueSendToBack( xQueue,(void*) &fXval, xBlockTime );
    		xQueueSend( xQueue,(void*) &fXval, xBlockTime );
			PRINTF("Sent X mag data to Queue: %f\r\n", fXval);
			//PRINTF("X magnetic val: %f \r\n", hmc5983_convert_hex_2_field(xVal, gainVal));
		}else{
    		PRINTF("FAILED GETTING X MAG VALUE\r\n");
		}

    	//Read the y measurement of the magnetometer
    	ySize = read_mag_y(&master_rtos_handle, sendBuffer, receiveBuffer);
    	if(ySize != 0){
    		yVal = receiveBuffer[1] << 8 | receiveBuffer[2];
    		fYval = hmc5983_convert_hex_2_field(yVal, gainVal);
    		//xQueueSendToBack( xQueue,(void*) &fYval, xBlockTime );
    		xQueueSend( xQueue,(void*) &fYval, xBlockTime );
			PRINTF("Sent Y mag data to Queue: %f\r\n", fYval);
    		//PRINTF("Y magnetic val: %f \r\n", hmc5983_convert_hex_2_field(yVal, gainVal));
		}else{
    		PRINTF("FAILED GETTING Y MAG VALUE\r\n");
		}

    	// Read the z measurement of the magnetometer
    	zSize = read_mag_z(&master_rtos_handle, sendBuffer, receiveBuffer);
    	if(zSize != 0){
    		zVal = receiveBuffer[1] << 8 | receiveBuffer[2];
    		fZval = hmc5983_convert_hex_2_field(zVal, gainVal);
			PRINTF("Sent Z mag data to Queue: %f\r\n", fZval);
			//xQueueSendToBack(xQueue, (void*)&fZval, xBlockTime );
    		xQueueSend( xQueue, (void*)&fZval, xBlockTime );
    		//PRINTF("Z magnetic val: %f \r\n", hmc5983_convert_hex_2_field(zVal, gainVal));
    	}else{
    		PRINTF("FAILED GETTING Z MAG VALUE\r\n");
    	}

    	// Resume the Queue Receiver Task so that it could
    	// look at the Queue
    	vTaskResume(QueTaskHandle);
    	// Suspend self so that the Queue task could run.
    	vTaskSuspend(NULL);

    	// Add 500 ms delay
    	vTaskDelay(xDelay500ms);
    	counter++;
    }


    // Deinitialize the LPSPI and related RTOS
    LPSPI_RTOS_Deinit(&master_rtos_handle);

    PRINTF("Finished Magnetometer Task\r\n");
    vTaskSuspend(NULL);
}

/* Convert the hex value to Gauss value
 * Depending on the Configuration Gain Value
 *
 */
float hmc5983_convert_hex_2_field(int16_t val, uint8_t hmc5983GainVal){
  float field = 0.0;
  if(hmc5983GainVal == HMC5983Gain08)
    field = val/1370.0;
  else if(hmc5983GainVal == HMC5983Gain13)
    field = val/1090.0;
  else if(hmc5983GainVal == HMC5983Gain19)
    field = val/820.0;
  else if(hmc5983GainVal ==HMC5983Gain25)
    field = val/660.0;
  else if(hmc5983GainVal == HMC5983Gain40)
    field = val/440.0;
  else if(hmc5983GainVal == HMC5983Gain47)
    field = val/390.0;
  else if(hmc5983GainVal == HMC5983Gain56)
    field = val/330.0;
  else
    field = val/230.0;
  return field;
}


/* Get the value in configuration A
 *
 * Returns: the size of the buffer which contains the received data
 */
uint8_t readConfigA(lpspi_rtos_handle_t* rtos_handle,  uint8_t* sendBuffer, uint8_t* receiveBuffer)
{
    uint8_t transferSize = 2;
    sendBuffer[0] = 0x80 | HMC5983_CONFIG_A_REG;

	status_t status = sendMessage(rtos_handle, sendBuffer, receiveBuffer, transferSize);

    if (status == kStatus_Success)
    {
        PRINTF("Able to read configuration A.\r\n");
        for(int j =0 ; j < transferSize; j++){
        	PRINTF("data %d: Input = 0x%x \t0x%x\r\n", j, sendBuffer[j], receiveBuffer[j]);
        }
        return transferSize;
    }

    PRINTF("Error in reading configuration A.\r\n");
    return 0;
}

/* Get the value in configuration B
 *
 * Returns: the size of the buffer which contains the received data
 */
uint8_t readConfigB(lpspi_rtos_handle_t* rtos_handle, uint8_t* sendBuffer, uint8_t* receiveBuffer)
{
    uint8_t transferSize = 2;
    sendBuffer[0] = 0x80 | HMC5983_CONFIG_B_REG;

	status_t status = sendMessage(rtos_handle, sendBuffer, receiveBuffer, transferSize);

    if (status == kStatus_Success)
    {
        PRINTF("Able to read configuration B.\r\n");
        for(int j =0 ; j < transferSize; j++){
        	PRINTF("data %d: Input = 0x%x \t0x%x\r\n", j, sendBuffer[j], receiveBuffer[j]);
        }
        return transferSize;
    }
	PRINTF("Error in reading configuration B.\r\n");
	return 0;
}

/* Get the value in the mode register
 *
 * Returns: the size of the buffer which contains the received data
 */
uint8_t readMode(lpspi_rtos_handle_t* rtos_handle,  uint8_t* sendBuffer, uint8_t* receiveBuffer)
{
    uint8_t transferSize = 2;
    sendBuffer[0] = 0x80 | HMC5983_MODE_REG;

	status_t status = sendMessage(rtos_handle, sendBuffer, receiveBuffer, transferSize);

    if (status == kStatus_Success)
    {
        PRINTF("Able to read Mode Register.\r\n");
        for(int j =0 ; j < transferSize; j++){
        	PRINTF("data %d: Input = 0x%x \t0x%x\r\n", j, sendBuffer[j], receiveBuffer[j]);
        }
        return transferSize;
    }

	PRINTF("Error in reading Mode Register.\r\n");
	return 0;
}

/*
 *  Write a value to the Mode Register
 */
void writeMode(lpspi_rtos_handle_t* rtos_handle,  uint8_t* sendBuffer, uint8_t val){
	uint8_t transferSize = 2;
	sendBuffer[0] = HMC5983_MODE_REG;
	sendBuffer[1] = val;

	status_t status = sendMessage(rtos_handle, sendBuffer, NULL, transferSize);

	if (status == kStatus_Success)
	{
		PRINTF("Able to write Mode Register.\r\n");
	}
	else
	{
		PRINTF("Error in reading Mode Register.\r\n");
	}
}


/* Get all 6 measurement data
 *
 * Returns: the size of the buffer which contains the received data
 */
uint8_t readMeasurement(lpspi_rtos_handle_t* rtos_handle,  uint8_t* sendBuffer, uint8_t* receiveBuffer)
{
    uint8_t transferSize = 7;
    sendBuffer[0] = 0xc0 | HMC5983_MSB_X_REG;

    status_t status = sendMessage(rtos_handle, sendBuffer, receiveBuffer, transferSize);

    if (status == kStatus_Success)
    {
        PRINTF("Able to read measurements.\r\n");
        for(int j =0 ; j < transferSize; j++){
        	PRINTF("data %d: Input = 0x%x \t0x%x\r\n", j, sendBuffer[j], receiveBuffer[j]);
        }
        return transferSize;
    }
    PRINTF("Error in reading Measurements.\r\n");
    return 0;
}

/*
 * Read the x magnetic field of the magnetometer
 */
uint8_t read_mag_x(lpspi_rtos_handle_t* rtos_handle, uint8_t* sendBuffer, uint8_t* receiveBuffer){
	uint8_t transferSize = 3;
	sendBuffer[0] = 0xC0 | HMC5983_MSB_X_REG;
    status_t status = sendMessage(rtos_handle, sendBuffer, receiveBuffer, transferSize);

    if (status == kStatus_Success)
    {
        return transferSize;
    }
    return 0;
}

/*
 * Read the y magnetic field of the magnetometer
 */
uint8_t read_mag_y(lpspi_rtos_handle_t* rtos_handle, uint8_t* sendBuffer, uint8_t* receiveBuffer){
	uint8_t transferSize = 3;
	sendBuffer[0] = 0xC0 | HMC5983_MSB_Y_REG;
    status_t status = sendMessage(rtos_handle, sendBuffer, receiveBuffer, transferSize);

    if (status == kStatus_Success)
    {
        return transferSize;
    }
    return 0;
}

/*
 * Read the z magnetic field of the magnetometer
 */
uint8_t read_mag_z(lpspi_rtos_handle_t* rtos_handle, uint8_t* sendBuffer, uint8_t* receiveBuffer){
	uint8_t transferSize = 3;
	sendBuffer[0] = 0xC0 | HMC5983_MSB_Z_REG;
    status_t status = sendMessage(rtos_handle, sendBuffer, receiveBuffer, transferSize);

    if (status == kStatus_Success)
    {
        return transferSize;
    }
    return 0;
}

/*
 * Send the message using SPI protocol
 */
status_t sendMessage(lpspi_rtos_handle_t* master_rtos_handle, uint8_t* sendBuffer, uint8_t* receiveBuffer, uint8_t transferSize){
    lpspi_transfer_t masterXfer;
    status_t status;

    /*Start master transfer*/
    masterXfer.txData      = sendBuffer;
    masterXfer.rxData      = receiveBuffer;
    masterXfer.dataSize    = transferSize;
    masterXfer.configFlags = LPSPI_MASTER_PCS_FOR_TRANSFER | kLPSPI_MasterPcsContinuous | kLPSPI_SlaveByteSwap;

    status = LPSPI_RTOS_Transfer(master_rtos_handle, &masterXfer);

    return status;
}

