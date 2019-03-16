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
	// Now what?
}

void XBeeShutterOnlineState::SendCommand(std::string& command)
{
	std::cout << "Remote command: " << command << std::endl;
	machine.SendToRemoteXbee(command);
}
