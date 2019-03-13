#include <XBeeApi.h>
#include "XBeeStateMachine.h"
//#include <Printers.h>

Timer IXBeeState::timer = Timer();

XBeeStateMachine::XBeeStateMachine(HardwareSerial& xBeePort, Stream& debugPort, XBeeApi& xbee) 
	: xbeeSerial(xBeePort), debug(debugPort), xbeeApi(xbee)
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

void XBeeStateMachine::SendToLocalXbee(String message) const
	{
	xbeeSerial.print(message);
	xbeeSerial.flush();
	debug.println(message + " to X");
	}

/*
Sample TX64 frame for Hello message
7E 00 7D 31 00 01 00 00 00 00 00 00 FF FF 00 59 6F 6F 68 6F 6F 83

*/

void XBeeStateMachine::SendToRemoteXbee(const std::string& message)
{
	std::cout << "Send: " << message << std::endl;
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
	uint16_t length = 11 + message.length();
	xbeeSerial.print((char)API_FRAME_START);
	printEscaped((byte)(length >> 8));	// Length MSB
	printEscaped((byte)(length));		// Length LSB
	printEscaped(Tx64Request);
	auto frameId = getNextFrameId();
	printEscaped(frameId);
	byte checksum = Tx64Request + frameId;
	// send the address bytes.
	for (auto index = remoteAddress.begin(); index < remoteAddress.end(); ++index)
	{
		byte addressByte = *index;
		printEscaped(addressByte);
		checksum += addressByte;
	}
	printEscaped((byte)0);	// No options
	// Now comes the data, up to 100 bytes
	for (auto data = message.begin(); data < message.end(); ++data)
	{
		byte dataByte = *data;
		printEscaped(dataByte);
		checksum += dataByte;
	}
	// And finally, the checksum.
	printEscaped((char)((byte)0xFF - checksum));
}

// Send a data byte to the local XBee, inserting an escape sequence if needed.
void XBeeStateMachine::printEscaped(byte data)
{
	static const std::vector<byte> escapableBytes = { 0x7E, 0x7D, 0x11, 0x13 };
	for (auto ptr = escapableBytes.begin(); ptr < escapableBytes.end(); ++ptr)
	{
		if (data == *ptr)
		{
			// Need to escape
			byte escaped = data ^ 0x20;
			data ^= 0x20;
			xbeeSerial.print((char)API_ESCAPE);
			std::cout << std::hex << (int)API_ESCAPE << " " ;
			break;
		}
	}
	xbeeSerial.print((char)data);
	std::cout << std::hex << (int)data << " ";
}

void XBeeStateMachine::ChangeState(IXBeeState* newState)
	{
	debug.println(newState->name());
	if (currentState != NULL)
	{
		currentState->OnExit();
		delete currentState;
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
	xbeeSerial.begin(9600);
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

//ToDo: diagnostics - delete me
void printAddress(const std::vector<byte>& address)
{
	std::cout << "Set address ";
	for (auto index = address.begin(); index < address.end(); ++index)
	{
		std::cout << std::hex << (int)* index << " ";
	}
	std::cout << std::endl;
}

// Extract and save a 64-bit destination address from a frame payload
void XBeeStateMachine::SetDestinationAddress(const std::vector<byte>& payload)
{
	// The source address starts at byte 11 in a payload.
	const byte* source = payload.begin() + 10;
	copyAddress(source);
	printAddress(remoteAddress);
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
	printAddress(remoteAddress);
}

void XBeeStateMachine::xbee_serial_receive()
	{
	static String rxBuffer;
	if (xbeeSerial.available() <= 0)
		return; // No data available
	auto rx = xbeeSerial.read();
	if (rx < 0)
		return; // Nothing read.
	char ch = static_cast<char>(rx);
	debug.println(ch);
	if (ch == 0x0D)
		{
		debug.println(rxBuffer + " from X");
		currentState->OnSerialLineReceived(rxBuffer);
		rxBuffer.remove(0);	// Truncate to empty string
		}
	else
		rxBuffer.concat(ch);
	}

void XBeeStateMachine::xbee_api_receive()
	{
	xbeeApi.loop();
	}

void XBeeStateMachine::OnXbeeFrameReceived(FrameType type, const std::vector<byte>& payload)
{
	//std::cout << "API ID " << (int)type << " frame " << (int)payload[0] << std::endl;
	switch (type)
	{
	case ModemStatusResponse:
	{
		auto status = xbeeApi.GetModemStatus();
		XBeeApi::printModemStatus(status);
		currentState->OnModemStatusReceived(status);
		break;
	}
	case Rx64Response:
		currentState->OnApiRx64FrameReceived(payload);
		break;
	case TxStatusResponse:
		//std::cout << "Frame " << payload[1] << " status " << payload[2] << std::endl;
		break;
	default:
		break;
	}
}


