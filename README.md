# STM32H743_CAN_bootloader

Bootloader via CAN-bus for STM32H743

**`Bootloader`** - program of bootloader via CAN-bus without set/reset additional pins of microcontroller
**`UserProgExample`** - example of user program with specifics that should be implemented for bootloader compliance   
**`Win_Canloader`** - windows program for loading user program ( in `.bin` format) to microcontroller via CAN-bus   

## Flash memory usage

This bootloader works only with BANK1 of Flash memory. 

`Sector0 (0x8000000)` - program of bootloader. After reset microcontroller always jumps to this address.  
`Sector1 (0x8020000)` - user config data that can be erased and written by user program.   
`Sector2 (0x8040000)` - user program. Can occupy all remaining sectors in BANK1.   

## Bootloader

After start running, bootloader waits CAN-messages with determined ID for some delay (default 2 sec).  This delay can be set in user config data (Sector1). 

After delay bootloader gives control to user program.

By default, bootloader communicates with two CAN-IDs: 0x560 and 0x570. If there are several boards on same CAN-bus, each board should identificate itself by writing its board-id to user config data (Sector1). Bootloader reads board-id and adds this value to CAN-ID expecting for program loading. For example, board-id is 2, so bootloader will wait CAN-messages with CAN-ID 0x562 and 0x572.

Messages with CAN-ID 0x56x are command messages to bootloader.  

Messages with CAN-ID 0x57x are messages with program text bytes.

Baudrate of CAN-bus: 500 kbps.

## User config data

`0x8020000` - (uint32) config header: 0x0123fedc

`0x8020004` - (uint32) board-id: from 0 to 0xF

`0x8020008` - (uint32) delay before jump from bootloader to user program. 

Units - milliseconds.

## User program

User program should implement some specifics to be compliant with bootloader:

1. Assignation of `Vector Table Offset Register` should be done at the beginning  of the user program.

2. Change memory areas address in `STM32H743ZITX_FLASH.ld`:

> FLASH (rx) : ORIGIN = 0x08040000, LENGTH = 1792K

   In Keil: `Options for target -> Linker -> R/O Base`: 0x08040000

3. In STM32CubeIDE set checkbox for creating output binary file :

> Project-> Properties -> C/C++ Build Settings -> Tool Settings -> 
> 
> MCU Post build outputs -> Convert to binary file (-O binary)

4. Implement features to change user config data.

## CANLoader for Windows

This windows application is used to download user program to microcontroller.   

It works and tested with **sysWORKXX USB CAN module1** of [SYS TEC electronic](https://systec-electronic.com)

Start `CANLoader.exe`

Input board-id number. Click button  `Connect` while bootloader program running. Ping with 1 sec period of CAN-message 0x551 starts.  

Click `Open` to select file in `.bin` format. Then click `Update` to download user program.
