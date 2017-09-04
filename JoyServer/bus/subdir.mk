CPP_SRCS += \
../bus/Business.cpp \
../bus/DbService.cpp \
../bus/ServicePool.cpp \
../bus/Process.cpp \
../bus/Heartbeat.cpp \
../bus/Test.cpp \


BUS_OBJS += \
./bus/Business.o \
./bus/DbService.o \
./bus/ServicePool.o \
./bus/Process.o \
./bus/Heartbeat.o \
./bus/Test.o \


# Each subdirectory must supply rules for building sources it contributes
bus/%.o: ../bus/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DTCP -DPB_FIELD_16BIT -DPB_MAX_REQUIRED_FIELDS=128 -I../src -I/usr/local/mysql/include -I/usr/include/mysql -I../. -O0 -g3 -c -fmessage-length=0 -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '
