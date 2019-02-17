#include "XBeeApiDetectShutterState.h"
#include "XBeeShutterOnlineState.h"
#include "XBeeStartupState.h"


void XBeeApiDetectShutterState::OnEnter()
	{
	timer.SetDuration(6000000);	// If we haven't had any messages from the shutter in 10 seconds, try to reconfigure
	}

void XBeeApiDetectShutterState::OnTimerExpired()
	{
	machine.ChangeState(new XBeeStartupState(machine));
	}

void XBeeApiDetectShutterState::OnApiRx64FrameReceived(Rx64Response & frame)
	{
	auto & remote = frame.getRemoteAddress64();
	machine.SetShutterAddress(remote);
	machine.ChangeState(new XBeeShutterOnlineState(machine));
	}
