# Copyright 2012 Emilie Gillet.
#
# Author: Emilie Gillet (emilie.o.gillet@gmail.com)
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
# 
# See http://creativecommons.org/licenses/MIT/ for more information.

# ------------------------------------------------------------------------------
# Files and directories
# ------------------------------------------------------------------------------

TOOLCHAIN_PATH ?= /usr/local/arm/

PGM_INTERFACE ?= stlink-v2
# PGM_INTERFACE ?= ftdi/olimex-arm-jtag-swd
# PGM_INTERFACE ?= ftdi/olimex-arm-usb-ocd-h

ifeq ($(PGM_INTERFACE),ftdi/olimex-arm-jtag-swd)
OPENOCD_INTERFACE_SCRIPT ?= interface/ftdi/olimex-arm-usb-ocd-h.cfg -f interface/ftdi/olimex-arm-jtag-swd.cfg
else
OPENOCD_INTERFACE_SCRIPT ?= interface/$(PGM_INTERFACE).cfg
endif

OPENOCD_SCRIPT_PATH ?= /opt/local/share/openocd/scripts/

PGM_SERIAL_PORT ?= /dev/ftdi-usbserial
PGM_SERIAL_BAUD_RATE ?= 115200
PGM_SERIAL_VERIFY ?= -v

# --- You should not have to modify these --------------------------------------

TOOLCHAIN_BIN  = $(TOOLCHAIN_PATH)bin/
STM32_PATH     = stmlib/third_party/STM

# --- Do not modify anything below this point ----------------------------------

BUILD_ROOT     = build/
BUILD_DIR      = $(BUILD_ROOT)$(TARGET)/

CC             = $(TOOLCHAIN_BIN)arm-none-eabi-gcc
CXX            = $(TOOLCHAIN_BIN)arm-none-eabi-g++
AS             = $(TOOLCHAIN_BIN)arm-none-eabi-as
OBJCOPY        = $(TOOLCHAIN_BIN)arm-none-eabi-objcopy
DB             = $(TOOLCHAIN_BIN)arm-none-eabi-gdb
OBJDUMP        = $(TOOLCHAIN_BIN)arm-none-eabi-objdump
AR             = $(TOOLCHAIN_BIN)arm-none-eabi-ar
SIZE           = $(TOOLCHAIN_BIN)arm-none-eabi-size
NM             = $(TOOLCHAIN_BIN)arm-none-eabi-nm
REMOVE         = rm -f
CAT            = cat
FORMAT_SIZE    = | figlet | cowsay -n -f moose
FORMAT_RAMSIZE = | figlet | cowsay -n -f small

STM32LOADER_PATH = stmlib/programming/serial/
OPENOCD_SCRIPTS_PATH = stmlib/programming/jtag/

# ------------------------------------------------------------------------------
# Files and directories for the user code
# ------------------------------------------------------------------------------

VPATH          = $(PACKAGES)

