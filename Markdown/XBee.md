## Summary of XBee Operation

###Key Requirements

The goal of the XBee implementation is to ensure a robust, error tolerant communication between the rotator module and the shutter module. The modules must be able to easily find each other and communicate, and to verify that communication has been successful, while avoiding any possibility of interfering with neighbouring installations.

From the factory, XBees are configured in a "transparent serial replacement" mode where any data sent to one is relayed to all others on the network. This mode doesn't really address any of the key aims, except that it works simply and without much configuration. In particular, it is likely to cause problems with multiple installations.

### Proposed Method of Operation

We will assume that all of the devices in one dome are on the same PAN (Personal Area Network) and that devices in different domes will be on different PANs.

The rotator module will assume the role of Coordinator, default PAN ID 0x6FBF, with automatic PAN ID and channel reassignment. In this mode, the module will start up and scan for existing PANs and channels, and will choose a PAN and channel that i snot currently in use.

The Shutter module will assume the role of an Endpoint Device, default PAN ID 0x6FBF and will be configured to associate to the Coordinator with the strongest signal.

The XBees will be configured to operate in API mode (rather than transparent mode), so that the PAN can be monitored and communications will be fault-tolerant. 64-bit addressing will be used.

As part of the configuration process, the XBees will be reset to factory defaults and then all required parameters explicitly set. No pre-configuration is required.

The XBee communications link will be managed by a state machine. If the communications link is lost for any reason, the state machine will automatically attempt to re-establish the link, rebooting and reconfiguring the XBees as needed. Whwnever there is a state change, this will be reported to the user via the serial port.