// 
// 
// 

#include "XBeeOnlineState.h"
#include "XBeeStartupState.h"
#include "Response.h"

extern Response DispatchCommand(const std::string& command);	// defined in global scope in the .ino file

void XBeeOnlineState::OnEnter()
	{
	timer.SetDuration(XBEE_HEARTBEAT_INTERVAL);
	handshakeTimer.Stop();
	}

inline void XBeeOnlineState::sendHello()
	{
	static const std::string hello(XBEE_HELLO_MESSAGE);
	machine.SendToRemoteXbee(hello);
	handshakeTimer.SetDuration(XBEE_REMOTE_HANDSHAKE_TIMEOUT);
	}

void XBeeOnlineState::OnTimerExpired()
	{
	sendHello();
	}

void XBeeOnlineState::Loop()
	{
	if (handshakeTimer.Expired())
		{
		machine.ChangeState(new XBeeStartupState(machine));
		}
	}

void XBeeOnlineState::OnApiRx64FrameReceived(const std::vector<byte>& payload)
	{
	const auto length = payload.size();
	// Minimum frame length is 10 bytes for a null message.
	if (length < 10)
		return;	// invalid frame - ignore
	byte options = payload[9];
	// payload[10] is the first byte of the received data
	if (length > 10 && payload[10] == '@')
		{
		// treat as a valid remote command
		const auto commandString = std::string(payload.begin() + 10, payload.end());
		const auto response = DispatchCommand(commandString);
		machine.SendToRemoteXbee(response.Message);
		return;
		}
	// Test for "Hello" acknowledgement
	const auto msgStart = payload.begin() + 10;
	const auto msgEnd = payload.end();
	const std::string rxMessage(msgStart, msgEnd);
	if (rxMessage == XBEE_HELLO_ACK)
		{
		handshakeTimer.Stop();
		}
	}
