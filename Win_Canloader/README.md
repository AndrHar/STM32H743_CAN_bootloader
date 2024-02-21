# CANLoader

This windows application is used to download user program to microcontroller.

It works and tested with **sysWORKXX USB CAN module1** of [SYS TEC electronic](https://systec-electronic.com)

Start `CANLoader.exe`

Input board-id number. Click button `Connect` while bootloader program running. Ping with 1 sec period of CAN-message 0x551 starts.

Click `Open` to select file in `.bin` format. Then click `Update` to download user program.
