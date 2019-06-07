// XBeeWaitForAssociationState.h

#ifndef _XBEEWAITFORASSOCIATIONSTATE_h
#define _XBEEWAITFORASSOCIATIONSTATE_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include <XBeeStateMachine.h>

class XBeeWaitForAssociationState : public IXBeeState
	{
public:
	std::string name() override { return "WaitAssociate"; }
	explicit XBeeWaitForAssociationState(XBeeStateMachine& machine) : IXBeeState(machine) { }
	void OnEnter() override;
	void OnModemStatusReceived(ModemStatus status) override;
	void OnTimerExpired() override;
	};

#endif
