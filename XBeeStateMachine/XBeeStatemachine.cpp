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

void XBeeStateMachine::SendToRemoteXbee(String message)
{
	auto destinationAddress = remoteAddress;
	// Length = API-ID + FrameId + Dest-Address + Options + message + checksum
	uint16_t length = 1 + 1 + 8 + 1 + message.length() + 1;
	xbeeSerial.print((byte)API_FRAME_START);
	xbeeSerial.print((byte)(length >> 8));	// Length MSB
	xbeeSerial.print((byte)(length));		// Length LSB
	xbeeSerial.print(Tx64Request);
	auto frameId = getNextFrameId();
	xbeeSerial.print(frameId);
	byte checksum = Tx64Request + frameId;
	// send the address bytes.
	for (auto index = remoteAddress.begin(); index < remoteAddress.end(); ++index)
	{
		byte addressByte = *index;
		xbeeSerial.print(addressByte);
		checksum += addressByte;
	}
	xbeeSerial.print((byte)0);	// No options
	// Now comes the data, up to 100 bytes
	for (auto data = message.begin(); data < message.end(); ++data)
	{
		byte dataByte = *data;
		xbeeSerial.print(dataByte);
		checksum += dataByte;
	}
	// And finally, the checksum.
	xbeeSerial.print((byte)0xFF - checksum);
}

// Send a data byte to the local XBee, inserting an escape sequence if needed.
void XBeeStateMachine::printEscaped(byte data)
{
	static const std::vector<byte> escapableBytes = { '\x7E', '\x7D', '\x11', '\x13' };
	for (auto ptr = escapableBytes.begin(); ptr < escapableBytes.end(); ++ptr)
	{
		if (data == *ptr)
		{
			// Need to escape
			xbeeSerial.print((byte)API_ESCAPE);
			xbeeSerial.print(data ^ 0x20);
			return;
		}
		// Non-escapable
		xbeeSerial.print(data);
	}
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

// Extract and save a 64-bit destination address from a frame payload
void XBeeStateMachine::SetDestinationAddress(std::vector<byte>& payload)
{
	// vectors are gauranteed to be in contiguous storage
	byte* source = payload.begin();
	byte* dest = remoteAddress.begin();
	byte count = 8;	// 64-bit address requires 8 bytes.
	while (count-- > 0)
		* dest++ = *source++;
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

void XBeeStateMachine::XBeeApiSendMessage(const String& message)
	{
	debug.print("Tx: ");
	debug.println(message);
	//tx64Frame.setAddress64(remoteAddress);
	//tx64Frame.setFrameId(GetNextFrameId());
	//tx64Frame.setPayload((uint8_t*)message.begin());
	//SendXbeeApiFrame(tx64Frame);
	}

void XBeeStateMachine::OnXbeeFrameReceived(FrameType type, std::vector<byte>& payload)
{
	switch (type)
	{
	case ModemStatusResponse:
	{
		auto status = xbeeApi.GetModemStatus();
		XBeeApi::printModemStatus(status);
		currentState->OnModemStatusReceived(status);
		break;
	}
	default:
		break;
	}
}


