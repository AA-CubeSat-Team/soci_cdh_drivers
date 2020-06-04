################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../amazon-freertos/freertos/event_groups.c \
../amazon-freertos/freertos/list.c \
../amazon-freertos/freertos/queue.c \
../amazon-freertos/freertos/stream_buffer.c \
../amazon-freertos/freertos/tasks.c \
../amazon-freertos/freertos/timers.c 

OBJS += \
./amazon-freertos/freertos/event_groups.o \
./amazon-freertos/freertos/list.o \
./amazon-freertos/freertos/queue.o \
./amazon-freertos/freertos/stream_buffer.o \
./amazon-freertos/freertos/tasks.o \
./amazon-freertos/freertos/timers.o 

C_DEPS += \
./amazon-freertos/freertos/event_groups.d \
./amazon-freertos/freertos/list.d \
./amazon-freertos/freertos/queue.d \
./amazon-freertos/freertos/stream_buffer.d \
./amazon-freertos/freertos/tasks.d \
./amazon-freertos/freertos/timers.d 


# Each subdirectory must supply rules for building sources it contributes
amazon-freertos/freertos/%.o: ../amazon-freertos/freertos/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -std=gnu99 -D__REDLIB__ -DCPU_MIMXRT1052DVL6B -DCPU_MIMXRT1052DVL6B_cm7 -DSDK_DEBUGCONSOLE=1 -DXIP_EXTERNAL_FLASH=1 -DXIP_BOOT_HEADER_ENABLE=1 -DFSL_RTOS_FREE_RTOS -DCR_INTEGER_PRINTF -DPRINTF_FLOAT_ENABLE=0 -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -I../board -I../source -I../ -I../drivers -I../device -I../CMSIS -I../drivers/freertos -I../amazon-freertos/freertos/portable -I../amazon-freertos/include -I../utilities -I../component/lists -I../component/serial_manager -I../component/uart -I../xip -O0 -fno-common -g3 -Wall -c  -ffunction-sections  -fdata-sections  -ffreestanding  -fno-builtin -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -D__REDLIB__ -fstack-usage -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


