.PHONY: builduartqemu buildramqemu buildvega run flash debugvega debugqemu traceuartqemu clean


builduartqemu:
	west build --pristine -b qemu_x86_64 . -- -DCONF_FILE=prj_uart_qemu.conf
	rm -f data/* && mkdir -p data/ && cp $${ZEPHYR_BASE}/subsys/tracing/ctf/tsdl/metadata data/metadata

buildramqemu:
	west build --pristine -b qemu_x86_64 . -- -DCONF_FILE=prj_ram_qemu.conf
	rm -f data/* && mkdir -p data/ && cp $${ZEPHYR_BASE}/subsys/tracing/ctf/tsdl/metadata data/metadata

buildvega:
	ZEPHYR_TOOLCHAIN_VARIANT=cross-compile
	CROSS_COMPILE=~/riscv32-unknown-elf-gcc/bin/riscv32-unknown-elf-
	west build --pristine -b rv32m1_vega_ri5cy . -- -DCMAKE_REQUIRED_FLAGS=-Wl,-dT=/dev/null -DCONF_FILE=prj.conf
	rm -f data/* && mkdir -p data/ && cp $${ZEPHYR_BASE}/subsys/tracing/ctf/tsdl/metadata data/metadata

run:
	west build -t run

flash:
	cd build; west flash --openocd=${HOME}/rv32m1-openocd

debugvega:
	west debug --openocd=${HOME}/rv32m1-openocd

debugqemu:
	cd build; ninja debugserver

traceuartqemu:
	cp build/channel0_0 data/channel0_0
	babeltrace2 data > data/trace && less data/trace

clean:
	rm -rf build
