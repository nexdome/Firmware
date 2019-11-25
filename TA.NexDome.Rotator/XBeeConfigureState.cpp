
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
			std::cout << message;
			message.push_back('\r');
			std::cout << message /*<< std::endl*/;
			machine.sendToLocalXbee(message);
			timer.SetDuration(XBEE_AT_COMMAND_TIMEOUT);
			return true;
		}
		message.push_back(ch);
	}
}

void XBeeConfigureState::OnSerialLineReceived(const std::string& message)
	{
	std::cout << " - " << message << std::endl;
	if (message == "OK")
		{
		if (!sendNextAtCommand())
			machine.ChangeState(new XBeeApiDetectShutterState(machine));
		return;
		}
	// Any response but "OK" causes the configuration process to be restarted from scratch
	machine.ChangeState(new XBeeStartupState(machine));
	}

