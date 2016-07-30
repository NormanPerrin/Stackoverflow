################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../utilidades/comunicaciones.c \
../utilidades/general.c \
../utilidades/sockets.c 

OBJS += \
./utilidades/comunicaciones.o \
./utilidades/general.o \
./utilidades/sockets.o 

C_DEPS += \
./utilidades/comunicaciones.d \
./utilidades/general.d \
./utilidades/sockets.d 


# Each subdirectory must supply rules for building sources it contributes
utilidades/%.o: ../utilidades/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


