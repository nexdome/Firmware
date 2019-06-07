``` puml
@startuml
hide empty description
title Shutter XBee State Machine

[*] -> Startup

Startup: OnEnter: set timeout = XBEE_BOOT_TIME (5s)
Startup: OnEnter: Listen in AT Command Mode
Startup: OnEnter: destination address = COORDINATOR (0xFFFF)
Startup --> WaitCommandMode: timeout

WaitCommandMode: OnEnter: send "+++" to XBee
WaitCommandMode: OnEnter: set timeout to 3 x guard time
WaitCommandMode --> Configure: "OK" received
WaitCommandMode --> Startup: timeout

Configure: OnEnter: send next AT Command
Configure --> Startup: timeout
Configure --> Configure: __"OK" received & more AT commands__\nsend next AT command\nreset timeout
Configure --> WaitForAssociation: "OK" received & No more AT commands

WaitForAssociation: OnEnter: set timeout to XBEE_REMOTE_HANDSHAKE_TIMEOUT
WaitForAssociation: OnEnter: Listen in API mode
WaitForAssociation --> Associated: "Associated" Modem status received
WaitForAssociation --> Startup: timeout

Associated: OnEnter: set timeout = XBEE_REMOTE_HANDSHAKE_TIMEOUT
Associated: OnEnter: send Hello message to remote XBee
Associated --> Startup: timeout
Associated --> Online: __Hello acknowledgement frame received__\nset destination address from ack message

Online: OnEnter: set timeout = XBEE_NO_ACTIVITY_TIMEOUT
Online --> Associated: timeout

@enduml
```
