export OPENOCD=/usr/local/Cellar/open-ocd/0.8.0/bin/openocd
export BRDPATH=/usr/local/Cellar/open-ocd/0.8.0/share/openocd/scripts/board/ek-tm4c123gxl.cfg


arm-none-eabi-gdb -ex 'target extended-remote | $OPENOCD -f $BRDPATH -c "gdb_port pipe; log_output openocd.log"; monitor reset; monitor halt' build/a.out

# http://processors.wiki.ti.com/index.php/Stellaris_Launchpad_with_OpenOCD_and_Linux
# (gdb) monitor reset halt
# (gdb) load
# (gdb) monitor reset init