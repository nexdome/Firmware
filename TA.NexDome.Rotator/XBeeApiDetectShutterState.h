// XBeeApiDetectShutterState.h

#ifndef _XBEEAPIDETECTSHUTTERSTATE_h
#define _XBEEAPIDETECTSHUTTERSTATE_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "XBeeStateMachine.h"

class XBeeApiDetectShutterState : public IXBeeState
	{
public:
	explicit XBeeApiDetectShutterState(XBeeStateMachine& machine) : IXBeeState(machine) {}
	String name() override { return "DetectShutter"; }
	void OnEnter() override;
	void OnTimerExpired() override;
	void OnApiRx64FrameReceived(Rx64Response& frame) override;
	};

#endif

