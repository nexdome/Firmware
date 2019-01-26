Summary of XBee Operation
=========================

The XBees work as a Personal Area Network with a Coordinator. The rotator unit is the coordinator and the shutter unit is allowed to associate with the rotator. The settings used are:

Configuration
-------------

Comand mode is entered using the character sequence "+++". Then, the following AT commands are sent as a single string:

| Target  | AT Command string                                    |
| ------- | ---------------------------------------------------- |
| Rotator | "ATCE1,ID7734,CH0C,MY0,DH0,DLFFFF,AP0,SM0,WR,BD7,CN" |
| Shutter | "ATCE0,ID7734,CH0C,MY1,DH0,DL0,AP0,SM0,WR,BD7,CN"    |

--------------------------
| AT Command  | Description                                            |
| ----------- | ------------------------------------------------------ |
| **Rotator** |
| CE1         | Coordinator Mode Enabled                               |
| ID7734      | Set PAN ID to 0x7734                                   |
| CH0C        | Set Channel Number to 0x0C [valid range 0x0B to 0x1A]  |
| MY0         | Set local (my) address to 0                            |
| DH0         | Destination Address (high) = 0                         |
| DLFFFF      | Destination Address (low) = 0xFFFF (broadcast address) |
| AP0         | API mode disabled                                      |
| SM0         | Sleep Mode disabled                                    |
| WR          | Write settings to nonvolatile memory                   |
| BD7         | Interface data rate = 115,200 bits per second          |
| CN          | Exit command mode                                      |
| **Shutter** |
| CE0         | Coordinator Mode disabled                              |
| ID7734      | Set PAN ID to 0x7734                                   |
| CH0C        | Set Channel Number to 0x0C [valid range 0x0B to 0x1A]  |
| MY1         | Set local (my) address to 1                            |
| DH0         | Destination Address (high) = 0                         |
| DL0         | Destination Address (low) = 0                          |
| AP0         | API mode disabled                                      |
| SM0         | Sleep Mode disabled                                    |
| WR          | Write settings to nonvolatile memory                   |
| BD7         | Interface data rate = 115,200 bits per second          |
| CN          | Exit command mode                                      |

XBee States
-----------

| State Name  | Description                           |
| ----------- | ------------------------------------- |
| Startup     | XBee has not yet been configured      |
| Configuring | XBee configuration in progress        |
| Error       | XBee failed, no comms available       |
| Ready       | XBee comms established, ready for use |