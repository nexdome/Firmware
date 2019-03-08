// XbeeState.h

#ifndef _XBEESTATE_h
#define _XBEESTATE_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "Timer.h"
#include <XBee.h>

#define XBEE_BOOT_TIME_MILLIS (5000UL)			// Time for XBEE to become ready from cold reset
#define XBEE_AT_GUARD_TIME (1000UL)				// Wait time before sending "+++" and receiving "OK"
#define XBEE_REMOTE_HANDSHAKE_TIMEOUT (20000UL)	// Maximum time to wait for remote device to say hello.
#define XBEE_HELLO_MESSAGE "Yoohoo"				// A retro shout-out to FidoNet era mailer called dBridge.
#define XBEE_HELLO_ACK "Yoohoo2U2"				// (yes I am old enough to remember FidoNet).

//ToDo: The following defs are project specific and need to be moved into the projects.
#define XBEE_ROTATOR_INIT_STRING "ATCE1,ID6FBF,CHC,MYFFFF,DH0,DLFFFF,A27,AP2,SM0,WR,AC,FR,CN\x0D"

class IXBeeState;

class XBeeStateMachine
{
public:
	XBeeStateMachine(HardwareSerial& xBeePort, Stream& debugPort);
	void Loop();
	void SendToXbee(String message) const;
	void ChangeState(IXBeeState* newState);
	void ListenInAtCommandMode();
	void ListenInApiMode();
	unsigned int GetNextFrameId();
	void SendXbeeApiFrame(XBeeRequest& request);
	void SetDestinationAddress(uint64_t address);
	void XBeeApiSendMessage(const String& message);

private:
	void xbee_serial_receive();
	void xbee_api_receive();
	HardwareSerial& xbeeSerial;
	XBee xbeeApi;
	XBeeAddress64 remoteAddress;
	IXBeeState* currentState;
	unsigned long startTime;
	bool ApiModeEnabled = false;
	unsigned int frameId = 1;
	Stream& debug;
	Tx64Request tx64Frame;
};

class IXBeeState
{
public:
	// State machine "plumbing"
	explicit IXBeeState(XBeeStateMachine& machine) : machine(machine) {}
	virtual ~IXBeeState() = default;
	virtual String name() = 0;
	virtual void Loop()
	{
		if (timer.Enabled() && timer.Expired())
		{
			OnTimerExpired();
		}
	}
	virtual void OnExit() {}
	virtual void OnEnter() {}
	// State machine triggers
	virtual void OnTimerExpired() {}
	virtual void OnSerialLineReceived(String& rxData) {}
	virtual void OnApiRx64FrameReceived(Rx64Response& frame) {}
	virtual void SendMessage(Tx64Request& message) {}
	virtual void OnModemStatusReceived(uint8_t state) {}

protected:
	XBeeStateMachine& machine;
	Timer timer;
};

#endif
