// XBeeOnlineState.h

#ifndef _XBEEONLINESTATE_h
#define _XBEEONLINESTATE_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif
#include "XBeeStateMachine.h"

class XBeeOnlineState : public IXBeeState
	{
public:
	explicit XBeeOnlineState(XBeeStateMachine& machine) : IXBeeState(machine) {  }
	 std::string name() override { return ("Online"); }
	void OnEnter() override;
	void OnTimerExpired() override;
	void Loop() override;
	virtual void OnApiRx64FrameReceived(const std::vector<byte>& payload) override;
private:
	Timer handshakeTimer;
	void sendHello();
};

#endif

