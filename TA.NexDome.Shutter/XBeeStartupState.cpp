
#include "XBeeStartupState.h"
#include "XBeeWaitForCommandModeState.h"
#include "NexDome.h"

void XBeeStartupState::OnEnter()
{
	machine.useCoordinatorAddress();			// Coordinator address is default destination
	machine.ListenInAtCommandMode();
	timer.SetDuration(XBEE_BOOT_TIME_MILLIS);	// Allow enough time for the XBee to boot up.
}

void XBeeStartupState::OnTimerExpired()
{
	// XBee should have booted by now, so try to put it into command mode.
	machine.SendToLocalXbee("+++");
	machine.ChangeState(new XBeeWaitForCommandModeState(machine));
}

