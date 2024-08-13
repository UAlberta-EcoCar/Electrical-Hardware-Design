BUILD_DIR = build/Debug
TARGET = FCC_CMAKE_TEST
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
flash: gen
	STM32_Programmer_CLI -c port=SWD -w $(BINARY) 0x08000000 -v -rst -run
