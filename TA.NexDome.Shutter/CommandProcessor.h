#ifndef _COMMANDPROCESSOR_h
#define _COMMANDPROCESSOR_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <ArduinoSTL.h>
#include <AdvancedStepper.h>
#include <XBeeStateMachine.h>
#include "PersistentSettings.h"
#include "Command.h"
#include "Response.h"
#include "LimitSwitch.h"

class CommandProcessor
	{
	public:
		CommandProcessor(MicrosteppingMotor& motor, PersistentSettings& settings, XBeeStateMachine& machine, LimitSwitch& limits, BatteryMonitor& monitor);
		void HandleCommand(const Command& command); // sets the global static response
		static int32_t microstepsToSteps(int32_t microsteps);
		static int32_t stepsToMicrosteps(int32_t wholeSteps);
		int32_t getPositionInWholeSteps() const;
		void sendStatus() const;
		void sendOpenNotification() const;
		void sendCloseNotification() const;

	private:
		void sendToLocalAndRemote(const std::string& message) const;
		void HandleOP(const Command& command); // Open shutter
		void HandleCL(const Command& command); // Close shutter
		void HandleAW(const Command& command); // AW - Acceleration ramp time write
		void HandleAR(const Command& command) const;
		void HandleBR(const Command& command) const; // Read battery low volts threshold
		void HandleBW(const Command& command); // Read battery low volts threshold
		void HandleFR(const Command& command) const; // Firmware version read
		void HandlePR(const Command& command) const; // Step position read
		void HandlePW(const Command& command); // Step position write (sync)
		void HandleRR(const Command& command) const; // Range (limit of travel) read
		void HandleRW(const Command& command); // Range (limit of travel) write
		void HandleSR(const Command& command); // Request status report
		void HandleSW(const Command& command); // Stop write (motor emergency stop)
		void HandleVR(const Command& command) const; // Velocity [sic] read (motor maximum speed in microsteps/sec)
		void HandleVW(const Command& command); // Velocity [sic] write (microsteps/sec)
		void HandleX(const Command& command); // Movement status read
		void HandleZW(const Command& command); // EEPROM write (save settings)
		void HandleZR(const Command& command); // EEPROM read (load settings)
		void HandleZD(const Command& command); // Reset to factory settings (clears both EEPROM and working settings)
		MicrosteppingMotor& motor;
		PersistentSettings& settings;
		LimitSwitch& limitSwitches;
		XBeeStateMachine& machine;
		BatteryMonitor& battery;
	};

#endif
