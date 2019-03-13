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
