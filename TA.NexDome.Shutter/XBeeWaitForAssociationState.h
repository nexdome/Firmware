// XBeeWaitForAssociationState.h

#ifndef _XBEEWAITFORASSOCIATIONSTATE_h
#define _XBEEWAITFORASSOCIATIONSTATE_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "XBeeStateMachine.h"

class XBeeWaitForAssociationState : public IXBeeState
	{
public:
	String name() override { return "WaitAssociate"; }
	explicit XBeeWaitForAssociationState(XBeeStateMachine& machine) : IXBeeState(machine) { }
	void OnEnter() override;
	void OnModemStatusReceived(uint8_t state) override;
	};

#endif
