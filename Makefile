#
# Configure your RiscV toolchain path and prefix
#
TOOLCHAIN_PATH		?= /opt/riscv64-unknown-elf-gcc-8.3.0-2020.04.1-x86_64-linux-ubuntu14/bin/
TOOLCHAIN_PREFIX	?= riscv64-unknown-elf-

# This path appears to be outside this project tree, so maybe instructions here to download this tool ?
FLASH_PATH		?= ../flasher/ft2232h


ifeq ($(MAKELEVEL),0)
 ifeq (deps,$(wildcard deps))
# Top level only
ROOTDIR			= $(CURDIR)
SUBMODULES		= necklace
 else
# Subdir module is toplevel
ROOTDIR			= $(PWD)/..
 endif
export ROOTDIR
unexport SUBMODULES
endif


GENDIR  = -I$(ROOTDIR)/deps/generated
INCDIRS = -I$(ROOTDIR)/deps $(GENDIR)

GCC	= $(TOOLCHAIN_PATH)$(TOOLCHAIN_PREFIX)gcc
OBJDUMP	= $(TOOLCHAIN_PATH)$(TOOLCHAIN_PREFIX)objdump
OBJCOPY = $(TOOLCHAIN_PATH)$(TOOLCHAIN_PREFIX)objcopy
GREP    = grep



# Newer GCC/binutils 2.36+ may require use of -march=rv32i_zicsr of inline __asm__ is used with CSRs
GCC_COMPILER_SUPPORTS_ZICSR = $(shell $(GCC) -dumpspecs 2>/dev/null | $(GREP) -i -E "^march" | $(GREP) -i -c -E "_zicsr")
ifeq ($(GCC_COMPILER_SUPPORTS_ZICSR),0)
 CFLAGS_MARCH = -march=rv32i
else
 CFLAGS_MARCH = -march=rv32i_zicsr
endif
# VexRiscV on Caravel
CFLAGS_ARCH	=	-mabi=ilp32 $(CFLAGS_MARCH) -D__vexriscv__ -mstrict-align

#
#CFLAGS_CODEGEN	= -O0
# For caravel production reduce code size and amount of stack usage around function calls and
#   set RiscV ABI minimum frame size (16 bytes) due to the limited RAM caravel has 1.5KiB.
CFLAGS_CODEGEN	=	-O0 -fomit-frame-pointer -mpreferred-stack-boundary=4

CFLAGS		=	-Wall

SRC_FILES	=	crt0_vex.S \
			isr.c \
			stub.c

SRCS		=	$(addprefix $(ROOTDIR)/deps/,$(SRC_FILES))

all: submodule-all


submodule-all:
	@for submodule in $(SUBMODULES); do \
		$(MAKE) -C $${submodule} submodule-all; \
	done


flash_tigard: $(PATTERN).bin
	@if [ ! -e "$(FLASH_PATH)" ]; then \
		echo "###" 1>&2; \
		echo "### WARNING: FLASH OPERATION SKIPPED"; 1>&2 \
		echo "###" 1>&2; \
		echo "###          FLASH_PATH=${FLASH_PATH} does not exist (please install flasher)" 1>&2; \
		echo "###" 1>&2; \
		exit 0; \
	else \
		PYTHONPATH=$(FLASH_PATH) python3 $(FLASH_PATH)/caravelflash/flash_util.py  --write $<; \
	fi

%.elf: %.c $(ROOTDIR)/deps/sections.lds $(SRCS)
	$(GCC) $(INCDIRS) $(CFLAGS_CODEGEN) $(CFLAGS_ARCH) $(CFLAGS) -Wl,-Bstatic,-T,$(ROOTDIR)/deps/sections.lds,--strip-debug -ffreestanding -nostdlib -o $@ $(SRCS) $<
	$(OBJDUMP) -D $@ > $(PATTERN).lst

%.hex: %.elf
	$(OBJCOPY) -O verilog $< $@
	sed -i 's/@1000/@0000/g' $@

%.bin: %.elf
	$(OBJCOPY) -O binary $< $@

# ---- Clean ----

clean:
	@for submodule in $(SUBMODULES); do \
		$(MAKE) -C $${submodule} clean; \
	done
	rm -f *.elf *.hex *.bin *.lst

monitor:
	pio device monitor -p /dev/serial/by-id/usb-Arduino_Nano_33_BLE_3C48BB3E0BD44A03-if00

.PHONY: clean all submodule-all monitor hex flash
