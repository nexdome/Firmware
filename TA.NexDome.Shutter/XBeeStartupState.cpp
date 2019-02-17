
#include "XBeeStartupState.h"
#include "XBeeWaitForCommandModeState.h"
#include "NexDome.h"

String XBeeStartupState::name() { return "Start"; }

void XBeeStartupState::OnEnter()
{
	machine.ListenInAtCommandMode();
	timer.SetDuration(XBEE_BOOT_TIME_MILLIS);
}

void XBeeStartupState::OnTimerExpired()
{
	machine.SendToXbee("+++");
	machine.ChangeState(new XBeeWaitForCommandModeState(machine));
}

