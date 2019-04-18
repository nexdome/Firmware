#include "XBeeWaitForCommandModeState.h"
#include "XBeeResetCompleteState.h"
#include "XBeeConfigureState.h"

void XBeeWaitForCommandModeState::OnTimerExpired()
	{
	std::cout << "XBee did not enter command mode, try again." << std::endl;
	machine.ChangeState(new XBeeWaitForCommandModeState(machine));
	}

void XBeeWaitForCommandModeState::OnEnter()
	{
	std::cout << "Attempting to put XBee into AT Command Mode..." << std::endl;
	machine.ListenInAtCommandMode();
	machine.sendToLocalXbee(XBEE_ATTENTION);
	timer.SetDuration(XBEE_AT_GUARD_TIME);
	}

void XBeeWaitForCommandModeState::OnSerialLineReceived(const std::string& rxData) 
	{
	if (rxData != "OK")
		return;
	// Send configuration string to XBee
	std::cout << "In AT command mode, configuring..." << std::endl;
	machine.ChangeState(new XBeeConfigureState(machine));
	}


