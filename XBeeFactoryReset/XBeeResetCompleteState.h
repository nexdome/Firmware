// XBeeStartupState.h

#ifndef _XBEERESETCOMPLETESTATE_h
#define _XBEERESETCOMPLETESTATE_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#include <XBeeStateMachine.h>

class XBeeResetCompleteState : public IXBeeState
{
public:
	std::string name() override { return "Factory Reset Complete"; }
	void OnEnter() override;
	explicit XBeeResetCompleteState(XBeeStateMachine& machine) : IXBeeState(machine) {}
};

#endif

