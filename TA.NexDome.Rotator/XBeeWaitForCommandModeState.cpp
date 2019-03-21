#include "XBeeWaitForCommandModeState.h"
#include "XBeeStartupState.h"
#include "XBeeApiDetectShutterState.h"

void XBeeWaitForCommandModeState::OnTimerExpired()
	{
	machine.ChangeState(new XBeeStartupState(machine));
	}

void XBeeWaitForCommandModeState::OnEnter()
	{
	machine.ListenInAtCommandMode();
	timer.SetDuration(XBEE_AT_GUARD_TIME * 3);
	machine.SendToLocalXbee("+++");
	}

void XBeeWaitForCommandModeState::OnSerialLineReceived(String& rxData) 
	{
	if (rxData != "OK")
		return;
	// Send configuration string to XBee
	machine.SendToLocalXbee(rotatorInitString);	// Enter API2 mode.
	machine.ChangeState(new XBeeApiDetectShutterState(machine));
	}


