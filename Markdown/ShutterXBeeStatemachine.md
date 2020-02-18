``` puml
@startuml
hide empty description
title Shutter XBee State Machine

[*] -> Startup

Startup: OnEnter: set timeout = XBEE_BOOT_TIME (5s)
Startup --> WaitCommandMode: timeout

WaitCommandMode: OnEnter: Listen in AT Command Mode
WaitCommandMode: OnEnter: destination address = COORDINATOR (0xFFFF)
WaitCommandMode: OnEnter: send "+++" to XBee
WaitCommandMode: OnEnter: set timeout to 3 x guard time
WaitCommandMode --> Configure: "OK" received
WaitCommandMode --> WaitCommandMode: timeout

Configure: OnEnter: send next AT Command
Configure --> WaitCommandMode: timeout
Configure --> Configure: __"OK" received & more AT commands__\nsend next AT command\nreset timeout
Configure --> WaitForAssociation: "OK" received & No more AT commands

WaitForAssociation: OnEnter: set timeout to XBEE_REMOTE_HANDSHAKE_TIMEOUT
WaitForAssociation: OnEnter: Listen in API mode
WaitForAssociation --> Associated: "Associated" Modem status received
WaitForAssociation --> WaitCommandMode: timeout

Associated: OnEnter: start Hello Response timer
Associated: OnEnter: send Hello message to remote XBee
Associated --> WaitCommandMode: Hello Response timeout
Associated --> Online: __Hello response received__\nset destination address from ack message

Online: OnEnter: start Heartbeat timer
Online: OnEnter: stop Hello Response timer
Online --> WaitCommandMode: Hello response timeout
Online --> Online: __heartbeat interval timeout__\nSend Hello\nstart Hello timer
Online --> Online: __Hello response__\nStop hello response timer\nstart heartbeat interval timer

@enduml
```
