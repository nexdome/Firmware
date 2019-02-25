#include "XBeeStateMachine.h"
#include <Printers.h>

XBeeStateMachine::XBeeStateMachine(HardwareSerial& xBeePort, Stream& debugPort) : xbeeSerial(xBeePort), debug(debugPort)
	{
	xbeeApi = XBeeWithCallbacks();
	xbeeApi.setSerial(xbeeSerial);
	auto xbeePrint = (uintptr_t)(Print*)& Serial;
	// Make sure that any errors are logged to Serial. The address of
    // Serial is first cast to Print*, since that's what the callback
    // expects, and then to uintptr_t to fit it inside the data parameter.
	xbeeApi.onPacketError(printErrorCb, xbeePrint);
	xbeeApi.onTxStatusResponse(printErrorCb, xbeePrint);
	xbeeApi.onZBTxStatusResponse(printErrorCb, xbeePrint);

	// These are called when an actual packet received
	//xbeeApi.onZBRxResponse(zbReceive);
	//xbeeApi.onRx16Response(receive16);
	//xbeeApi.onRx64Response(receive64);

	// Print any unhandled response with proper formatting
	xbeeApi.onOtherResponse(printResponseCb, xbeePrint);

	}

void XBeeStateMachine::Loop()
	{
	if (ApiModeEnabled)
		{
		xbeeApi.loop();
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

void XBeeStateMachine::SetDestinationAddress(uint64_t address)
	{
		remoteAddress.set(address);
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

union rxResponses
	{
	Rx64Response rx64;
	Rx16Response rx16;
	};

void XBeeStateMachine::xbee_api_receive()
	{
	// Reusable response objects for responses we expect to handle.
	static auto xbeeResponse = XBeeResponse();
	static rxResponses response = {Rx64Response()};

	xbeeApi.readPacket();
	if (xbeeApi.getResponse().isAvailable())
		{
		auto frameType = xbeeApi.getResponse().getApiId();
		// got something
		debug.print("xf ");
		debug.println(frameType);

		if (frameType == RX_16_RESPONSE || frameType == RX_64_RESPONSE)
			{
			// got a rx packet

			if (frameType == RX_16_RESPONSE)
				{
				xbeeApi.getResponse().getRx16Response(response.rx16);
				}
			else
				{
				xbeeApi.getResponse().getRx64Response(response.rx64);
				currentState->OnApiRx64FrameReceived(response.rx64);
				}
			}
		}
	}

void XBeeStateMachine::XBeeApiSendMessage(const String& message)
	{
	messageTx.setAddress64(remoteAddress);
	messageTx.setFrameId(GetNextFrameId());
	messageTx.setPayload((uint8_t*)message.begin());
	SendXbeeApiFrame(messageTx);
	}

