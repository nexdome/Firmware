// XBeeWaitForPostConfigureCommandModeState.h

#ifndef _XBEEWAITFORPOSTCONFIGURECOMMANDMODESTATE_h
#define _XBEEWAITFORPOSTCONFIGURECOMMANDMODESTATE_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
#include "XBeeStateMachine.h"

class XBeeWaitForPostConfigureCommandModeState : public IXBeeState
{
public:
	explicit XBeeWaitForPostConfigureCommandModeState(XBeeStateMachine& machine) :IXBeeState(machine) {};
	std::string name() override { return "Wait for AT Command Mode"; }
	void OnTimerExpired() override;
	void OnEnter() override;
	void OnSerialLineReceived(const std::string& rxData) override;
};

#endif

