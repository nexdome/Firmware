#include "XBeeWaitForOkState.h"

XBeeWaitForOkState::XBeeWaitForOkState(XBeeStateMachine& machine) : IXBeeState(machine)
	{
	timer.SetDuration(XBEE_AT_GUARD_TIME * 2);
	}



void XBeeWaitForOkState::OnTimerExpired()
	{
		if (timer.Expired())
		{
		machine.ChangeState(new XBeeStartupRotatorState(machine));
		}

	}

void XBeeWaitForOkState::OnEnter()
	{
	timer.SetDuration(XBEE_BOOT_TIME_MILLIS);
	}

void XBeeWaitForOkState::OnSerialLineReceived(String& rxData) 
	{
	if (rxData != "OK")
		return;
	// Set serial comms parameters and API mode 2
	machine.SendToXbee(XBEE_ROTATOR_INIT_STRING);	// Enter API mode.
	machine.ListenInApiMode();
	machine.ChangeState(new XBeeApiDetectShutterState(machine));
	}


