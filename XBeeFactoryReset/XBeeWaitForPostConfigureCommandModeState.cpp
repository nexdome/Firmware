//
//
//

#include "XBeeWaitForPostConfigureCommandModeState.h"
#include "XBeeReadFirmwareVersionState.h"

void XBeeWaitForPostConfigureCommandModeState::OnTimerExpired()
{
	std::cout << "XBee not ready yet, try again." << std::endl;
	machine.ChangeState(new XBeeWaitForPostConfigureCommandModeState(machine));
}

void XBeeWaitForPostConfigureCommandModeState::OnEnter()
{
	std::cout << "Waiting for XBee to reboot and enter AT Command Mode..." << std::endl;
	machine.ListenInAtCommandMode();
	machine.sendToLocalXbee(XBEE_ATTENTION);
	timer.SetDuration(XBEE_AT_GUARD_TIME);
}

void XBeeWaitForPostConfigureCommandModeState::OnSerialLineReceived(const std::string& rxData)
{
	if (rxData != "OK")
		return;
	// Send configuration string to XBee
	std::cout << "In AT command mode, reading firmware version..." << std::endl;
	machine.ChangeState(new XBeeReadFirmwareVersionState(machine));
}

