Summary of XBee Operation
=====================

Key Requirements
----------------

The goal of the XBee implementation is to ensure a robust, error tolerant communication between the rotator module and the shutter module. The modules must be able to easily find each other and communicate, and to verify that communication has been successful, while avoiding any possibility of interfering with neighbouring installations.

By default, XBees are configured in a "transparent" mode where any data sent to one is relayed to all others on the network. This mode doesn't really address any of the key aims, except that it works simply and without much configuration. In particular, it is likely to cause problems with multiple installations.

Proposed Method of Operation
----------------------------

We will assume that all of the fevices in one dome are on the same PAN (Personal Area Network) and that devices in different domes will be on different PANs.

The rotator module will assume the role of Coordinator, default PAN ID 0x6FBF, short address 0xD0, with automatic PAN ID reassignment. In this mode, the module will start up and scan for existing PANs, and will choose a PAN that does not conflict.

The Shutter module will assume the role of an Endpoint Device, default PAN ID 0x6FBF, short address 0xD1 and will be configured to associate to the Coordinator with the strongest signal.

Once a shutter module has associated successfully, the Rotator module will be reconfigured to disallow further associations. This should mean that, as long as rotator and shutter modules are powered-on in pairs, the shutter should always associate with the correct rotator.

The XBees will be configured to operate in API mode (rather than transparent mode), so that the PAN can be monitored and communications will be fault-tolerant.

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