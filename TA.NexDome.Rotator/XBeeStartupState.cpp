
#include "XBeeStartupState.h"
#include "XBeeWaitForOkState.h"

String XBeeStartupState::name() { return "Start"; }

void XBeeStartupState::OnEnter()
{
	machine.ListenInAtCommandMode();
	timer.SetDuration(XBEE_BOOT_TIME_MILLIS);
}

void XBeeStartupState::OnTimerExpired()
{
	machine.SendToXbee("+++");
	machine.ChangeState(new XBeeWaitForOkState(machine));
}

