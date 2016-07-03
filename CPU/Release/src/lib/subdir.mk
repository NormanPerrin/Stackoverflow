################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/lib/primitivasAnSISOP.c \
../src/lib/principalesCPU.c \
../src/lib/secundariasCPU.c \
../src/lib/secundariasPrimitivas.c 

OBJS += \
./src/lib/primitivasAnSISOP.o \
./src/lib/principalesCPU.o \
./src/lib/secundariasCPU.o \
./src/lib/secundariasPrimitivas.o 

C_DEPS += \
./src/lib/primitivasAnSISOP.d \
./src/lib/principalesCPU.d \
./src/lib/secundariasCPU.d \
./src/lib/secundariasPrimitivas.d 


# Each subdirectory must supply rules for building sources it contributes
src/lib/%.o: ../src/lib/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


