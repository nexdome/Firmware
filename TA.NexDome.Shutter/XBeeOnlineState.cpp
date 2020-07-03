//
// Handles Rotator <-> Shutter comms when the connection is established and stable
//

#include "XBeeOnlineState.h"
#include "XBeeStartupState.h"
#include "Response.h"

extern void DispatchCommand(const Command& command);	// defined in global scope in the .ino file

void XBeeOnlineState::OnEnter()
	{
	timer.SetDuration(XBEE_HEARTBEAT_INTERVAL);
	handshakeTimer.Stop();
	}

inline void XBeeOnlineState::sendHello()
	{
	static const std::string hello(XBEE_HELLO_MESSAGE);
	machine.SendToRemoteXbee(hello);
	timer.SetDuration(XBEE_HEARTBEAT_INTERVAL);
	handshakeTimer.SetDuration(XBEE_REMOTE_HANDSHAKE_TIMEOUT);
#ifdef DEBUG_HEARTBEAT
	std::cout << "ping " << millis() << std::endl;
#endif
	}

void XBeeOnlineState::OnTimerExpired()
	{
	sendHello();
	}

void XBeeOnlineState::Loop()
	{
	IXBeeState::Loop();
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
    const auto *const msgStart = payload.begin() + 10;
    const auto *const msgEnd = payload.end();
	const std::string rxMessage(msgStart, msgEnd);
#ifdef DEBUG_XBEE_API
	std::cout << "Rx64 " << rxMessage << std::endl;
#endif
	// payload[10] is the first byte of the received data
	// Hand the payload to the command dispatcher and send the response to the remote XBee.
    if (length > 10 && payload[10] == '@')
    {
		//std::cout << "Cmd " << rxMessage << std::endl;
        auto command = Command(rxMessage);
		DispatchCommand(command);
        machine.SendToRemoteXbee(ResponseBuilder::Message);
        return;
    }
	// Not a command. Test for "Hello" acknowledgement
	if (rxMessage == XBEE_HELLO_ACK)
		{
		handshakeTimer.Stop();
		timer.SetDuration(XBEE_HEARTBEAT_INTERVAL);
		}
	}
