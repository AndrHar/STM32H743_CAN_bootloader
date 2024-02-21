# LED_blinking_NUCLEO-H743ZI

Simple example of user program with implemented bootloader specifics to be compliant with bootloader:

1. Assignation of `Vector Table Offset Register` should be done at the beginning of the user program.

2. Change memory areas address in `STM32H743ZITX_FLASH.ld`:

> FLASH (rx) : ORIGIN = 0x08040000, LENGTH = 1792K

In Keil: `Options for target -> Linker -> R/O Base`: 0x08040000

3. In STM32CubeIDE set checkbox for creating output binary file :

> Project-> Properties -> C/C++ Build Settings -> Tool Settings ->
> 
> MCU Post build outputs -> Convert to binary file (-O binary)

4. Implement features to change user config data.

Example was created in STM32CubeIDE.
