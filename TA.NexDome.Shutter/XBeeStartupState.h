// XBeeStartupState.h

#ifndef _XBEESTARTUPSTATE_h
#define _XBEESTARTUPSTATE_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
#include <XBeeStateMachine.h>

class XBeeStartupState : public IXBeeState
{
public:
	std::string name() override { return "Start"; }
	void OnEnter() override;
	void OnTimerExpired() override;
	explicit XBeeStartupState(XBeeStateMachine& machine) : IXBeeState(machine) {}
};

#endif

