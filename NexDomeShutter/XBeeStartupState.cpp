
#include "XBeeStartupState.h"
#include "XBeeWaitForCommandModeState.h"

String XBeeStartupState::name() { return "Start"; }

void XBeeStartupState::OnEnter()
{
	machine.SetDestinationAddress(0x0000FFFEu);	// Coordinator address is default destination
	machine.ListenInAtCommandMode();
	timer.SetDuration(XBEE_BOOT_TIME_MILLIS);	// Allow enough time for the XBee to boot up.
}

void XBeeStartupState::OnTimerExpired()
{
	// XBee should have booted by now, so try to put it into command mode.
	machine.SendToXbee("+++");
	machine.ChangeState(new XBeeWaitForCommandModeState(machine));
}

