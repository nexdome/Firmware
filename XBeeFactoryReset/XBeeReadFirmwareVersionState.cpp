// 
// 
// 

#include "XBeeReadFirmwareVersionState.h"
#include "XBeeResetCompleteState.h"
#include "XBeeWaitForPostConfigureCommandModeState.h"

void XBeeReadFirmwareVersionState::OnTimerExpired()
	{
	std::cout << "No response to ATVR command, try again." << std::endl;
	machine.ChangeState(new XBeeWaitForPostConfigureCommandModeState(machine));
	}

void XBeeReadFirmwareVersionState::OnEnter()
{
	timer.SetDuration(XBEE_AT_COMMAND_TIMEOUT);
	machine.sendToLocalXbee("ATVR\r");
}

void XBeeReadFirmwareVersionState::OnSerialLineReceived(const std::string& rxData)
	{
	std::cout << "Firmware version: " << rxData << std::endl;
	machine.ChangeState(new XBeeResetCompleteState(machine));
	}

