# STM32H743_CAN_bootloader
Bootloader via CAN-bus for STM32H743

**`Bootloader`** - program of bootloader via CAN-bus without using additional pins on MC  
**`UserProgExample`** - example of user program with issues that should be implemented for bootloader   
**`Win_Canloader`** - windows program for loading user program ( in .bin format) to MC via CAN-bus   

## Flash memory usage   
This bootloader works only with BANK1 of Flash memory. 
`Sector0 (0x8000000)` - program of bootloader. After reset microcontroller always jumps to this address.  
`Sector1 (0x8020000)` - user config data that can be erased and written by user program. 
`Sector2 (0x8040000)` - user program. Can occupy all remaining sectors in BANK1.
  


