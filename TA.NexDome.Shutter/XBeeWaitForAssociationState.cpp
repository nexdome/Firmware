// 
// 
// 

#include "XBeeWaitForAssociationState.h"
#include "XBeeAssociatedState.h"
#include "XBeeStartupState.h"


void XBeeWaitForAssociationState::OnEnter()
	{
	timer.SetDuration(20000);
	machine.ListenInApiMode();
	}

void XBeeWaitForAssociationState::OnModemStatusReceived(uint8_t status)
	{
	// Any change of status resets the timer.
	timer.Repeat();
	if (status == ASSOCIATED)
		{
		machine.XBeeApiSendMessage(XBEE_HELLO_MESSAGE);
		machine.ChangeState(new XBeeAssociatedState(machine));
		}
	}

/*
 * If not associated with a coordinator within the allotted timeout,
 * start again from scratch and reconfigure the XBee.
 */
void XBeeWaitForAssociationState::OnTimerExpired()
	{
	machine.ChangeState(new XBeeStartupState(machine));
	}

