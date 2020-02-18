``` puml
@startuml
hide empty description
title Rotator XBee State Machine

[*] -> Startup
Startup: OnEnter: set timeout = XBEE_BOOT_TIME (5s)
Startup: OnEnter: Listen in AT Command Mode
Startup --> WaitCommandMode: timeout

WaitCommandMode: OnEnter: send "+++" to XBee
WaitCommandMode: OnEnter: set timeout to 2 x guard time
WaitCommandMode --> Configure: "OK" received
WaitCommandMode --> WaitCommandMode: timeout

Configure: OnEnter: send next AT Command
Configure --> WaitCommandMode: timeout
Configure --> Configure: __"OK" received & more AT commands__\nsend next AT command\nreset timeout
Configure --> DetectShutter: "OK" received & No more AT commands

DetectShutter: OnEnter: set timeout = DETECT_SHUTTER_TIMEOUT
DetectShutter: OnEnter: Listen in API mode
DetectShutter --> WaitCommandMode: timeout
DetectShutter -> ShutterOnline: __Hello message received__\nset destination address from received frame

ShutterOnline: OnEnter: send Hello Ack Message
ShutterOnline: OnEnter: set timeout = NO_ACTIVITY_TIMEOUT
ShutterOnline --> WaitCommandMode: timeout

@enduml
```
