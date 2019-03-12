// XbeeStateMachine.h

#ifndef _XBEESTATE_h
#define _XBEESTATE_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include <ArduinoSTL.h>
#include <XBeeApi.h>
#include <Timer.h>


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
	XBeeStateMachine(HardwareSerial& xBeePort, Stream& debugPort, XBeeApi& xbee);
	void Loop();
	void SendToLocalXbee(String message) const;
	void SendToRemoteXbee(String message);
	void ChangeState(IXBeeState* newState);
	void ListenInAtCommandMode();
	void ListenInApiMode();
	//void SendXbeeApiFrame(XBeeRequest& request);
	void SetDestinationAddress(std::vector<byte>& payload);
	void XBeeApiSendMessage(const String& message);
	void OnXbeeFrameReceived(FrameType type, std::vector<byte>& payload);
private:
	void xbee_serial_receive();
	void xbee_api_receive();
	void printEscaped(byte data);
	byte getNextFrameId();
	HardwareSerial& xbeeSerial;
	XBeeApi& xbeeApi;
	std::vector<byte> remoteAddress;
	IXBeeState* currentState;
	unsigned long startTime;
	bool ApiModeEnabled = false;
	byte frameId = 0;
	Stream& debug;
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
			Serial.println("Timeout");
			OnTimerExpired();
		}
	}
	virtual void OnExit() {}
	virtual void OnEnter() {}
	// State machine triggers
	virtual void OnTimerExpired() {}
	virtual void OnSerialLineReceived(String& rxData) {}
	virtual void OnApiRx64FrameReceived(std::vector<byte>& payload) {}
	virtual void OnModemStatusReceived(ModemStatus state) {}

protected:
	XBeeStateMachine& machine;
	static Timer timer;
};

#endif
