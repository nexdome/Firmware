#include <XBeeApi.h>
#include "XBeeStateMachine.h"

Timer IXBeeState::timer = Timer();

XBeeStateMachine::XBeeStateMachine(HardwareSerial& xBeePort, XBeeApi& xbee)
	: xbeeSerial(xBeePort), xbeeApi(xbee)
	{
	remoteAddress.reserve(8);	// 8 bytes for a 64-bit address
	}

void XBeeStateMachine::Loop()
	{
	if (ApiModeEnabled)
		{
		xbee_api_receive();
		}
	else
		{
		xbee_serial_receive();
		}
	currentState->Loop();
	}

void XBeeStateMachine::sendToLocalXbee(const std::string& message) const
	{
#ifdef DEBUG_XBEE_STATE
	std::cout << "Tx: " << message << std::endl;
#endif
	xbeeSerial.write(message.begin(), message.length());
	}

/*
Sample TX64 frame for Hello message
7E 00 7D 31 00 01 00 00 00 00 00 00 FF FF 00 59 6F 6F 68 6F 6F 83

*/

void XBeeStateMachine::SendToRemoteXbee(const std::string& message)
	{
#ifdef DEBUG_XBEE_STATE
	std::cout << "Send: " << message << std::endl;
#endif
	auto destinationAddress = remoteAddress;
	/* Length =
	API-ID		1
	FrameId		1
	DestAddr	8
	Options		1
	Message		n
	Checksum	0 (doesn't count towards frame length)
	Total = 11 + message.length()
	*/
	const uint16_t length = 11 + message.length();
	xbeeSerial.print(char(API_FRAME_START));
	printEscaped(byte(length >> 8));	// Length MSB
	printEscaped(byte(length));		// Length LSB
	printEscaped(Tx64Request);
	const auto frameId = getNextFrameId();
	printEscaped(frameId);
	byte checksum = Tx64Request + frameId;
	// send the address bytes.
	for (auto index = remoteAddress.begin(); index < remoteAddress.end(); ++index)
		{
		const byte addressByte = *index;
		printEscaped(addressByte);
		checksum += addressByte;
		}
	printEscaped(byte(0));	// No options
	// Now comes the data, up to 100 bytes
	for (auto data = message.begin(); data < message.end(); ++data)
		{
		const byte dataByte = *data;
		printEscaped(dataByte);
		checksum += dataByte;
		}
	// And finally, the checksum.
	printEscaped(char(byte(0xFF) - checksum));
	}

// Send a data byte to the local XBee, inserting an escape sequence if needed.
void XBeeStateMachine::printEscaped(byte data) const
	{
	static const std::vector<byte> escapableBytes = { 0x7E, 0x7D, 0x11, 0x13 };
	for (auto ptr = escapableBytes.begin(); ptr < escapableBytes.end(); ++ptr)
		{
		if (data == *ptr)
			{
			data ^= 0x20;
			xbeeSerial.print(char(API_ESCAPE));
			break;
			}
		}
	xbeeSerial.print(char(data));
	}

void XBeeStateMachine::ChangeState(IXBeeState* newState)
	{
	// The following line is not diagnostics, it's part of the protocol and must not be commented out.
	std::cout << "XB->" << newState->name() << std::endl;
	if (currentState != nullptr)
		{
		currentState->OnExit();
		if (currentState != newState)	// we are possibly re-entering the same state
			{
			delete currentState;
			}
		}
	currentState = newState;
	newState->OnEnter();
	}

void XBeeStateMachine::ListenInAtCommandMode()
	{
	ApiModeEnabled = false;
	xbeeSerial.begin(9600);
	}

void XBeeStateMachine::ListenInApiMode()
	{
	ApiModeEnabled = true;
	xbeeApi.reset();
	}

/*
 * Returns the next frame ID and ensures that it is never zero.
 * Frame ID of zero has special connotations in XBee protocol.
 */
byte XBeeStateMachine::getNextFrameId()
	{
	auto next = ++frameId;
	if (next == 0)
		next = ++frameId;
	return next;
	}

#ifdef DEBUG_XBEE_STATE
void printAddress(const std::vector<byte>& address)
	{
	std::cout << "Set address ";
	for (auto index = address.begin(); index < address.end(); ++index)
		{
		std::cout << std::hex << int(*index) << " ";
		}
	std::cout << std::endl;
	}
#endif

// Extract and save a 64-bit destination address from a frame payload
void XBeeStateMachine::SetDestinationAddress(const std::vector<byte>& payload)
	{
	copyAddress(payload.begin());
#ifdef DEBUG_XBEE_STATE
	printAddress(remoteAddress);
#endif
	}

// Copy 8 bytes (64 bits) of address data starting at (source)
void XBeeStateMachine::copyAddress(const byte* source)
	{
	remoteAddress.clear();
	byte count = 8;	// 64-bit address requires 8 bytes.
	while (count-- > 0)
		remoteAddress.push_back(*source++);
	}

void XBeeStateMachine::useCoordinatorAddress()
	{
	const std::vector<byte> coordinator({ 0,0,0,0,0,0,0xFF,0xFF });
	copyAddress(coordinator.begin());
#ifdef DEBUG_XBEE_STATE
	printAddress(remoteAddress);
#endif
	}

void XBeeStateMachine::xbee_serial_receive() const
	{
	static std::string rxBuffer;
	if (xbeeSerial.available() <= 0)
		return; // No data available
	const auto rx = xbeeSerial.read();
	if (rx < 0)
		return; // Nothing read.
	const auto ch = char(rx);
	if (ch == '\r')
		{
#ifdef DEBUG_XBEE_STATE
		std::cout << "XB Rx " << rxBuffer << std::endl;
#endif
		currentState->OnSerialLineReceived(rxBuffer);
		rxBuffer.clear();
		}
	else
		rxBuffer.push_back(ch);
	}

inline void XBeeStateMachine::xbee_api_receive() const
	{
	xbeeApi.loop();
	}

void XBeeStateMachine::onXbeeFrameReceived(const FrameType type, const std::vector<byte>& payload) const
	{
#ifdef DEBUG_XBEE_STATE
	std::cout << "Rx API ID=" << std::hex << type << ", ";
#endif
	switch (type)
		{
		case ModemStatusResponse:
		{
		const auto status = xbeeApi.GetModemStatus();
#ifdef DEBUG_XBEE_STATE
		std::cout << "Modem Status = " << status << std::endl;
#endif
		currentState->OnModemStatusReceived(status);
		break;
		}
		case Rx64Response:
#ifdef DEBUG_XBEE_STATE
			std::cout << "frame=" << (int)payload[0] << ", payload='" << std::string(payload.begin() + 10, payload.end()) << "'" << std::endl;
#endif
			currentState->OnApiRx64FrameReceived(payload);
			break;
#ifdef DEBUG_XBEE_STATE
		case TxStatusResponse:
			std::cout << "frame=" << (int)payload[0] << " TxStatus=" << (int)payload[1] << std::endl;
			break;
#endif
		default:
#ifdef DEBUG_XBEE_STATE
			std::cout << "unknown" << std::endl;
#endif
			break;
		}
	}

void XBeeStateMachine::TriggerSendCommand(std::string& command)
	{
	currentState->SendCommand(command);
	}


