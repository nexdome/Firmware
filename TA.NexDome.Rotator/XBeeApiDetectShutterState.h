// XBeeApiDetectShutterState.h

#ifndef _XBEEAPIDETECTSHUTTERSTATE_h
#define _XBEEAPIDETECTSHUTTERSTATE_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include <XBeeStateMachine.h>

class XBeeApiDetectShutterState : public IXBeeState
	{
public:
	explicit XBeeApiDetectShutterState(XBeeStateMachine& machine) : IXBeeState(machine) {}
	std::string name() override { return stateName; }
	void OnEnter() override;
	void OnTimerExpired() override;
	void OnApiRx64FrameReceived(const std::vector<byte>& payload) override;
private:
	const char* stateName __ATTR_PROGMEM__ = "Detect";
	};

#endif

