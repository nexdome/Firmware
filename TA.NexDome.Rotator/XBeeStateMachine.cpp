#include "XBeeStateMachine.h"

XBeeStateMachine::XBeeStateMachine(HardwareSerial& port) : xbeeSerial(port)
	{
	xbeeApi = XBee();
	xbeeApi.setSerial(xbeeSerial);
	}

void XBeeStateMachine::Loop()
	{
	if (ApiModeEnabled) {}
	else
		{
		xbee_serial_receive();
		}
	currentState->Loop();
	}

void XBeeStateMachine::ChangeState(IXBeeState* newState)
	{
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
	xbeeSerial.begin(115200);
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
	if (ch == '\n')
	{
		currentState->OnSerialLineReceived(rxBuffer);
		rxBuffer = String();
	}
	else
		rxBuffer.concat(ch);
	}

void XBeeStateMachine::xbee_api_receive()
	{
	xbee.readPacket();

	if (xbee.getResponse().isAvailable()) {
		// got something

		if (xbee.getResponse().getApiId() == RX_16_RESPONSE || xbee.getResponse().getApiId() == RX_64_RESPONSE) {
			// got a rx packet

			if (xbee.getResponse().getApiId() == RX_16_RESPONSE) {
				xbee.getResponse().getRx16Response(rx16);
				option = rx16.getOption();
				data = rx16.getData(0);
			}
			else {
				xbee.getResponse().getRx64Response(rx64);
				option = rx64.getOption();
				data = rx64.getData(0);
			}
	}

IXBeeState::~IXBeeState() = default;
