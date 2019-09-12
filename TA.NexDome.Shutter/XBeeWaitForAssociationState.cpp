// 
// 
// 

#include "XBeeWaitForAssociationState.h"
#include "XBeeAssociatedState.h"
#include "XBeeStartupState.h"


void XBeeWaitForAssociationState::OnEnter()
	{
	machine.useCoordinatorAddress();
	machine.ListenInApiMode();
	timer.SetDuration(XBEE_ASSOCIATE_TIMEOUT);
	}

void XBeeWaitForAssociationState::OnModemStatusReceived(ModemStatus status)
	{
	// Any change of status resets the timer.
	timer.SetDuration(XBEE_ASSOCIATE_TIMEOUT);
	if (status == Associated)
		{
		machine.ChangeState(new XBeeAssociatedState(machine));
		}
	}

/*
 * If not associated with a coordinator within the allotted timeout,
 * start again from scratch and reconfigure the XBee.
 */
void XBeeWaitForAssociationState::OnTimerExpired()
	{
	machine.CCA = std::min(machine.CCA + machine.CCAIncrement, machine.CCAMax);
	std::cout << "Setting CCA to " << std::hex << machine.CCA << std::dec << std::endl;
	machine.ChangeState(new XBeeStartupState(machine));
	}
