# Bootloader_CAN_H743

Bootloader via CAN-bus for STM32H743



## Flash memory usage

This bootloader works only with BANK1 of Flash memory.

`Sector0 (0x8000000)` - program of bootloader. After reset microcontroller always jumps to this address.  
`Sector1 (0x8020000)` - user config data that can be erased and written by user program.  
`Sector2 (0x8040000)` - user program. Can occupy all remaining sectors in BANK1.

## Bootloader

After start running, bootloader waits CAN-msg with determined ID for some delay (default 2 sec). This delay can be set in user config data (Sector1).

After delay bootloader gives control to user program.

By default, bootloader communicates with two CAN-ids: 0x560 and 0x570. If there are several boards on same CAN-bus, each board should identificate itself by writing its board-id to user config data (Sector1). Bootloader reads board-id and adds this value to CAN-id expecting for program loading. For example, board-id is 2, so bootloader will wait CAN-msg with can-id 0x562 and 0x572.

Messages with can-id 0x56x are command messages for bootloader.

Messages with can-id 0x57x are messages with program bytes.

Baudrate of CAN-bus: 500 kbps.

## User config data

`0x8020000` - (uint32) config header: 0x0123fedc

`0x8020004` - (uint32) board-id: from 0 to 0xF

`0x8020008` - (uint32) delay before jump from bootloader to user program.

Units - milliseconds.
