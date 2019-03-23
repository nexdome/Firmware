#include "XBeeResetCompleteState.h"
#include "XBeeStartupState.h"

void XBeeResetCompleteState::OnEnter()
	{
	timer.SetDuration(60000);	// 1 minute
	std::cout << "You may now upload a new sketch." << std::endl
		<< "Reboot or wait 1 minute to reset the XBee module again." << std::endl;
	}

void XBeeResetCompleteState::OnTimerExpired()
	{
	machine.ChangeState(new XBeeStartupState(machine));
	}
