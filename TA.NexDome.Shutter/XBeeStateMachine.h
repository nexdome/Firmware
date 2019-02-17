// XbeeState.h

#ifndef _XBEESTATE_h
#define _XBEESTATE_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include <XBee-Arduino_library/XBee.h>
#include "Timer.h"

class IXBeeState;

class XBeeStateMachine
{
public:
	XBeeStateMachine(HardwareSerial& serialPort, Stream& debugPort);
	void Loop();
	void SendToXbee(String message) const;
	void ChangeState(IXBeeState* newState);
	void ListenInAtCommandMode();
	void ListenInApiMode();
	void SetShutterAddress(const XBeeAddress64& remote);
	unsigned int GetNextFrameId();
void SendXbeeApiFrame(XBeeRequest& request);

private:
	void xbee_serial_receive();
	void xbee_api_receive();
	HardwareSerial& xbeeSerial;
	XBee xbeeApi;
	XBeeAddress64 shutterAddress;
	IXBeeState* currentState;
	unsigned long startTime;
	bool ApiModeEnabled = false;
	unsigned int frameId = 1;
	Stream& debug;
};


class IXBeeState
{
public:
	// State machine "plumbing"
	IXBeeState(XBeeStateMachine& machine) : machine(machine) {}
	virtual ~IXBeeState() = default;
	virtual String name() = 0;
	virtual void Loop() { if (timer.Expired()) OnTimerExpired(); }
	virtual void OnExit() {}
	virtual void OnEnter() {}
	// State machine triggers
	virtual void OnTimerExpired() {}
	virtual void OnSerialLineReceived(String& rxData) {}
	virtual void OnApiRx64FrameReceived(Rx64Response& frame) {}
	virtual void OnModemStatusReceived(uint8_t state) {}
	virtual void SendMessage(Tx64Request& message) {}
protected:
	XBeeStateMachine& machine;
	Timer timer;
};

#endif
