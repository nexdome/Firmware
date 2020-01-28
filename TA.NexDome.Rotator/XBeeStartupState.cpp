
#include "XBeeStartupState.h"
#include "XBeeWaitForCommandModeState.h"

void XBeeStartupState::OnEnter()
{
	//machine.ListenInAtCommandMode();
	timer.SetDuration(XbeeBootTime);
}

void XBeeStartupState::OnTimerExpired()
{
	machine.ChangeState(new XBeeWaitForCommandModeState(machine));
}

