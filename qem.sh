#!/bin/bash

#./build/zephyr/zephyr.exe -trace-file=data/channel0_0 && \
west build --pristine -b qemu_x86 $HOME/Projects/zephyr-scheduling -- -DCONF_FILE=prj.conf && \
rm -f "$HOME"/Projects/zephyr-scheduling/data/* && mkdir -p "$HOME"/Projects/zephyr-scheduling/data && \
cp "$ZEPHYR_BASE"/subsys/tracing/ctf/tsdl/metadata "$HOME"/Projects/zephyr-scheduling/data && \
west build -t run && \
cp "$HOME"/Projects/zephyr-scheduling/build/channel0_0 "$HOME"/Projects/zephyr-scheduling/data && \
babeltrace2 data/ | less
