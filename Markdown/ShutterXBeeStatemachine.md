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
WaitCommandMode --> WaitForAssociation: "OK" received\nsend XBEE_SHUTTER_INIT_STRING
WaitCommandMode --> Startup: timeout

WaitForAssociation: OnEnter: set timeout to XBEE_REMOTE_HANDSHAKE_TIMEOUT
WaitForAssociation: OnEnter: Listen in API mode
WaitForAssociation --> Associated: Modem status event\n"Associated" received
WaitForAssociation --> Startup: timeout

Associated: OnEnter: set timeout = XBEE_REMOTE_HANDSHAKE_TIMEOUT
Associated: OnEnter: send Hello message to remote XBee
Associated --> Startup: timeout
Associated --> Online: Hello acknowledgement frame received\nset destination address from ack message

Online: OnEnter: set timeout = XBEE_NO_ACTIVITY_TIMEOUT
Online --> Associated: timeout

@enduml
```