CC_FILES       = $(notdir $(wildcard $(patsubst %,%/*.cc,$(PACKAGES))))
C_FILES        = $(notdir $(wildcard $(patsubst %,%/*.c,$(PACKAGES))))
AS_FILES       = $(notdir $(wildcard $(patsubst %,%/*.s,$(PACKAGES))))

TARGET_BIN     = $(BUILD_DIR)$(TARGET).bin
TARGET_ELF     = $(BUILD_DIR)$(TARGET).elf
TARGET_HEX     = $(BUILD_DIR)$(TARGET).hex
COMBO_HEX      = $(BUILD_DIR)$(TARGET)_bootloader_combo.hex
COMBO_BIN      = $(BUILD_DIR)$(TARGET)_bootloader_combo.bin
BOOTLOADER_HEX = $(BUILD_ROOT)$(BOOTLOADER)/$(BOOTLOADER).hex
TARGET_SIZE    = $(BUILD_DIR)$(TARGET).size
TARGETS        = $(BUILD_DIR)$(TARGET).*
DEP_FILE       = $(BUILD_DIR)depends.mk

# ------------------------------------------------------------------------------
# Platform specific flags
# ------------------------------------------------------------------------------

ifeq ($(FAMILY),f4xx)

MODEL_DEFINE = STM32F40_41xxx -DSTM32F4XX -DARM_MATH_CM4 -D__FPU_PRESENT
STARTUP_OBJ   = $(BUILD_DIR)startup_stm32f4xx.o
LINKER_SCRIPT = stmlib/linker_scripts/stm32f4xx_flash$(LD_SUFFIX).ld
FW_STDDRIVER_DIR = $(STM32_PATH)/STM32F4xx_StdPeriph_Driver
ARCHFLAGS = -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -mthumb-interwork -funroll-loops
ARCH_DEFINE = GCC_ARMCM4
OPENOCD_TARGET = f4x

else ifeq ($(FAMILY),f37x)

MODEL_DEFINE = STM32F37X -DARM_MATH_CM4 -D__FPU_PRESENT
STARTUP_OBJ   = $(BUILD_DIR)startup_stm32f37x.o
LINKER_SCRIPT = stmlib/linker_scripts/stm32f373x_flash$(LD_SUFFIX).ld
FW_STDDRIVER_DIR = $(STM32_PATH)/STM32F37x_StdPeriph_Driver
ARCHFLAGS = -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -mthumb-interwork -funroll-loops --specs=nano.specs
ARCH_DEFINE = GCC_ARMCM4
OPENOCD_TARGET = f3x

else ifeq ($(FAMILY),f0xx)

MODEL_DEFINE = STM32F0XX
STARTUP_OBJ   = $(BUILD_DIR)startup_stm32f0xx.o
LINKER_SCRIPT = stmlib/linker_scripts/stm32f0xx_flash$(LD_SUFFIX).ld
FW_STDDRIVER_DIR = $(STM32_PATH)/STM32F0xx_StdPeriph_Driver
ARCHFLAGS = -mcpu=cortex-m0 -mthumb -mthumb-interwork -funroll-loops
ARCH_DEFINE = GCC_ARMCM0
OPENOCD_TARGET = f0x

else ifeq ($(FAMILY),f7xx)

MODEL_DEFINE = STM32F7XX -DSTM32F765xx -DARM_MATH_CM7 -D__FPU_PRESENT
STARTUP_OBJ   = $(BUILD_DIR)startup_stm32f765xx.o
LINKER_SCRIPT = stmlib/linker_scripts/stm32f7xx_flash$(LD_SUFFIX).ld
FW_STDDRIVER_DIR = $(STM32_PATH)/STM32F7xx_HAL_Driver
ARCHFLAGS = -mcpu=cortex-m7 -mthumb -mfloat-abi=hard -mfpu=fpv5-d16 -mthumb-interwork -funroll-loops
ARCH_DEFINE = GCC_ARMCM7
OPENOCD_TARGET = f7x

else ifeq ($(FAMILY),h7xx)

MODEL_DEFINE = STM32H7XX -DSTM32H743xx -DARM_MATH_CM7 -D__FPU_PRESENT
STARTUP_OBJ   = $(BUILD_DIR)startup_stm32h743xx.o
LINKER_SCRIPT = stmlib/linker_scripts/stm32h7xx_flash$(LD_SUFFIX).ld
FW_STDDRIVER_DIR = $(STM32_PATH)/STM32H7xx_HAL_Driver
ARCHFLAGS = -mcpu=cortex-m7 -mthumb -mfloat-abi=hard -mfpu=fpv5-d16 -mthumb-interwork -funroll-loops
ARCH_DEFINE = GCC_ARMCM7
OPENOCD_TARGET = h7x_dual_bank

else ifeq ($(FAMILY),g4xx)

MODEL_DEFINE = STM32G4XX -DSTM32G473xx -DARM_MATH_CM7 -D__FPU_PRESENT
STARTUP_OBJ   = $(BUILD_DIR)startup_stm32g473xx.o
LINKER_SCRIPT = stmlib/linker_scripts/stm32g4xx_flash$(LD_SUFFIX).ld
FW_STDDRIVER_DIR = $(STM32_PATH)/STM32G4xx_HAL_Driver
ARCHFLAGS = -mcpu=cortex-m7 -mthumb -mfloat-abi=hard -mfpu=fpv5-d16 -mthumb-interwork -funroll-loops
ARCH_DEFINE = GCC_ARMCM7
OPENOCD_TARGET = g4x

else

ifeq ($(DENSITY),hd)
MODEL_DEFINE = STM32F10X_HD -DSTM32F10X
else ifeq ($(DENSITY),md)
MODEL_DEFINE = STM32F10X_MD -DSTM32F10X
else
MODEL_DEFINE = STM32F10X_CL -DSTM32F10X
endif

STARTUP_OBJ   = $(BUILD_DIR)startup_stm32f10x_$(DENSITY).o
LINKER_SCRIPT = stmlib/linker_scripts/stm32f10x_flash_$(DENSITY)$(LD_SUFFIX).ld
FW_STDDRIVER_DIR = $(STM32_PATH)/STM32F10x_StdPeriph_Driver
ARCHFLAGS = -mcpu=cortex-m3 -mthumb -fno-unroll-loops
ARCH_DEFINE = GCC_ARMCM3
OPENOCD_TARGET = f1x

endif

ifeq ($(APPLICATION),TRUE)
APPLICATION_DEFINE = -DAPPLICATION
LD_SUFFIX = _application
BASE_ADDRESS = 0x08004000
else ifeq ($(APPLICATION_SMALL),TRUE)
APPLICATION_DEFINE = -DAPPLICATION
LD_SUFFIX = _application_small
BASE_ADDRESS = 0x08001000
else ifeq ($(APPLICATION_LARGE),TRUE)
APPLICATION_DEFINE = -DAPPLICATION
LD_SUFFIX = _application_large
BASE_ADDRESS = 0x08008000
else
APPLICATION_DEFINE = -DNO_APPLICATION
LD_SUFFIX =
BASE_ADDRESS = 0x08000000
endif

OPENOCD_TARGET_SCRIPT ?= target/stm32$(OPENOCD_TARGET).cfg

# ------------------------------------------------------------------------------
# Flags for gcc/binutils
# ------------------------------------------------------------------------------

INCFLAGS = \
			-I. \
			-D$(ARCH_DEFINE) \
			-D$(MODEL_DEFINE) \
			$(APPLICATION_DEFINE) \
			$(PROJECT_CONFIGURATION) \
			-DF_CPU=$(F_CPU) \
			-DF_CRYSTAL=$(F_CRYSTAL) \
			-DHSE_VALUE=$(F_CRYSTAL) \
			-DUSE_STDPERIPH_DRIVER \
			-D$(SYSCLOCK)=$(F_CPU)

CCFLAGS = \
			-g -O2 -Wall -Werror -Wno-unused-local-typedefs -Wno-strict-aliasing \
			-Wuninitialized \
			-fasm \
			-finline \
			-finline-functions-called-once \
			-fdata-sections \
			-ffunction-sections \
			-fshort-enums \
			-fno-move-loop-invariants \
			-fno-unwind-tables \
			$(INCFLAGS) \
			$(ARCHFLAGS)

CPPFLAGS = -fno-exceptions -fno-rtti

ASFLAGS = $(ARCHFLAGS)

LDFLAGS = \
			-Wl,-Map=$(BUILD_DIR)$(TARGET).map \
			-Wl,--gc-sections \
			-T $(LINKER_SCRIPT) \
			$(ARCHFLAGS) \
			$(INCFLAGS) \
			-L$(STM32_PATH)


# ------------------------------------------------------------------------------
# Files and directories for the system firmware
# ------------------------------------------------------------------------------

FW_BUILD_DIR = $(BUILD_ROOT)stm_firmware/
FW_CORE_DIR = $(STM32_PATH)/CMSIS/CM3_$(FAMILY)
FW_STARTUP_DIR = $(FW_CORE_DIR)/startup/gcc

FW_STDDRIVER_SRCDIR = $(FW_STDDRIVER_DIR)/src
FW_STDDRIVER_INCDIR = $(FW_STDDRIVER_DIR)/inc

FW_USB_DIR = $(STM32_PATH)/STM32_USB-FS-Device_Driver
FW_USB_SRCDIR = $(FW_USB_DIR)/src
FW_USB_INCDIR = $(FW_USB_DIR)/inc

INCFLAGS += -I$(STM32_PATH) -I$(FW_CORE_DIR) -I$(FW_STARTUP_DIR) -I$(FW_STDDRIVER_INCDIR)
VPATH += $(FW_CORE_DIR) $(FW_STARTUP_DIR) $(FW_STDDRIVER_SRCDIR) $(STM32_PATH) $(FW_BUILD_DIR)

CORE_C_FILES = $(notdir $(wildcard $(patsubst %,%/*.c,$(FW_CORE_DIR))))
DRIVERS_C_FILES = $(notdir $(wildcard $(patsubst %,%/*.c,$(FW_STDDRIVER_SRCDIR))))
USB_C_FILES = $(notdir $(wildcard $(patsubst %,%/*.c,$(FW_USB_SRCDIR))))

C_FILES += $(CORE_C_FILES)
C_FILES += $(DRIVERS_C_FILES)

ifeq ($(USB),enabled)
INCFLAGS += -I$(FW_USB_INCDIR)
VPATH += $(FW_USB_SRCDIR)
C_FILES += $(USB_C_FILES)
endif


# ------------------------------------------------------------------------------
# Source compiling and dependency analysis
# ------------------------------------------------------------------------------

$(BUILD_DIR)%.o: %.s
	$(CC) -c -x assembler-with-cpp $(ASFLAGS) $< -o $@

$(BUILD_DIR)%.o: %.c
	$(CC) -c $(CCFLAGS) -std=c99 $< -o $@

$(BUILD_DIR)%.o: %.cc
	$(CXX) -c $(CCFLAGS) $(CPPFLAGS) $< -o $@

$(BUILD_DIR)%.d: %.s
	$(CC) -c -x assembler-with-cpp -MM $(ASFLAGS) $< -MF $@ -MT $(@:.d=.o)

$(BUILD_DIR)%.d: %.c
	$(CC) -MM $(CCFLAGS) $< -MF $@ -MT $(@:.d=.o)

$(BUILD_DIR)%.d: %.cc
	$(CXX) -MM $(CCFLAGS) $(CPPFLAGS) $< -MF $@ -MT $(@:.d=.o)


# ------------------------------------------------------------------------------
# Object file conversion
# ------------------------------------------------------------------------------

$(BUILD_DIR)%.hex: $(BUILD_DIR)%.elf
	$(OBJCOPY) -O ihex $< $@

$(BUILD_DIR)%.bin: $(BUILD_DIR)%.elf
	$(OBJCOPY) -O binary $< $@

$(BUILD_DIR)%.lss: $(BUILD_DIR)%.elf
	$(OBJDUMP) -d -h -S $< > $@

$(BUILD_DIR)%.sym: $(BUILD_DIR)%.elf
	$(NM) -n $< > $@


# ------------------------------------------------------------------------------
# What to build
# ------------------------------------------------------------------------------

OBJ_FILES      = $(CC_FILES:.cc=.o) $(C_FILES:.c=.o) $(AS_FILES:.s=.o)
OBJS           = $(patsubst %,$(BUILD_DIR)%,$(OBJ_FILES)) $(STARTUP_OBJ)
DEPS           = $(OBJS:.o=.d)

all: $(BUILD_DIR) $(TARGET_HEX)


# ------------------------------------------------------------------------------
# Resources
# ------------------------------------------------------------------------------

RESOURCE_COMPILER = stmlib/tools/resources_compiler.py

resources: $(TARGET)/resources.cc

$(TARGET)/resources.cc: $(wildcard $(RESOURCES)/*.py)
		python $(RESOURCE_COMPILER) $(RESOURCES)/resources.py


# ------------------------------------------------------------------------------
# Main rules
# ------------------------------------------------------------------------------

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET_ELF): $(OBJS)
	$(CC) $(LDFLAGS) -o $(TARGET_ELF) $(OBJS) -lm

$(DEP_FILE): $(BUILD_DIR) $(DEPS)
	cat $(DEPS) > $(DEP_FILE)

bin: $(TARGET_BIN)

hex: $(TARGET_HEX)

clean:
	$(REMOVE) $(OBJS) $(TARGETS) $(DEP_FILE) $(DEPS)

depends: $(DEPS)
	cat $(DEPS) > $(DEP_FILE)

$(TARGET_SIZE): $(TARGET_ELF)
	$(SIZE) $(TARGET_ELF) > $(TARGET_SIZE)

$(BUILD_DIR)$(TARGET).top_symbols: $(TARGET_ELF)
	$(NM) $(TARGET_ELF) --size-sort -C -f bsd -r > $@

size: $(TARGET_SIZE)
	cat $(TARGET_SIZE) | awk '{ print $$1+$$2 }' | tail -n1 $(FORMAT_SIZE)

ramsize: $(TARGET_SIZE)
	cat $(TARGET_SIZE) | awk '{ print $$2+$$3 }' | tail -n1 $(FORMAT_RAMSIZE)

disassemble: build/$(TARGET)/$(TARGET).lss build/$(TARGET)/$(TARGET).top_symbols

.PHONY: all bin clean depends resources

include $(DEP_FILE)


# ------------------------------------------------------------------------------
# Bootloader merging
# ------------------------------------------------------------------------------

$(COMBO_BIN): $(TARGET_HEX) $(BOOTLOADER_HEX)
	cat	$(TARGET_HEX) $(BOOTLOADER_HEX) | \
			awk -f stmlib/programming/merge_hex.awk > $(COMBO_HEX)
	$(OBJCOPY) -I ihex -O binary $(COMBO_HEX) $(COMBO_BIN)


# ------------------------------------------------------------------------------
# Firmware flashing
# ------------------------------------------------------------------------------

BASE_JTAG_CMD = $(OPENOCD_BIN_PATH)openocd \
			-s $(OPENOCD_SCRIPT_PATH) \
			-f $(OPENOCD_INTERFACE_SCRIPT) \
			-f $(OPENOCD_TARGET_SCRIPT) \
			-c "init" -c "halt" -c "sleep 200"

POSTLUDE = -c "sleep 200" -c "reset run" -c "shutdown"

upload: $(UPLOAD_COMMAND)

image_serial:
	python $(STM32LOADER_PATH)stm32loader.py \
		-p $(PGM_SERIAL_PORT) \
		-b $(PGM_SERIAL_BAUD_RATE) \
		-r -l 131072 image.bin

upload_serial: $(TARGET_BIN)
	python $(STM32LOADER_PATH)stm32loader.py \
		-p $(PGM_SERIAL_PORT) \
		-b $(PGM_SERIAL_BAUD_RATE) \
		-e $(PGM_SERIAL_VERIFY) \
		-a $(BASE_ADDRESS) \
		-w $(TARGET_BIN)

upload_serial_no_erase: $(TARGET_BIN)
	python $(STM32LOADER_PATH)stm32loader.py \
		-p $(PGM_SERIAL_PORT) \
		-b $(PGM_SERIAL_BAUD_RATE) $(PGM_SERIAL_VERIFY)\
		-a $(BASE_ADDRESS) \
		-w $(TARGET_BIN)

upload_combo_serial: $(COMBO_BIN)
	python $(STM32LOADER_PATH)stm32loader.py \
		-p $(PGM_SERIAL_PORT) \
		-b $(PGM_SERIAL_BAUD_RATE) \
		-e $(PGM_SERIAL_VERIFY) \
		-w $(COMBO_BIN)

upload_combo_serial_no_erase: $(COMBO_BIN)
	python $(STM32LOADER_PATH)stm32loader.py \
		-p $(PGM_SERIAL_PORT) \
		-b $(PGM_SERIAL_BAUD_RATE) $(PGM_SERIAL_VERIFY)\
		-w $(COMBO_BIN)

image_jtag:
	$(BASE_JTAG_CMD) \
		-c "dump_image image.bin 0x08000000 131072" \
		$(POSTLUDE)

image_jtag_256k:
	$(BASE_JTAG_CMD) \
		-c "dump_image image.bin 0x08000000 262144" \
		$(POSTLUDE)

image_jtag_1M:
	$(BASE_JTAG_CMD) \
		-c "dump_image image.bin 0x08000000 1048576" \
		$(POSTLUDE)

debug_server: $(TARGET_BIN)
	$(OPENOCD_BIN_PATH)openocd \
		-s $(OPENOCD_SCRIPT_PATH) \
		-f $(OPENOCD_INTERFACE_SCRIPT) \
		-f $(OPENOCD_TARGET_SCRIPT) \
		-c "init;reset;halt" \

debug_client:
	$(DB) --eval-command="target remote localhost:3333" $(TARGET_ELF)

erase_jtag:
	$(BASE_JTAG_CMD) \
				-f $(OPENOCD_SCRIPTS_PATH)erase_$(FAMILY).cfg \
				$(POSTLUDE)

upload_jtag: $(TARGET_BIN)
	$(BASE_JTAG_CMD) \
				-f $(OPENOCD_SCRIPTS_PATH)erase_$(FAMILY).cfg \
				-c "flash write_image erase $(TARGET_BIN) 0x08000000" \
				-c "verify_image $(TARGET_BIN) 0x08000000" \
				$(POSTLUDE)

upload_combo_jtag: $(COMBO_BIN)
	$(BASE_JTAG_CMD) \
				-f $(OPENOCD_SCRIPTS_PATH)erase_$(FAMILY).cfg \
				-c "flash write_image erase $(COMBO_BIN) 0x08000000" \
				-c "verify_image $(COMBO_BIN) 0x08000000" \
				$(POSTLUDE)

upload_jtag_no_erase: $(TARGET_BIN)
	$(BASE_JTAG_CMD) \
				-c "flash write_image erase $(TARGET_BIN) 0x08000000" \
				-c "verify_image $(TARGET_BIN) 0x08000000" \
				$(POSTLUDE)

upload_combo_jtag_no_erase: $(COMBO_BIN)
	$(BASE_JTAG_CMD) \
				-c "flash write_image erase $(COMBO_BIN) 0x08000000" \
				-c "verify_image $(COMBO_BIN) 0x08000000" \
				$(POSTLUDE)

upload_jtag_erase_first: $(TARGET_BIN)
	$(BASE_JTAG_CMD) \
				-c "flash erase_address 0x08000000 32768" \
				-c "reset halt" \
				-c "flash write_image erase $(TARGET_BIN) 0x08000000" \
				-c "verify_image $(TARGET_BIN) 0x08000000" \
				$(POSTLUDE)

upload_combo_jtag_erase_first: $(COMBO_BIN)
	$(BASE_JTAG_CMD) \
				-c "flash erase_address 0x08000000 32768" \
				-c "reset halt" \
				-c "flash write_image erase $(COMBO_BIN) 0x08000000" \
				-c "verify_image $(COMBO_BIN) 0x08000000" \
				$(POSTLUDE)

upload_jtag_erase_first_sector: $(TARGET_BIN)
	$(BASE_JTAG_CMD) \
				-c "flash erase_address 0x08000000 0x20000" \
				-c "reset halt" \
				-c "flash write_image erase $(TARGET_BIN) 0x08000000" \
				-c "verify_image $(TARGET_BIN) 0x08000000" \
				$(POSTLUDE)

upload_combo_jtag_erase_first_sector: $(COMBO_BIN)
	$(BASE_JTAG_CMD) \
				-c "flash erase_address 0x08000000 0x20000" \
				-c "reset halt" \
				-c "flash write_image erase $(COMBO_BIN) 0x08000000" \
				-c "verify_image $(COMBO_BIN) 0x08000000" \
				$(POSTLUDE)

# Phony target for killing the system FTDI driver on OS X.
kill_ftdi:
	sudo kextunload -b com.FTDI.driver.FTDIUSBSerialDriver

reload_ftdi:
	sudo kextload -b com.FTDI.driver.FTDIUSBSerialDriver

.PHONY: upload

.PHONY: image_serial
.PHONY: upload_serial upload_serial_no_erase
.PHONY: upload_combo_serial upload_combo_serial_no_erase

.PHONY: image_jtag image_jtag_1M
.PHONY: erase_jtag
.PHONY: upload_jtag upload_jtag_no_erase upload_jtag_erase_first
.PHONY: upload_combo_jtag upload_combo_jtag_no_erase upload_combo_jtag_erase_first

.PHONY: kill_ftdi reload_ftdi
.PHONY: debug_client debug_server
