// 
// 
// 

#include "XBeeAssociatedState.h"
#include "XBeeOnlineState.h"
#include "XBeeStartupState.h"

/*
 * Send a "Hello" message to the Rotator to let it know we are alive and
 * so that it can receive our 64-bit unique address. We expect it to reply
 * within a certain time, se we set the timer for that here too.
 */
void XBeeAssociatedState::OnEnter()
	{
	timer.SetDuration(XBEE_REMOTE_HANDSHAKE_TIMEOUT);
	sendHello();
	}

inline void XBeeAssociatedState::sendHello()
	{
	static const std::string hello(XBEE_HELLO_MESSAGE);
	machine.SendToRemoteXbee(hello);
	}

/*
 * We are expecting the Rotator to acknowledge our hello message.
 * It will come in the payload of an Rx response with 64-bit address.
 * All other frame types are ignored at this point.
 */
void XBeeAssociatedState::OnApiRx64FrameReceived(const std::vector<byte>& payload)
	{
	// If it's the ACK message, save the remote address, and go online.
	// Check whether the message is correct, otherwise ignore frame
	// Skip first 10 bytes of payload, look for string in bytes 11 onward.
	const auto msgStart = payload.begin() + 10;
	const auto msgEnd = payload.end();
	const std::string rxMessage(msgStart, msgEnd);
	if (rxMessage.compare(XBEE_HELLO_ACK) == 0)
		{
		machine.SetDestinationAddress(payload);
		machine.ChangeState(new XBeeOnlineState(machine));
		}
	}

/*
 * No acknowledgement from the rotator means something is wrong. Go right back to scratch,
 * and reconfigure the XBee.
 */
void XBeeAssociatedState::OnTimerExpired()
	{
	timer.SetDuration(XBEE_REMOTE_HANDSHAKE_TIMEOUT);
	sendHello();
	}
