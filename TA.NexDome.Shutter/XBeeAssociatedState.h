// XBeeShutterReadyState.h

#ifndef _XBEESHUTTERREADYSTATE_h
#define _XBEESHUTTERREADYSTATE_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <XBeeStateMachine.h>

class XBeeAssociatedState : public IXBeeState
	{
public:
	explicit XBeeAssociatedState(XBeeStateMachine& machine) : IXBeeState(machine){}
	const std::string name() override { return "Associated"; }
	void OnEnter() override;
	void OnApiRx64FrameReceived(const std::vector<byte>& payload) override;
	void OnTimerExpired() override;
private:
	void sendHello();
	};

#endif

