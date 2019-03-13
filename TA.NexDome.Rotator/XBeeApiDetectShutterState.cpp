#include "XBeeApiDetectShutterState.h"
#include "XBeeShutterOnlineState.h"
#include "XBeeStartupState.h"

/*
 * We expect to receive a Hello message from the shutter XBee within a reasonable period
 * of time. We set the timer for that here, on entry to this state.
 */
void XBeeApiDetectShutterState::OnEnter()
	{
	timer.SetDuration(XBEE_DETECT_SHUTTER_TIMEOUT);
	machine.ListenInApiMode();
	}

/*
 * If we haven't received a hello message from the shutter after a reasonable time,
 * then we assume that the coordinator has not started up correctly, and we
 * reconfigure it from scratch.
 */
void XBeeApiDetectShutterState::OnTimerExpired()
	{
	machine.ChangeState(new XBeeStartupState(machine));
	}

/*
 * When we detect the shutter (when it sends us a Tx64 frame) then
 * we can obtain its full 64-bit unique address from the frame header.
 * We configure the state machine to use that address for future transmissions.
 * This allows as to talk directly and specifically to the shutter.
 */
void XBeeApiDetectShutterState::OnApiRx64FrameReceived(const std::vector<byte>& payload)
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
	}
}
