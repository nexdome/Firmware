// 
// 
// 

#include "XBeeOnlineState.h"
#include "XBeeAssociatedState.h"
#include "Response.h"

extern Response DispatchCommand(const std::string& command);	// defined in global scope in the .ino file

void XBeeOnlineState::OnEnter()
{
	timer.SetDuration(XBEE_NO_ACTIVITY_TIMEOUT);
}

void XBeeOnlineState::OnTimerExpired()
{
	machine.ChangeState(new XBeeAssociatedState(machine));
}

void XBeeOnlineState::OnApiRx64FrameReceived(const std::vector<byte>& payload)
{
	auto length = payload.size();
	// Minimum frame length is 10 bytes for a null message.
	if (length < 10)
		return;	// invalid frame - ignore
	// payload[0] is the source address (8 bytes).
	byte rssi = payload[8];
	byte options = payload[9];
	// payload[10] is the first byte of the received data
	if (length > 10 && payload[10] == '@')
	{
		// treat as a valid remote command
		auto commandString = std::string(payload.begin() + 10, payload.end());
		auto response = DispatchCommand(commandString);
	}
}
