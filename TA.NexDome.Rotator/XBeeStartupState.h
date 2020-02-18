// XBeeStartupState.h

#ifndef _XBEESTARTUPSTATE_h
#define _XBEESTARTUPSTATE_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif
#include <XBeeStateMachine.h>

// The time that the state machine will wait before attempting to configure the XBee
constexpr Duration XbeeBootTime = Timer::Seconds(2);

class XBeeStartupState : public IXBeeState
{
public:
	std::string name() override { return "Start"; }
	void OnEnter() override;
	void OnTimerExpired() override;
	explicit XBeeStartupState(XBeeStateMachine& machine) : IXBeeState(machine) {}
};

#endif

