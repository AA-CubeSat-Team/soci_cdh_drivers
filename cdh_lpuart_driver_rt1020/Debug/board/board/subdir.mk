################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../board/board/peripherals.c \
../board/board/pin_mux.c 

OBJS += \
./board/board/peripherals.o \
./board/board/pin_mux.o 

C_DEPS += \
./board/board/peripherals.d \
./board/board/pin_mux.d 


# Each subdirectory must supply rules for building sources it contributes
board/board/%.o: ../board/board/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -std=gnu99 -D__REDLIB__ -DCPU_MIMXRT1021DAG5A -DSDK_DEBUGCONSOLE=1 -DXIP_EXTERNAL_FLASH=1 -DXIP_BOOT_HEADER_ENABLE=1 -DSERIAL_PORT_TYPE_UART=1 -DFSL_RTOS_FREE_RTOS -DCPU_MIMXRT1021DAG5A_cm7 -DCR_INTEGER_PRINTF -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -I"/Users/stlp/Documents/MCUX_workspace/evkmimxrt1020_freertos_lpuart2/board" -I"/Users/stlp/Documents/MCUX_workspace/evkmimxrt1020_freertos_lpuart2/source" -I"/Users/stlp/Documents/MCUX_workspace/evkmimxrt1020_freertos_lpuart2" -I"/Users/stlp/Documents/MCUX_workspace/evkmimxrt1020_freertos_lpuart2/drivers" -I"/Users/stlp/Documents/MCUX_workspace/evkmimxrt1020_freertos_lpuart2/device" -I"/Users/stlp/Documents/MCUX_workspace/evkmimxrt1020_freertos_lpuart2/CMSIS" -I"/Users/stlp/Documents/MCUX_workspace/evkmimxrt1020_freertos_lpuart2/drivers/freertos" -I"/Users/stlp/Documents/MCUX_workspace/evkmimxrt1020_freertos_lpuart2/freertos/freertos_kernel/include" -I"/Users/stlp/Documents/MCUX_workspace/evkmimxrt1020_freertos_lpuart2/freertos/freertos_kernel/portable/GCC/ARM_CM4F" -I"/Users/stlp/Documents/MCUX_workspace/evkmimxrt1020_freertos_lpuart2/utilities" -I"/Users/stlp/Documents/MCUX_workspace/evkmimxrt1020_freertos_lpuart2/component/serial_manager" -I"/Users/stlp/Documents/MCUX_workspace/evkmimxrt1020_freertos_lpuart2/component/lists" -I"/Users/stlp/Documents/MCUX_workspace/evkmimxrt1020_freertos_lpuart2/component/uart" -I"/Users/stlp/Documents/MCUX_workspace/evkmimxrt1020_freertos_lpuart2/xip" -O0 -fno-common -g3 -Wall -c  -ffunction-sections  -fdata-sections  -ffreestanding  -fno-builtin -fmerge-constants -fmacro-prefix-map="../$(@D)/"=. -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -D__REDLIB__ -fstack-usage -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


