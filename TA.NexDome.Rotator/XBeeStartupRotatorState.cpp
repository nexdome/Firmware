#include "XBeeStateMachine.h"
#include "XBeeWaitForOkState.h"

void XBeeStartupRotatorState::OnEnter()
{
	machine.ListenInAtCommandMode();
	timer.SetDuration(XBEE_BOOT_TIME_MILLIS);
}

void XBeeStartupRotatorState::OnTimerExpired()
	{
		machine.SendToXbee("+++");
		machine.ChangeState(new XBeeWaitForOkState(machine));
	}



