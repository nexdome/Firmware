#include "XBeeStateMachine.h"
//#include <Printers.h>

XBeeStateMachine::XBeeStateMachine(HardwareSerial& xBeePort, Stream& debugPort, XBeeApi& xbee) 
	: xbeeSerial(xBeePort), debug(debugPort), xbeeApi(xbee)
	{
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

void XBeeStateMachine::SendToXbee(String message) const
	{
	xbeeSerial.print(message);
	xbeeSerial.flush();
	debug.println(message + " to X");
	}

void XBeeStateMachine::ChangeState(IXBeeState* newState)
	{
	debug.println(newState->name());
	if (currentState != NULL)
		currentState->OnExit();
	delete currentState;
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
unsigned XBeeStateMachine::GetNextFrameId()
	{
	auto next = ++frameId;
	if (next == 0)
		next = ++frameId;
	return next;
	}


void XBeeStateMachine::SetDestinationAddress(uint64_t address)
	{
	remoteAddress = address;
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
		auto status = XBeeApi::GetModemStatus(payload);
		XBeeApi::printModemStatus(status);
		currentState->OnModemStatusReceived(status);
		break;
	}
	default:
		break;
	}
}


