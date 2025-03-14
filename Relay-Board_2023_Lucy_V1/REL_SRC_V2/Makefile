BUILD_DIR = build/Debug
TARGET = REL_SRC_V2
BINARY = $(BUILD_DIR)/$(TARGET).elf

all: desc gen

desc:
	@echo "Generating Binary"

gen:
	ninja -C $(BUILD_DIR) 

clean:
	ninja clean -C $(BUILD_DIR) 

# Need only specify the address 0x08000000 if we aren't flashing using
# an .elf file. I do not know how thouroughly true that is.
flash_dep: gen
	STM32_Programmer_CLI -c port=SWD -w $(BINARY) 0x08000000 -v -rst -run

flash: gen
	openocd -f "interface/stlink.cfg" -f "target/stm32f4x.cfg" -c "program build/Debug/$(TARGET).elf verify reset exit"
