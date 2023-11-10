TOOLCHAIN_PATH=/opt/riscv64-unknown-elf-gcc-8.3.0-2020.04.1-x86_64-linux-ubuntu14/bin/
FLASH_PATH=../flasher/ft2232h
TOOLCHAIN_PREFIX=riscv64-unknown-elf

flash_tigard: ${PATTERN}.bin
	PYTHONPATH=$(FLASH_PATH) python3 $(FLASH_PATH)/caravelflash/flash_util.py  --write $<

%.elf: %.c ../deps/sections.lds ../deps/crt0_vex.S
	$(TOOLCHAIN_PATH)$(TOOLCHAIN_PREFIX)-gcc -I../deps -I../deps/generated/ -O0 -mabi=ilp32 -march=rv32i -D__vexriscv__ -Wl,-Bstatic,-T,../deps/sections.lds,--strip-debug -ffreestanding -nostdlib -o $@ ../deps/crt0_vex.S ../deps/isr.c ../deps/stub.c $<
	${TOOLCHAIN_PATH}$(TOOLCHAIN_PREFIX)-objdump -D $@ > ${PATTERN}.lst

%.hex: %.elf
	$(TOOLCHAIN_PATH)$(TOOLCHAIN_PREFIX)-objcopy -O verilog $< $@
	sed -i 's/@1000/@0000/g' $@

%.bin: %.elf
	$(TOOLCHAIN_PATH)$(TOOLCHAIN_PREFIX)-objcopy -O binary $< $@

# ---- Clean ----

clean:
	rm -f *.elf *.hex *.bin *.lst

monitor:
	pio device monitor -p /dev/serial/by-id/usb-Arduino_Nano_33_BLE_3C48BB3E0BD44A03-if00

.PHONY: clean hex flash
