#include <XBeeStateMachine.h>
#include "XBeeWaitForCommandModeState.h"
#include "XBeeConfigureState.h"

void XBeeWaitForCommandModeState::OnTimerExpired()
	{
	machine.ChangeState(new XBeeWaitForCommandModeState(machine));
	}

void XBeeWaitForCommandModeState::OnEnter()
	{
	machine.ListenInAtCommandMode();
	machine.sendToLocalXbee(XBEE_ATTENTION);
	timer.SetDuration(XBEE_AT_GUARD_TIME);
	}

void XBeeWaitForCommandModeState::OnSerialLineReceived(const std::string& rxData) 
	{
	if (rxData != "OK")
		return;
	machine.ChangeState(new XBeeConfigureState(machine));
	}


