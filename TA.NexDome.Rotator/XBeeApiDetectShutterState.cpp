// 
// 
// 

#include "XBeeApiDetectShutterState.h"


void XBeeApiDetectShutterState::OnEnter()
	{
	timer.SetDuration(10000);	// If we haven't had any messages from the shutter in 10 seconds, try to reconfigure
	}

void XBeeApiDetectShutterState::OnTimerExpired()
	{
	machine.ChangeState(new XBeeStartupRotatorState(machine));
	}

void XBeeApiDetectShutterState::OnApiRx64FrameReceived(Rx64Response & frame)
	{
	auto & remote = frame.getRemoteAddress64();
	}
