export OPENOCD=/usr/local/Cellar/open-ocd/HEAD/bin/openocd
export BRDPATH=/usr/local/Cellar/open-ocd/HEAD/share/openocd/scripts/board/ek-tm4c123gxl.cfg

#$OPENOCD -f $BRDPATH &
#arm-none-eabi-gdb -ex 'target extended-remote :3333;'

# arm-none-eabi-gdb -ex 'target extended-remote | $OPENOCD -f $BRDPATH -c "gdb_port pipe; log_output openocd.log"; monitor reset halt; load; monitor reset init;' build/a.out

arm-none-eabi-gdb -ex 'target extended-remote | $OPENOCD -f $BRDPATH -c "gdb_port pipe; log_output openocd.log"; monitor reset halt; load;' build/a.out


# http://processors.wiki.ti.com/index.php/Stellaris_Launchpad_with_OpenOCD_and_Linux
# (gdb) monitor reset halt
# (gdb) load
# (gdb) monitor reset init