This library provides a simple wrapper class around the predefined Serial_ object.
Write behaviour is slightly modified, so that no writes will occur unless they
are non-blocking (i.e. there is enough room in the write buffer).

This solves a problem where disconnecting the host (PuTTY or Windows .NET SerialPort instance)
leaves the Arduino thinking a host is still connected and serial output then begins to block,
causing multi-second delays each time around the Arduino main loop.

This is probably specific to Arduino Leonardo. It has not been tested on any other platform.