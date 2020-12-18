################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Startup/subdir.mk
-include Src/subdir.mk
-include Drivers/STM32F1xx_HAL_Driver/Src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
SMP_Projekt.elf \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
SMP_Projekt.list \

OBJCOPY_BIN += \
SMP_Projekt.bin \


# All Target
all: SMP_Projekt.elf secondary-outputs

# Tool invocations
SMP_Projekt.elf: $(OBJS) $(USER_OBJS) C:\Users\skorp\Desktop\projects\HAL_embedded_C_tutorial\SMP_Projekt\STM32F103RBTX_FLASH.ld
	arm-none-eabi-gcc -o "SMP_Projekt.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m3 -T"C:\Users\skorp\Desktop\projects\HAL_embedded_C_tutorial\SMP_Projekt\STM32F103RBTX_FLASH.ld" --specs=nosys.specs -Wl,-Map="SMP_Projekt.map" -Wl,--gc-sections -static --specs=nano.specs -mfloat-abi=soft -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

SMP_Projekt.list: $(EXECUTABLES)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "SMP_Projekt.list"
	@echo 'Finished building: $@'
	@echo ' '

SMP_Projekt.bin: $(EXECUTABLES)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "SMP_Projekt.bin"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) *
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_BIN)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
