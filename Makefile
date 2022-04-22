.PHONY: build run trace clean

build:
	west build --pristine -b qemu_x86 . -- -DCONF_FILE=prj.conf
	rm -f data/* && mkdir -p data/ && cp $${ZEPHYR_BASE}/subsys/tracing/ctf/tsdl/metadata data/metadata

run:
	west build -t run

trace:
	cp build/channel0_0 data/channel0_0
	babeltrace2 data > data/trace && less data/trace

clean:
	rm -rf build

all: build run
