################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/iterator/ContainerIterator.cpp \
../src/iterator/MetaIterator.cpp 

OBJS += \
./src/iterator/ContainerIterator.o \
./src/iterator/MetaIterator.o 

CPP_DEPS += \
./src/iterator/ContainerIterator.d \
./src/iterator/MetaIterator.d 


# Each subdirectory must supply rules for building sources it contributes
src/iterator/%.o: ../src/iterator/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -I/usr/include/openssl/ -I/usr/local/include/ -O3 -msse4.2 -g -Wall -c -fmessage-length=0  -DVERSION=\"$(GIT_VERSION)\" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

