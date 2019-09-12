
#include "XBeeConfigureState.h"
#include "XBeeStartupState.h"
#include "XBeeWaitForAssociationState.h"

void XBeeConfigureState::OnTimerExpired()
	{
	machine.ChangeState(new XBeeStartupState(machine));
	}

void XBeeConfigureState::OnEnter()
	{
	sendNextAtCommand();
	}

bool XBeeConfigureState::sendNextAtCommand()
{
	static std::string message;
	static char ccaBuffer[8];
	message.clear();
	message.append("AT");
	while (true)
	{
		auto ch = initSequence[index++];
		if (ch == 0) return false;
		if (ch == '$')
			{
			itoa(machine.CCA, ccaBuffer, 16);
			std::cout << "CCA " << ccaBuffer << std::endl;
			message.push_back(ccaBuffer[0]);
			if (ccaBuffer[1] != '\0')
				message.push_back(ccaBuffer[1]);
			continue;
			}
		if (ch == ',')
		{
			message.push_back('\r');
			machine.sendToLocalXbee(message);
			timer.SetDuration(XBEE_AT_COMMAND_TIMEOUT);
			return true;
		}
		message.push_back(ch);
	}
}

void XBeeConfigureState::OnSerialLineReceived(const std::string& message)
	{
	if (message != "OK")
		machine.ChangeState(new XBeeStartupState(machine));

	if (sendNextAtCommand() == false)
		machine.ChangeState(new XBeeWaitForAssociationState(machine));
	}

