#include "XBeeWaitForCommandModeState.h"
#include "XBeeStartupState.h"
#include "XBeeApiDetectShutterState.h"

void XBeeWaitForCommandModeState::OnTimerExpired()
	{
	machine.ChangeState(new XBeeStartupState(machine));
	}

void XBeeWaitForCommandModeState::OnEnter()
	{
	timer.SetDuration(XBEE_AT_GUARD_TIME * 2);
	}

void XBeeWaitForCommandModeState::OnSerialLineReceived(String& rxData) 
	{
	if (rxData != "OK")
		return;
	// Send configuration string to XBee
	machine.SendToLocalXbee(rotatorInitString);	// Enter API2 mode.
	machine.ChangeState(new XBeeApiDetectShutterState(machine));
	}


