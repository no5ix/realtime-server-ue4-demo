################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/profile_config.cpp \
../src/Main.cpp \
../src/Worker.cpp \
../src/xxtea.cpp \

SRC_OBJS += \
./src/profile_config.o \
./src/Main.o \
./src/Worker.o \
./src/xxtea.o \

# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DTCP -DPB_FIELD_16BIT -DPB_MAX_REQUIRED_FIELDS=128 -I../src -I/usr/local/mysql/include -I/usr/include/mysql -I../. -O0 -g3 -c -fmessage-length=0  -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


