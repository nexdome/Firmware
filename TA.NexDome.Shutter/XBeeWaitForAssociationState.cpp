// 
// 
// 

#include "XBeeWaitForAssociationState.h"
#include "XBeeAssociatedState.h"


void XBeeWaitForAssociationState::OnEnter()
	{
	timer.SetDuration(20000);
	}

void XBeeWaitForAssociationState::OnModemStatusReceived(uint8_t status)
	{
	// Any change of status resets the timer.
	timer.Repeat();
	if (status == ASSOCIATED)
		{
		machine.ChangeState(new XBeeAssociatedState(machine));
		}
	}

