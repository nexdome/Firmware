// 
// 
// 

#include "XBeeOnlineState.h"
#include "XBeeAssociatedState.h"

void XBeeOnlineState::OnEnter()
{
	timer.SetDuration(XBEE_NO_ACTIVITY_TIMEOUT);
}

void XBeeOnlineState::OnTimerExpired()
{
	machine.ChangeState(new XBeeAssociatedState(machine));
}
