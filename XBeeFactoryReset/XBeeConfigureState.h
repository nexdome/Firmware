// XBeeConfigureState.h

#ifndef _XBEECONFIGURESTATE_h
#define _XBEECONFIGURESTATE_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <XBeeStateMachine.h>

class XBeeConfigureState : public IXBeeState
{
public:
	explicit XBeeConfigureState(XBeeStateMachine& machine) : IXBeeState(machine) { index = 0; };
	std::string name() override { return "Config"; };
	void OnTimerExpired() override;
	void OnEnter() override;
	void OnSerialLineReceived(const std::string& message) override;

private:
bool sendNextAtCommand();
	unsigned short index;
	const char* initSequence __ATTR_PROGMEM__ = "RE,WR,AC,FR,FR";
};

#endif

