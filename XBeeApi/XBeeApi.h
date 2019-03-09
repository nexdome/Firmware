/*
 Name:		XBeeApi.h
 Created:	3/9/2019 12:31:46 AM
 Author:	Tim Long, Tigra Astronomy
 Editor:	http://www.visualmicro.com

 This is a minimal implementation of XBee API mode 2 protocol.
 Only the functionality needed for the NexDome project has been implemented.

 The receiver state machine handles escaped characters and checksum validation.

 The client must register a handler method which will be called when a valid frame is received.
*/

#ifndef _XBeeApi_h
#define _XBeeApi_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <ArduinoSTL.h>

#define API_ESCAPE				(0x7DU)
#define API_FRAME_START			(0x7EU)
#define API_MAX_FRAME_LENGTH	(32)		// This is not the actual XBee max frame length

enum FrameType : byte
{
	Tx64Request = 0x00,
	Tx16Request = 0x01,
	AtCommandRequest = 0x08,
	AtCommandQueueRequest = 0x09,
	RemoteAtCommandRequest = 0x17,
	ZbTxRequest = 0x10,
	ZbExplicitTxRequest = 0x11,
	Rx64Response = 0x80,
	Rx16Response = 0x81,
	Rx64IoResponse = 0x82,
	Rx16IoResponse = 0x83,
	AtResponse = 0x88,
	TxStatusResponse = 0x89,
	ModemStatusResponse = 0x8a,
	ZbRxResponse = 0x90,
	ZbExplicitRxResponse = 0x91,
	ZbTxStatusResponse = 0x8b,
	ZbIoSampleResponse = 0x92,
	ZbIoNodeIdentifierResponse = 0x95,
	AtCommandResponse = 0x88,
	RemoteAtCommandResponse = 0x97,
};

enum RxState
{
	WaitFrameStart,
	ReceiveLengthMsb,
	ReceiveLengthLsb,
	ReceiveApiId,
	ReceivePayload,
	ReceiveChecksum,
	Complete
};

enum ModemStatus
{
	HardwareReset=0,
	WatchdogTimerReset=1,
	Associated=2,
	Disassociated=3,
	SynchronizationLost=4,
	CoordinatorRealignment=5,
	CoordinatorStarted=6
};

typedef int (*ReceiveHandler) (FrameType frameType, std::vector<byte>& payload) ;

class XBeeApi
	{
public:
	explicit XBeeApi(Stream& channel, std::vector<byte> & buffer, ReceiveHandler onFrameReceived);
	void loop();
	void reset();
	static uint64_t GetRemoteAddress(std::vector<byte>& payload);
	static ModemStatus GetModemStatus(std::vector<byte>& payload);
private:
	Stream& channel;
	std::vector<byte>& buffer;
	void handleReceivedByte(byte rxb);
	RxState rxState;
	bool escapeNextCharacter;
	byte checksum;
	uint16_t frameLength;
	FrameType frameType;
	ReceiveHandler onFrameReceived;

	};

#endif

