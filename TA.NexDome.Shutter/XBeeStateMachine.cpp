#include "XBeeStateMachine.h"

XBeeStateMachine::XBeeStateMachine(HardwareSerial& xBeePort, Stream& debugPort) : xbeeSerial(xBeePort), debug(debugPort)
	{
	xbeeApi = XBee();
	xbeeApi.setSerial(xbeeSerial);
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
	debug.println(message + " to X");
	xbeeSerial.print(message);
	}

void XBeeStateMachine::ChangeState(IXBeeState* newState)
	{
	debug.println(newState->name());
	if (currentState != NULL)
		currentState->OnExit();
	newState->OnEnter();
	delete currentState;
	currentState = newState;
	}

void XBeeStateMachine::ListenInAtCommandMode()
	{
	ApiModeEnabled = false;
	}

void XBeeStateMachine::ListenInApiMode()
	{
	ApiModeEnabled = true;
	//xbeeSerial.begin(115200);
	}

void XBeeStateMachine::SetShutterAddress(const XBeeAddress64& remote)
	{
	shutterAddress = XBeeAddress64(remote);
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

void XBeeStateMachine::SendXbeeApiFrame(XBeeRequest& request)
	{
	xbeeApi.send(request);
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
		rxBuffer = String();
		}
	else
		rxBuffer.concat(ch);
	}

void XBeeStateMachine::xbee_api_receive()
	{
	xbeeApi.readPacket();
	if (xbeeApi.getResponse().isAvailable())
		{
		// got something
		auto frameType = xbeeApi.getResponse().getApiId();
		debug.print("xf ");
		debug.println(frameType);
		switch (frameType)
			{
		case RX_64_RESPONSE:
			Rx64Response rx64Response;
			xbeeApi.getResponse().getRx64Response(rx64Response);
			currentState->OnApiRx64FrameReceived(rx64Response);
			break;
		case MODEM_STATUS_RESPONSE:
			ModemStatusResponse response;
			xbeeApi.getResponse().getModemStatusResponse(response);
			auto status = response.getStatus();
			currentState->OnModemStatusReceived(status);
			break;
		default:
			debug.println("ignore frame");
			}
		}
	}
