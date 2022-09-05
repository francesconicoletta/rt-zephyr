.PHONY: metadata buildqemu builddue buildvega run flash debugvega trace debugqemu traceqemu clean

metadata:
	rm -f data/* && mkdir -p data/ && cp ${ZEPHYR_BASE}/subsys/tracing/ctf/tsdl/metadata data/metadata

buildqemu:
	west build --pristine -b qemu_x86_64 . -- -DCONF_FILE=prj_qemu.conf

buildqemur:
	west build --pristine -b qemu_riscv64 . -- -DCONF_FILE=prj_qemu.conf

builddue:
	west build --pristine -b arduino_due . -- -DCONF_FILE=prj_uart_due.conf

buildvega:
	ZEPHYR_TOOLCHAIN_VARIANT=cross-compile
	CROSS_COMPILE=~/riscv32-unknown-elf-gcc/bin/riscv32-unknown-elf-
	west build --pristine -b rv32m1_vega_ri5cy . -- -DCMAKE_REQUIRED_FLAGS=-Wl,-dT=/dev/null -DCONF_FILE=prj_uart_vega.conf

run:
	west build -t run

flashvega:
	cd build; west flash --openocd=${HOME}/rv32m1-openocd; cd -

debugvega:
	west debug --openocd=${HOME}/rv32m1-openocd

trace:
	rm -f data/channel0_0
	cd data; python3 ${ZEPHYR_BASE}/scripts/tracing/trace_capture_uart.py -d /dev/cu.usbmodem1101 -b 115200; cd -

debugqemu:
	cd build && ninja debugserver && cd ..

traceqemu:
	cp build/channel0_0 data/channel0_0
	babeltrace2 data > data/trace && less data/trace

clean:
	rm -rf build
