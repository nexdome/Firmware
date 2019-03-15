``` puml
@startuml
hide empty description
[*] -> Startup
Startup: OnEnter: set timeout = XBEE_BOOT_TIME (5s)
Startup: OnEnter: Listen in AT Command Mode
Startup --> WaitCommandMode: timeout
WaitCommandMode: OnEnter: send "+++" to XBee
WaitCommandMode: OnEnter: set timeout to 2 x guard time
WaitCommandMode --> DetectShutter: "OK" received\nsend XBEE_ROTATOR_INIT_STRING
WaitCommandMode --> Startup: timeout
DetectShutter: OnEnter: set timeout = DETECT_SHUTTER_TIMEOUT
DetectShutter: OnEnter: Listen in API mode
DetectShutter --> Startup: timeout
DetectShutter -> ShutterOnline: Hello message received\nset destination address from received frame
ShutterOnline: OnEnter: send Hello Ack Message
ShutterOnline: OnEnter: set timeout = NO_ACTIVITY_TIMEOUT
ShutterOnline --> Startup: timeout
@enduml
```
