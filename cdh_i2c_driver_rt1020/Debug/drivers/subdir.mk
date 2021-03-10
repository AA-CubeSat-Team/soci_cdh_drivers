################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../drivers/fsl_clock.c \
../drivers/fsl_common.c \
../drivers/fsl_gpio.c \
../drivers/fsl_lpi2c.c \
../drivers/fsl_lpuart.c 

OBJS += \
./drivers/fsl_clock.o \
./drivers/fsl_common.o \
./drivers/fsl_gpio.o \
./drivers/fsl_lpi2c.o \
./drivers/fsl_lpuart.o 

C_DEPS += \
./drivers/fsl_clock.d \
./drivers/fsl_common.d \
./drivers/fsl_gpio.d \
./drivers/fsl_lpi2c.d \
./drivers/fsl_lpuart.d 


# Each subdirectory must supply rules for building sources it contributes
drivers/%.o: ../drivers/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -std=gnu99 -D__REDLIB__ -DCPU_MIMXRT1021DAG5A -DSDK_DEBUGCONSOLE=1 -DXIP_EXTERNAL_FLASH=1 -DXIP_BOOT_HEADER_ENABLE=1 -DSERIAL_PORT_TYPE_UART=1 -DFSL_RTOS_FREE_RTOS -DCPU_MIMXRT1021DAG5A_cm7 -DCR_INTEGER_PRINTF -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -I"/Users/stlp/Documents/MCUX_workspace/soci_cdh_drivers/cdh_i2c_driver_rt1020/board" -I"/Users/stlp/Documents/MCUX_workspace/soci_cdh_drivers/cdh_i2c_driver_rt1020/source" -I"/Users/stlp/Documents/MCUX_workspace/soci_cdh_drivers/cdh_i2c_driver_rt1020" -I"/Users/stlp/Documents/MCUX_workspace/soci_cdh_drivers/cdh_i2c_driver_rt1020/drivers" -I"/Users/stlp/Documents/MCUX_workspace/soci_cdh_drivers/cdh_i2c_driver_rt1020/device" -I"/Users/stlp/Documents/MCUX_workspace/soci_cdh_drivers/cdh_i2c_driver_rt1020/CMSIS" -I"/Users/stlp/Documents/MCUX_workspace/soci_cdh_drivers/cdh_i2c_driver_rt1020/drivers/freertos" -I"/Users/stlp/Documents/MCUX_workspace/soci_cdh_drivers/cdh_i2c_driver_rt1020/freertos/freertos_kernel/include" -I"/Users/stlp/Documents/MCUX_workspace/soci_cdh_drivers/cdh_i2c_driver_rt1020/freertos/freertos_kernel/portable/GCC/ARM_CM4F" -I"/Users/stlp/Documents/MCUX_workspace/soci_cdh_drivers/cdh_i2c_driver_rt1020/utilities" -I"/Users/stlp/Documents/MCUX_workspace/soci_cdh_drivers/cdh_i2c_driver_rt1020/component/serial_manager" -I"/Users/stlp/Documents/MCUX_workspace/soci_cdh_drivers/cdh_i2c_driver_rt1020/component/lists" -I"/Users/stlp/Documents/MCUX_workspace/soci_cdh_drivers/cdh_i2c_driver_rt1020/component/uart" -I"/Users/stlp/Documents/MCUX_workspace/soci_cdh_drivers/cdh_i2c_driver_rt1020/xip" -O0 -fno-common -g3 -Wall -c  -ffunction-sections  -fdata-sections  -ffreestanding  -fno-builtin -fmerge-constants -fmacro-prefix-map="../$(@D)/"=. -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -D__REDLIB__ -fstack-usage -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

