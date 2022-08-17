.PHONY: buildqemu builddue buildvega run flash debugvega uartvega buartvega buartdue debugqemu traceqemu clean

buildqemu:
	west build --pristine -b qemu_riscv64 . -- -DCONF_FILE=prj_qemu.conf
	rm -f data/* && mkdir -p data/ && cp $${ZEPHYR_BASE}/subsys/tracing/ctf/tsdl/metadata data/metadata

builddue:
	west build --pristine -b arduino_due . -- -DCONF_FILE=prj_uart_due.conf
	rm -f data/* && mkdir -p data/ && cp $${ZEPHYR_BASE}/subsys/tracing/ctf/tsdl/metadata data/metadata

buildvega:
	ZEPHYR_TOOLCHAIN_VARIANT=cross-compile
	CROSS_COMPILE=~/riscv32-unknown-elf-gcc/bin/riscv32-unknown-elf-
	west build --pristine -b rv32m1_vega_ri5cy . -- -DCMAKE_REQUIRED_FLAGS=-Wl,-dT=/dev/null -DCONF_FILE=prj_uart_vega.conf
	rm -f data/* && mkdir -p data/ && cp $${ZEPHYR_BASE}/subsys/tracing/ctf/tsdl/metadata data/metadata

run:
	west build -t run

flash:
	cd build; west flash --openocd=${HOME}/rv32m1-openocd

debugvega:
	west debug --openocd=${HOME}/rv32m1-openocd

uartvega:
	rm -f data/* && mkdir -p data/ && cp $${ZEPHYR_BASE}/subsys/tracing/ctf/tsdl/metadata data/metadata
	(stty raw; cat > data/channel0_0) < /dev/ttyACM0

buartvega:
	rm -f data/* && mkdir -p data/ && cp $${ZEPHYR_BASE}/subsys/tracing/ctf/tsdl/metadata data/metadata
	(stty speed 115200 >/dev/null; cat > data/channel0_0) < /dev/cu.usbmodem1102

buartdue:
	rm -f data/* && mkdir -p data/ && cp $${ZEPHYR_BASE}/subsys/tracing/ctf/tsdl/metadata data/metadata
	(stty raw; cat > data/channel0_0) < /dev/cu.usbmodem1101

debugqemu:
	cd build && ninja debugserver && cd ..

traceqemu:
	cp build/channel0_0 data/channel0_0
	babeltrace2 data > data/trace && less data/trace

clean:
	rm -rf build
