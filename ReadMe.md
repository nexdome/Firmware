# NexDome firmware for Arduino

This firmware was professionally developed for \[NexDome\] by [Tigra
Astronomy](http://tigra-astronomy.com/ "Tigra Astronomy home page"). The code is open-source and is
licensed under the [Tigra Astronomy MIT
license](http://tigra.mit-license.org "MIT license details").

## Development Environment

The Arduino IDE is rather limiting for serious software development. This project was developed
using Microsoft Visual Studio, with the VisualMicro Arduino extension, which makes it easy to have
*local libraries* and *shared code projects* and reference them in exactly the same way you would in
a C\# project, without having to install the library or move it to some aesoteric location. We have
also included a firmware updater utility written in C\# within the same solution, which is not
possible using the Arduino IDE.

Therefore, to compile this project, you will need Microsoft Visual Studio 2017 or later, and the
VisualMicro Arduino extension. Both products have free versions and can be downloaded from:

Microsoft Visual Studio 2017 Community Edition <https://visualstudio.microsoft.com/> VisualMicro
Arduino Extension <https://www.visualmicro.com/>

Visual Studio Code is another great option for developing Arduino sketches, although it is not clear
how you would deal with the local libraries and shared code project that we have used.

## Design Philosophy

Within the limitations of the Arduino Uno (a resource-constrained embedded system), we have tried to
apply the SOLID principles of object oriented design:

S - Single responsibility principle; each class should have only one responsibility\
O - Open/Closed principle; open for extension, closed for modification\
L - Liskov substitution principle; superclasses and subclasses classes should be interchangeable\
I - Interface segregation principle; no client should be forced to depend on methods it does not
use\
D - Dependency inversion principle: depend on abstractions not details.\

Well-factored object oriented code that adheres to the SOLID principles should be loosely coupled,
highly cohesive, testable and have low viscosity for future maintenance.

Due to resource constraints imposed by the target hardware, we have had to refactor some of the code
into a more procedural style in order to save memory.

## Memory Management

Dynamic memory allocations have been agressively avoided. As a resource-constrained embedded system
with just 2Kb of data memory, there is not much space available for a heap and we can't tolerate
"Out Of Memory" errors at runtime. The system must be stable for days, months or even years at a
time so the memory management strategy must be frugal, deterministic and stable.

Our solution to this is to statically pre-allocate as many objects as possible once, in global
scope, then never delete them. The top level `.ino` file contains these allocations either as
statically initialized global variables or in the setup() method and this essentially forms the
Composition Root for the system. The main exception to this rule is in the XBee state machine
classes, which are created and destroyed as necessary - however the system will usually remain in
the `Online` state so there will be a stable configuration most of the time.

Since we can assume that most objects are never freed, there is little to be gained from the use of
smart pointers and we have chosen to avoid the overhead and use "raw" pointers where necessary.
Where possible, we have used references (`&`) and pass-by-reference, and these references typically
resolve to one of the objects defined in global scope in the main sketch file.

We make use of the C++ standard template library defined in namespace `std::` and provided by the
library `ArduinoSTL`. This is perhaps unusual in the context of an Arduino sketch, for reasons we
don't really understand. Perhaps many Arduino programmers are not familiar with the full
capabilities fo th C++ language (a situation that the Arduino IDE and approach to programming seems
to propagate). Perhaps it is because many people believe that these classes require more memory and
the Arduino is somehow "not powerful enough" to use them. In fact, these classes are highly
optimised and do not incur much, if any, overhead. What they do provide is algorithms and data
structures that have been highly polished over many years, and a degree of readability and
modularity to the code. We think that the trade-off of a slight memory and performance overhead is
more than worth the extra readability and maintainability that it gives the code. We make good use
of the `std::vector<T>` class to manage collections and in particular for sending, receiving and
manipulating XBee API data frames. Typically we construct vectors with a reserved capacity, so that
re-allocation of memory is not necessary, and we never free these structures so they remain
permanently allocated (note: passing by reference does not copy the data). We also make use of
`std::string`.

## Motor Control

The stepper motors have a Direction/Step/Enable interface and are driven by generating a square wave
onto the Step pin while the Enable pin is asserted. Direction of movement is controlled, self
evidently, by the Direction pin. Negating the Enable pin removes energy from the motor coils and
releases holding torque.

The stepping process is time sensitive and demands high throughput speeds and consistent step pulse
train generation for smooth motor operation. Tigra Astronomy has developed a control library for
stepper motors that ensures that steps can be delivered at high speed and with consistent timing,
regardless of what is happening in the Arduino main loop. The popular AccelStepper library does not
achieve this because it relies on being called within the Arduino main loop and mingles all of the
acceleration computations in with the step generation.

Here we see a clear advantage in paying attention to the Single Responsibility Principle. Tigra's
stepper driver logically divides stepping into two parts:

### Step Generator

A step generator (implements: `IStepGenerator`) is responsible for generating a pulse train where
each rising edge causes the motor to make one step. The step generator is responsible for timing
(step speed) and has no concept of position, direction or the type of steps (whole steps,
microsteps, etc.)

We have provided a single implementation, `CounterTimer1StepGenerator`, which uses the Timer 1 block
of the AVR processor to generate accurately timed pulses with 50% duty cycle. The timer is
configured to generate interrupts using the `OCR1A` compare register. The timing source is the
undivided system clock, which allows for a theoretical stepping bandwidth of about 244 steps/second
up to 16,000,000 steps/second.

### Step Sequencer

The step sequencer (implements: `IStepSequencer`) carries the responsibility of writing the correct
hardware signals to the motor driver and keeping track of the step position.

Our `Motor` class provides the `IStepSequencer` implementation and allows for acceleration and
deceleration. `Motor` also keeps track of the current step position and enforces limits of travel on
the motors.

### Acceleration

The `Motor` class implements acceleration and deceleration based on the equation of uniform
acceleration, v = u + at. This reduces the risk of stalling, especially when moving heavy loads.

The `ComputeAcceleratedVelocity` method is called once per Arduino main loop to recompute the motor
velocity and acceleration curves. We have found that acceleration can be treated as a lower priority
task and does not need to be computed for every step. This allows us to schedule it in the main loop
with other tasks and maintain a clean separation of concerns in the code.

The "Ramp Time" (the time taken to accelerate from rest to maximum speed) is configurable by the
user. Ramp time is specified in milliseconds. 250 to 500 milliseconds is usually sufficient for
moderate loads but for more massive loads this can be increased.

### Speed and Power Considerations

The NexDome controllers us a 15:1 gearing arrangement to drive both the rotator and shutter
mechanisms. This arrangement provides a naturally stable system at rest which means that holding
torque in the stepper motors is unnecessary. Therefore, the step drivers are disabled once motion
has ceased. This reduces power consuption and keeps the motors and step drivers cool when not
actively driving the mechanism.

While the firmware is capable of very high step rates, there is a trade-off between maximum stepping
speed, available torque and power consumption. The firmware should provide commands for adjusting
maximum step rate (`insert-command`) and acceleration ramp time (`insert-command`) so that the end
user can manage this speed/torque/power tradeoff.

The Tigra stepping algorithm is capable of driving steps at a theoretical rate of at least 50,000
steps per second on an Arduino AVR family CPU (UNO, Leonardo). In practice the motors and driver
modules used will dictate the maximum possible speed. The motors and modules used in the NexDome
will perform well up to at least 8,000 microsteps, 1000 whole steps or about 5 revolutions per
second when used with moderate loads. The motors are 1.8° per step, 200 steps per revolution. The
driver modules are configured for 8 microsteps per step.

Beyond that, the motors may be unable to provide sufficient torque and may stall. Therefore we do
not advise increasing the maximum step speed much beyond the default 8,000 microsteps per second.

For larger loads with higher inertia, it may be desirable to lower the maximum step rate
(`insert-command`) and increase the acceleration ramp time (`insert-command`).

## Command Processor

Command processing is handled by the `CommandProcessor` class. This is actually a port of a more
object-oriented library (also developed by Tigra Astronomy) but is one area where we found it
necessary to favour smaller resource usage over code structure. Therefore our object oriented design
has been collapsed into more procedural code in this implementation.

When a well formed command is received from the serial communications channel it is passed to
`DispatchCommand()` which passes it on to `CommandProcessor::HandleCommand()`.

Each command verb has its own handler method. `CommandProcessor::HandleCommand()` decides which
handler method to call based on the command verb.

All command handlers return a `Response` structure, which contains the text (if any) to be
transmitted back to the client application.

## Command Protocol

### Command Grammar

Commands have the form: <kbd>@</kbd> `Verb` `Device`<kbd>,</kbd> `Parameter`
<kbd>`<CR>`</kbd><kbd>`<LF>`</kbd>.

-   <kbd>@</kbd> is a literal character that marks the start of a new command and clears the receive
    buffer. Use of the <kbd>@</kbd> initiator is optional, but strongly recommended when generating
    commands programmatically.
-   `Verb` is the command verb, which normally consists of two characters. Single character verbs
    are also possible but in this case the entire command is a single character.
-   `Device` is the target device for the command, generally `R` (rotator) or `S` (shutter). Where
    no device address is given, a default value may be assumed **\[to be defined\]**
-   <kbd>,</kbd> is a literal character that separates the device ID from the parameter.
-   `Parameter` is a positive integer. If omitted, zero is assumed.
-   <kbd>`<CR>`</kbd><kbd>`<LF>`</kbd> is the command terminator and submits the command to the
    dispatcher. Only one is required. If both are present then they can be in any order.

<example>
Example: `@GAR,100`.
</example>

If the parameter field is not required for a command, then it can be omitted or, if specified, it
will be ignored.

### Errors

Any unrecognised or invalid command responds with the text `Err`.

### Command Protocol Details

#### General Command Syntax ####

<kbd>@</kbd>CCT,Param<kbd>CR</kbd><kbd>LF</kbd>
where:
 - <kbd>@</kbd> (literal character), command initiator, optional but strongly encouraged when being generated programmatically. It may be omitted for convenience when typing in a terminal emulator.
 - `CC` is a two letter command code
 - `T` is the target device, one of: `R` (rotator) or `S` (shutter).
 - `,Param` optional parameter, command specific. May be omitted if not needed.
 - <kbd>CR</kbd><kbd>LF</kbd> line terminator. May consist of Carriage Return (ASCII 0x0D), Line feed (ASCII 0x0A), or both, in any order.

 Examples:
    @FVR<kbd>CR</kbd> - read rotator firmware version
    @AWS,1000<kbd>CR</kbd><kbd>LF</kbd> - write stepper ramp time of 1000 milliseconds to the shutter

#### Responses ####

Unless otherwise stated in the table below, all commands respond by echoing their command code and target device, in the format:

<kbd>:</kbd>CCT<kbd>#</kbd>

The parameter value (if any) is not echoed. For example, the response to `@GAR,1000` is `:GAR#`. Receipt of this echo response indicates that the command is valid and was successfully received.

Commands that return a value include the value immediately after the target and before the terminating `#`. Example: `:VRR10000#`

Any command that cannot be processed for any reason will respond with `:Err#`

**Other status and debug output may be generated at any time, not necessarily in response to any command.**

#### Commands ####


Cmd | Targets | Parameter | Response   | Example    | Description
----|---------|-----------|------------|------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
FR  | RS      | none      | :FRm.n#    | @FRR       | Read firmware version
GA  | R       | ddd       |            | @GAR,180   | Goto Azimuth (param: integer degrees)
SW  | RS      | none      |            |            |
PR  | RS      | none      | :PRt-dddd# | @PRR       | Position Read - get current step position in whole steps (signed integer)
PW  | RS      | ±dddd     |            | @PWR,-1000 | Position Write (sync) - set step position
RR  | RS      | none      | :RRtdddd   | @RRS       | Range Read - read the range of travel in whole steps.
VR | RS | none | :VRtddddd# | @VRR | Velocity read - motor speed in whole steps per second
VW | RS | ddddd | :VWt# | @VWS,10000 | Velocity Write - motor speed in whole steps per second

		if (command.Verb == "PR") return HandlePR(command);	// Position read
		if (command.Verb == "PW") return HandlePW(command);	// Position write (sync)
		if (command.Verb == "RR") return HandleRR(command);	// Range Read (get limit of travel)
		if (command.Verb == "RW") return HandleRW(command);	// Range Write (set limit of travel)
		if (command.Verb == "VR") return HandleVR(command);	// Read maximum motor speed
		if (command.Verb == "VW") return HandleVW(command);	// Read maximum motor speed
		if (command.Verb == "ZD") return HandleZD(command);	// Reset to factory settings (load defaults).
		if (command.Verb == "ZR") return HandleZR(command);	// Load settings from persistent storage
		if (command.Verb == "ZW") return HandleZW(command);	// Write settings to persistent storage



Note that omitting all of the optional parts of each command gives a more convenient syntax when
entering commands manually in a terminal emulator. The table above gives the shortest possible form
of each command. However, when commands are generated programmatically, it is recommended that the
initial `@` character and the destination device address are always included.

## Arduino Libraries Used

-   ArduinoSTL - standard template library (install from library manager)
-   eeprom - for reading and writing the nonvolatile storage (install from library manager)
-   AdvancedStepper - Tigra Astronomy's advanced stepper motor control (included/local)
-   XBeeAPI - NexDome specific, used for sending, receiving and parsing XBee API data frames
    (included/local)
-   XBeeStateMachine - NexDome specific, used to control the sate of XBee communications
    (included/local)
-   Timer - Tigra Astronomy's timer utility, used primarily for monitoring timeouts.
    (included/local)

## XBee Communications

A major part of the firmware is concerned with handling communications to and from the XBee wireless
communications module.

### Key Requirements

The goal of the XBee implementation is to ensure a robust, error tolerant communication between the
rotator module and the shutter module. The modules must be able to easily find each other and
communicate, and to verify that communication has been successful, while avoiding any possibility of
interfering with neighbouring installations.

By default, XBees are configured in a "transparent" mode where any data sent to one is relayed to
all others on the network. This mode doesn't really address any of the key aims, except that it
works simply and without much configuration. In particular, it is likely to cause problems with
multiple installations.

## Proposed Method of Operation

We will assume that all of the devices in one dome are on the same PAN (Personal Area Network) and
that devices in different domes will be on different PANs.

The rotator module will assume the role of Coordinator, default PAN ID 0x6FBF, short address 0xD0,
with automatic PAN ID reassignment. In this mode, the module will start up and scan for existing
PANs, and will choose a PAN that does not conflict.

The Shutter module will assume the role of an Endpoint Device, default PAN ID 0x6FBF, short address
0xD1 and will be configured to associate to the Coordinator with the strongest signal.

Once a shutter module has associated successfully, the Rotator module will be reconfigured to
disallow further associations. This should mean that, as long as rotator and shutter modules are
powered-on in pairs, the shutter should always associate with the correct rotator.

The XBees will be configured to operate in API mode 2 (rather than transparent mode), so that the
PAN can be monitored and communications will be fault-tolerant.

## XBee State Machine

The XBee communications is managed by a state machine that controls startup, configuration and
ongoing monitoring of the connection to the remote device.

The framework of the state machine is common to both rotator and shutter, so it is contained in a
shared library called `XBeeStateMachine`. The operation of the two modules differs as the Rotator
module must act as coordinator and the Shutter module as an endpoint. The states are therefore
implemented directly in each sketch.

## Rotator XBee State Machine

![Rotator XBee State Machine](/assets/Rotator-XBee-State-Machine.PNG)

## Shutter XBee State Machine

![Shutter XBee State Machine](/assets/Shutter-XBee-State-Machine.PNG)

## Revision Notes

### Release 2.0

First version by Tigra Astronomy.

NexDome: https://www.nexdome.com/ "NexDome home page"

