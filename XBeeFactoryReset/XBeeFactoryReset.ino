/*
 Name:		XBeeFactoryReset.ino
 Created:	3/23/2019 8:25:29 PM
 Author:	Tim
*/

#if defined(ARDUINO) && ARDUINO >= 100
#include "XBeeReadFirmwareVersionState.h"
#include "XBeeWaitForPostConfigureCommandModeState.h"
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <ArduinoSTL.h>
#include <XBeeApi.h>
#include "XBeeStateMachine.h"
#include "XBeeStartupState.h"
#include "../SharedCode/NexDome.h"


auto& xbeeSerial = Serial1;
auto& host = Serial;
std::string hostReceiveBuffer;
std::vector<byte> xbeeApiRxBuffer;
void onXbeeFrameReceived(FrameType type, std::vector<byte>& payload);
auto xbeeApi = XBeeApi(xbeeSerial, xbeeApiRxBuffer, ReceiveHandler(onXbeeFrameReceived));
auto machine = XBeeStateMachine(xbeeSerial, xbeeApi);

// Placeholder method - does nothing.
void onXbeeFrameReceived(FrameType type, std::vector<byte>& payload)
{
}

// the setup function runs once when you press reset or power the board
void setup() 
{
	hostReceiveBuffer.reserve(HOST_SERIAL_RX_BUFFER_SIZE);
	xbeeApiRxBuffer.reserve(API_MAX_FRAME_LENGTH);
	host.begin(115200);
	xbeeSerial.begin(9600);
	while (!Serial);	// Wait for Leonardo software USB stack to become active
	delay(1000);		// Let the USB/serial stack warm up a bit longer.
	xbeeApi.reset();
	interrupts();
	std::cout << "XBee Factory Reset Sketch started" << std::endl;
	machine.ChangeState(new XBeeStartupState(machine));

}

// the loop function runs over and over again until power down or reset
void loop() {
	machine.Loop();
}
