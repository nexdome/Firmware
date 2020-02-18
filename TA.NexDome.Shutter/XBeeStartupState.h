// XBeeStartupState.h

#ifndef _XBEESTARTUPSTATE_h
#define _XBEESTARTUPSTATE_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif
#include <XBeeStateMachine.h>

/*
 * The time that the XBee state machine will wait before attempting to configure the XBee.
 * The idea here is to give the XBee time to initialize and become ready.
 * Also, the shutter boot delay is deliberately greater than the rotator delay, so that if both
 * happen to be powered on simultaneously, then the rotator has time to be configured and ready
 * when the shutter XBee tries to associate with the coordinator.
*/
constexpr Duration XbeeBootTime = Timer::Seconds(6);

class XBeeStartupState : public IXBeeState
{
public:
	std::string name() override { return "Start"; }
	void OnEnter() override;
	void OnTimerExpired() override;
	explicit XBeeStartupState(XBeeStateMachine& machine) : IXBeeState(machine) {}
};

#endif

