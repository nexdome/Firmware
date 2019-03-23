
#include "XBeeStartupState.h"
#include "XBeeWaitForCommandModeState.h"

void XBeeStartupState::OnEnter()
{
	std::cout << "Waiting for XBee to initialize" << std::endl;
	machine.ListenInAtCommandMode();
	timer.SetDuration(XBEE_BOOT_TIME_MILLIS);
}

void XBeeStartupState::OnTimerExpired()
{
	machine.ChangeState(new XBeeWaitForCommandModeState(machine));
}

