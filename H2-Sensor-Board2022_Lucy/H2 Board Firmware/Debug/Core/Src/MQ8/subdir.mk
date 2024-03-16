################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/MQ8/mq8calibration.c 

OBJS += \
./Core/Src/MQ8/mq8calibration.o 

C_DEPS += \
./Core/Src/MQ8/mq8calibration.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/MQ8/%.o Core/Src/MQ8/%.su Core/Src/MQ8/%.cyclo: ../Core/Src/MQ8/%.c Core/Src/MQ8/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L432xx -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-MQ8

clean-Core-2f-Src-2f-MQ8:
	-$(RM) ./Core/Src/MQ8/mq8calibration.cyclo ./Core/Src/MQ8/mq8calibration.d ./Core/Src/MQ8/mq8calibration.o ./Core/Src/MQ8/mq8calibration.su

.PHONY: clean-Core-2f-Src-2f-MQ8

