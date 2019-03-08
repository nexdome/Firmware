#include "XBeeApiDetectShutterState.h"
#include "XBeeShutterOnlineState.h"
#include "XBeeStartupState.h"

/*
 * We expect to receive a Hello message from the shutter XBee within a reasonable period
 * of time. We set the timer for that here, on entry to this state.
 */
void XBeeApiDetectShutterState::OnEnter()
	{
	timer.SetDuration(XBEE_REMOTE_HANDSHAKE_TIMEOUT);
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
void XBeeApiDetectShutterState::OnApiRx64FrameReceived(Rx64Response & frame)
	{
	const auto remote = frame.getRemoteAddress64().get();
	machine.SetDestinationAddress(remote);
	machine.ChangeState(new XBeeShutterOnlineState(machine));
	}
