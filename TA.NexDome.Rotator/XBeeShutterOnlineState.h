// XBeeShutterReadyState.h

#ifndef _XBEESHUTTERREADYSTATE_h
#define _XBEESHUTTERREADYSTATE_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include <XBeeStateMachine.h>

class XBeeShutterOnlineState : public IXBeeState
	{
protected:
	void OnTimerExpired() override;
public:
	std::string name() override { return stateName; };
	explicit XBeeShutterOnlineState(XBeeStateMachine& machine) : IXBeeState(machine) {}
	void OnEnter() override;
	void OnApiRx64FrameReceived(const std::vector<byte>& payload) override;
	void SendCommand(std::string& command) override;
private:
	const char* stateName __ATTR_PROGMEM__ = "Online";
	};


#endif
