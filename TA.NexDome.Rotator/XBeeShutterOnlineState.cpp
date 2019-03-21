// 
// 
// 

#include "XBeeShutterOnlineState.h"

void XBeeShutterOnlineState::OnEnter()
{
	static const std::string ack(XBEE_HELLO_ACK);
	machine.SendToRemoteXbee(ack);
	timer.SetDuration(XBEE_NO_ACTIVITY_TIMEOUT);
}

void XBeeShutterOnlineState::OnApiRx64FrameReceived(const std::vector<byte>& payload)
{
	static const std::string ShutterHelloMessage(XBEE_HELLO_MESSAGE);
	// Check whether the message is correct, otherwise ignore frame
	// Skip first 10 bytes of payload, look for string in bytes 11 onward.
	auto msgStart = payload.begin() + 10;
	auto msgEnd = payload.end();
	std::string rxMessage(msgStart, msgEnd);
	if (rxMessage.compare(XBEE_HELLO_MESSAGE) == 0)
	{
		machine.SetDestinationAddress(payload);
		machine.ChangeState(new XBeeShutterOnlineState(machine));
		return;
	}

	// If no other interpretation seems appropriate, then send to the host.
	std::cout << rxMessage << std::endl;
}

void XBeeShutterOnlineState::SendCommand(std::string& command)
{
	//std::cout << "Remote command: " << command << std::endl;
	machine.SendToRemoteXbee(command);
}
