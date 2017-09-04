################################################################################
# Automatically-generated file. Do not edit!
################################################################################

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include bus/subdir.mk
-include objects.mk

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: JoyServer

# Tool invocations
JoyServer: $(SRC_OBJS) $(BUS_OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C++ Linker'
	g++ -L$(HOME)/boost/stage/lib -L$(HOME)/glog/lib -L/usr/lib64/mysql -L/usr/local/lib -L/usr/local/mysql/lib -L../. -o "JoyServer" $(SRC_OBJS) $(BUS_OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(SRC_OBJS)$(BUS_OBJS) JoyServer
	-@echo ' '

clean_bus:
	-$(RM) $(BUS_OBJS) JoyServer
	-@echo ' '

post-build:
	-mv JoyServer ../Debug/
	-@echo ' '

.PHONY: all clean clean_bus
.SECONDARY: post-build