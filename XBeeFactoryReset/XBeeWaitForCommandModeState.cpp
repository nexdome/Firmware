#include "XBeeWaitForCommandModeState.h"
#include "XBeeStartupState.h"
#include "XBeeResetCompleteState.h"

void XBeeWaitForCommandModeState::OnTimerExpired()
	{
	std::cout << "XBee did not enter command mode, try again." << std::endl;
	machine.ChangeState(new XBeeWaitForCommandModeState(machine));
	}

void XBeeWaitForCommandModeState::OnEnter()
	{
	std::cout << "Attempting to put XBee into AT Command Mode..." << std::endl;
	machine.ListenInAtCommandMode();
	timer.SetDuration(XBEE_AT_GUARD_TIME * 3);
	machine.SendToLocalXbee("+++");
	}

void XBeeWaitForCommandModeState::OnSerialLineReceived(String& rxData) 
	{
	if (rxData != "OK")
		return;
	// Send configuration string to XBee
	std::cout << "In AT command mode, sending factory reset..." << std::endl;
	machine.SendToLocalXbee(xbeeInitString);	// Write factory defaults and reboot.
	machine.ChangeState(new XBeeResetCompleteState(machine));
	}


