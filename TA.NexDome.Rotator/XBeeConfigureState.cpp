
#include "XBeeConfigureState.h"
#include "XBeeApiDetectShutterState.h"
#include "XBeeStartupState.h"

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
	message.clear();
	message.append("AT");
	while (true)
	{
		auto ch = initSequence[index++];
		if (ch == 0) return false;
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
		machine.ChangeState(new XBeeApiDetectShutterState(machine));
	}
