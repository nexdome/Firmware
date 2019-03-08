#include "XBeeWaitForCommandModeState.h"
#include "XBeeStartupState.h"
#include "XBeeWaitForAssociationState.h"
#include "NexDome.h"

void XBeeWaitForCommandModeState::OnTimerExpired()
	{
		if (timer.Expired())
		{
		machine.ChangeState(new XBeeStartupState(machine));
		}
	}

void XBeeWaitForCommandModeState::OnEnter()
	{
	timer.SetDuration(XBEE_AT_GUARD_TIME * 3);
	}

void XBeeWaitForCommandModeState::OnSerialLineReceived(String& rxData) 
	{
	if (rxData != "OK")
		return;
	// Set serial comms parameters and API mode 2
	machine.SendToXbee(XBeeInitString);	// Enter API mode.
	machine.ChangeState(new XBeeWaitForAssociationState(machine));
	}


