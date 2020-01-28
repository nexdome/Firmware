// XbeeStateMachine.h

#pragma once
#include <Arduino.h>

#include <ArduinoSTL.h>
#include <XBeeApi.h>
#include <Timer.h>

constexpr Duration XBEE_AT_GUARD_TIME = Timer::Milliseconds(1200);	// Wait time before sending AT attention string and receiving "OK"
constexpr Duration XBEE_AT_COMMAND_TIMEOUT = Timer::Seconds(5);				// Time to wait for "OK" response to AT command before giving up
constexpr Duration XbeeInterAtCommandDelay = Timer::Milliseconds(100);
constexpr Duration XBEE_REMOTE_HANDSHAKE_TIMEOUT = Timer::Seconds(3);		// Maximum time to wait for rotator to acknowledge the hello message.
constexpr Duration XBEE_ASSOCIATE_TIMEOUT = Timer::Seconds(20);				// Maximum time to wait for shutter to associate with a coordinator.
constexpr Duration XBEE_DETECT_SHUTTER_TIMEOUT = Timer::Seconds(60);		// Maximum time to wait for the shutter to say hello.
constexpr Duration XBEE_HEARTBEAT_INTERVAL = Timer::Seconds(8);				// How often the shutter sends a 'heartbeat' message
constexpr Duration XBEE_NO_HEARTBEAT_TIMEOUT = (XBEE_HEARTBEAT_INTERVAL * 2) + Timer::Seconds(1);			// How long to wait for a heartbeat before assuming the link is down
#define XBEE_HELLO_MESSAGE "Yoohoo"	// A retro shout-out to FidoNet era mailer called dBridge.
#define XBEE_HELLO_ACK "2U2"		// (yes I am old enough to remember FidoNet).
#define XBEE_ATTENTION "+++"		// Guard Time + Attention + Guard Time reverts XBee to AT Command Mode

class IXBeeState;

class XBeeStateMachine
{
public:
	XBeeStateMachine(HardwareSerial& xBeePort, XBeeApi& xbee);
	void Loop();
	void ChangeState(IXBeeState* newState);
void StateTransitionIfRequested();
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
	IXBeeState* currentState = nullptr;
	bool ApiModeEnabled = false;
	byte frameId = 0;
	IXBeeState* nextState = nullptr;
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
