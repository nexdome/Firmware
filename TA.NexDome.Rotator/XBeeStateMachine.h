// XbeeState.h

#ifndef _XBEESTATE_h
#define _XBEESTATE_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "NexDome.h"
#include "Command.h"
#include "Timer.h"
#include <XBee-Arduino_library/XBee.h>

class IXBeeState;

class XBeeStateMachine
{
public:
	XBeeStateMachine(HardwareSerial& serialPort);
	void Loop();
	void SendToXbee(String message);
	void ChangeState(IXBeeState* newState);
	void ListenInAtCommandMode();
	void ListenInApiMode();

private:
	void xbee_serial_receive();
	void xbee_api_receive();
	HardwareSerial& xbeeSerial;
	XBee xbeeApi;
	IXBeeState* currentState;
	unsigned long startTime;
	bool ApiModeEnabled = false;
	//friend class XBeeStartupRotatorState;
};


class IXBeeState
{
public:
	// State machine "plumbing"
	virtual String name() = 0;
	virtual void Loop() { if (timer.Expired()) OnTimerExpired(); }
	virtual void OnExit() {};
	virtual void OnEnter() {};
	IXBeeState(XBeeStateMachine& machine) : machine(machine) {}
	virtual ~IXBeeState();
	// State machine triggers
	virtual void OnTimerExpired() {};
	virtual void OnSerialLineReceived(String& rxData) {};
	virtual void OnApiRx64FrameReceived(Rx64Response& frame) {};
protected:
	XBeeStateMachine& machine;
	Timer timer;
};

class XBeeStartupRotatorState : public IXBeeState
{
public:
	String name() override { return "Start"; };
	void OnEnter() override;
	void Loop() override;
	void OnTimerExpired() override;
	XBeeStartupRotatorState(XBeeStateMachine& machine);
};
#endif
