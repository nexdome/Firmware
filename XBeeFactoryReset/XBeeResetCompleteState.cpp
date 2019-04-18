#include "XBeeResetCompleteState.h"

void XBeeResetCompleteState::OnEnter()
	{
	timer.Stop();
	std::cout << "Factory reset complete." << std::endl;
	std::cout << "You may now upload a new sketch, reboot, or power off." << std::endl;
	}
