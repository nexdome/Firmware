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
#define XBEE_REMOTE_HANDSHAKE_TIMEOUT (5000UL)	// Maximum time to wait for rotator to acknowledge the hello message.
#define XBEE_DETECT_SHUTTER_TIMEOUT (30000UL)	// Maximum time to wait for the shutter to say hello.
#define XBEE_HEARTBEAT_TIMEOUT (10000UL)		// Time before we assume that the remote link is down
#define XBEE_HELLO_MESSAGE "Yoohoo"				// A retro shout-out to FidoNet era mailer called dBridge.
#define XBEE_HELLO_ACK "Yoohoo2U2"				// (yes I am old enough to remember FidoNet).

//ToDo: The following defs are project specific and need to be moved into the projects.
#define XBEE_ROTATOR_INIT_STRING "ATCE1,ID6FBF,CHC,MYFFFF,DH0,DLFFFF,A27,AP2,SM0,WR,AC,FR,CN\x0D"
#define XBEE_SHUTTER_INIT_STRING "ATCE0,ID6FBF,CHC,MYFFFF,DH0,DLFFFE,A15,AP2,SM0,AC,FR,CN\0xD"

class IXBeeState;

class XBeeStateMachine
{
public:
	XBeeStateMachine(HardwareSerial& xBeePort, Stream& debugPort, XBeeApi& xbee);
	void Loop();
	void ChangeState(IXBeeState* newState);
	void ListenInAtCommandMode();
	void ListenInApiMode();
	void SendToLocalXbee(String message) const;
	void SendToRemoteXbee(const std::string& message);
	void SetDestinationAddress(const std::vector<byte>& payload);
	void useCoordinatorAddress();
	void OnXbeeFrameReceived(FrameType type, const std::vector<byte>& payload);
	void TriggerSendCommand(std::string& command);
private:
	friend class IXBeeState;
	void xbee_serial_receive();
	void xbee_api_receive();
	void copyAddress(const byte* start);
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
	virtual const std::string name() = 0;
	virtual void Loop()
	{
		if (timer.Enabled() && timer.Expired())
		{
			//Serial.println("Timeout");
			timer.Stop();
			OnTimerExpired();
		}
	}
	virtual void OnExit() {}
	virtual void OnEnter() {}
	// State machine triggers
	virtual void OnTimerExpired() {}
	virtual void OnSerialLineReceived(String& rxData) {}
	virtual void OnApiRx64FrameReceived(const std::vector<byte>& payload) {}
	virtual void OnModemStatusReceived(ModemStatus state) {}
	virtual void SendCommand(std::string& command) {}

protected:
	XBeeStateMachine& machine;
	static Timer timer;
};

#endif
