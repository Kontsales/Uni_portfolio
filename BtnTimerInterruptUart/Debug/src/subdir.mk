################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
LD_SRCS += \
../src/lscript.ld 

C_SRCS += \
../src/interrupts.c \
../src/main.c \
../src/uart_comunication.c 

OBJS += \
./src/interrupts.o \
./src/main.o \
./src/uart_comunication.o 

C_DEPS += \
./src/interrupts.d \
./src/main.d \
./src/uart_comunication.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 gcc compiler'
	arm-none-eabi-gcc -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../BtnTimerInterruptUart_bsp/ps7_cortexa9_0/include -I"F:\Xilinx\workplaces\BtnTimerInteruptUart\zybo_hw" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


