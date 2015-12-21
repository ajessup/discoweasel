DiscoWeasel 
===========

Firmware for the DisoWeasel project, for the TM4C123GH6PM microcontroller. 

## Install tooling

This project requires the following to have been installed:

* [arm-cs-tools](https://github.com/jsnyder/arm-eabi-toolchain) for building and debugging
* [lm4tools](git://github.com/utzig/lm4tools.git) for flashing the microcontroller
* [openocd](http://openocd.sourceforge.net/) for debugging

## Install TivaWare driver and headers

The TivaWare package contains all of the header files and drivers for
Tiva parts. Grab the file *SW-TM4C-1.1.exe* from
[here](http://software-dl.ti.com/tiva-c/SW-TM4C/latest/index_FDS.html) and unzip it into a directory
then run `make` to build TivaWare.

    mkdir tivaware
    cd tivaware
    unzip SW-TM4C-1.1.exe

You can extract this anywhere, though a convienent place for it is under ./lib

## Building and flashing

1. Adjust the Makefile
    * Set OPENOCD\_PATH to the full path where `openocd` is installed
    * Set LM4\_TOOLS\_PATH to the full path where your `lm4tools` folder was extracted
    * Set ARM\_CS\_TOOLS\_PATH to the full path where you extracted the `arm-cs-tools` folder
    * Set TIVAWARE\_PATH to the full path to where you extracted and built
      TivaWare (eg: TIVAWARE_PATH = ./lib/tivaware)

3. Run `make`

4. The output files will be created in the 'build' folder. To flash it run `make up`.

## Debugging with gdb

These chips are supported in openocd HEAD ([credit to Karl Palsson](http://sourceforge.net/p/openocd/mailman/message/32139143/)). The [openocd website](http://openocd.sourceforge.net/) has instructions on how to install it.

With openocd installed, run gdb with this command:
```
arm-none-eabi-gdb -ex 'target extended-remote | openocd -f board/ek-tm4c1294xl.cfg -c "gdb_port pipe; log_output openocd.log"; monitor reset; monitor halt'
```
