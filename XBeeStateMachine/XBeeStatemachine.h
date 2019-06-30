// XbeeStateMachine.h

#ifndef _XBEESTATE_h
#define _XBEESTATE_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <ArduinoSTL.h>
#include <XBeeApi.h>
#include <Timer.h>


#define XBEE_BOOT_TIME_MILLIS (5000UL)			// Time for XBEE to become ready from cold reset
#define XBEE_AT_GUARD_TIME (1500UL)				// Wait time before sending AT attention string and receiving "OK"
#define XBEE_AT_COMMAND_TIMEOUT (5000UL)		// Time to wait for "OK" response to AT command before giving up
#define XBEE_REMOTE_HANDSHAKE_TIMEOUT (5000UL)	// Maximum time to wait for rotator to acknowledge the hello message.
#define XBEE_ASSOCIATE_TIMEOUT (20000UL)		// Maximum time to wait for shutter to associate with a coordinator.
#define XBEE_DETECT_SHUTTER_TIMEOUT (40000UL)	// Maximum time to wait for the shutter to say hello.
#define XBEE_HEARTBEAT_INTERVAL (10000UL)		// How often the shutter sends a 'heartbeat' message
#define XBEE_NO_HEARTBEAT_TIMEOUT (	30000UL)	// How long to wait for a heartbeat before assuming the link is down
#define XBEE_HELLO_MESSAGE "Yoohoo"				// A retro shout-out to FidoNet era mailer called dBridge.
#define XBEE_HELLO_ACK "2U2"				// (yes I am old enough to remember FidoNet).
#define XBEE_ATTENTION "+++"					// Guard Time + Attention + Guard Time reverts XBee to AT Command Mode

class IXBeeState;

class XBeeStateMachine
{
public:
	XBeeStateMachine(HardwareSerial& xBeePort, XBeeApi& xbee);
	void Loop();
	void ChangeState(IXBeeState* newState);
	void ListenInAtCommandMode();
	void ListenInApiMode();
	void sendToLocalXbee(const std::string& message) const;
	void SendToRemoteXbee(const std::string& message);
	void SetDestinationAddress(const std::vector<byte>& payload);
	void useCoordinatorAddress();
	void onXbeeFrameReceived(FrameType type, const std::vector<byte>& payload) const;
	void TriggerSendCommand(std::string& command);
private:
	friend class IXBeeState;
	void xbee_serial_receive() const;
	void xbee_api_receive() const;
	void copyAddress(const byte* source);
	void printEscaped(byte data) const;
	byte getNextFrameId();
	HardwareSerial& xbeeSerial;
	XBeeApi& xbeeApi;
	std::vector<byte> remoteAddress;
	IXBeeState* currentState;
	bool ApiModeEnabled = false;
	byte frameId = 0;
};

class IXBeeState
{
public:
	// State machine "plumbing"
	explicit IXBeeState(XBeeStateMachine& machine) : machine(machine) {}
	virtual ~IXBeeState() = default;
	virtual std::string name() = 0;
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
	virtual void OnSerialLineReceived(const std::string& message) {}
	virtual void OnApiRx64FrameReceived(const std::vector<byte>& payload) {}
	virtual void OnModemStatusReceived(ModemStatus state) {}
	virtual void SendCommand(std::string& command) {}

protected:
	virtual void OnTimerExpired() {}
	XBeeStateMachine& machine;
	static Timer timer;
};

#endif
