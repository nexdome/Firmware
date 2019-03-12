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
	machine.XBeeApiSendMessage(XBEE_HELLO_MESSAGE);
	}

/*
 * We are expecting the Rotator to acknowledge our hello message.
 * It will come in the payload of an Rx response with 64-bit address.
 * All other frame types are ignored at this point.
 */
void XBeeAssociatedState::OnApiRx64FrameReceived(std::vector<byte>& payload)
	{
	//if( memcmp(xbeeHelloAckMessage.begin(), payload, xbeeHelloAckMessage.length())==0)
	//	{
	//	// Showtime
	//	machine.SetDestinationAddress(sender);
	//	machine.ChangeState(new XBeeOnlineState(machine));
	//	}
	}

/*
 * No acknowledgement from the rotator means something is wrong. Go right back to scratch,
 * and reconfigure the XBee.
 */
void XBeeAssociatedState::OnTimerExpired()
	{
	machine.ChangeState(new XBeeStartupState(machine));
	}
