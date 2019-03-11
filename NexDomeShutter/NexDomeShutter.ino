/*
    Name:       NexDomeShutter.ino
    Created:	10/03/2019 22:21:06
    Author:     TYCO\Tim 
*/

#include <ArduinoSTL.h>
#include <XBeeStateMachine.h>
#include "XBeeStartupState.h"
void onFrameReceived(FrameType type, std::vector<byte>& payload);
auto rxBuffer = std::vector<byte>();
auto xbeeApi = XBeeApi(Serial1, rxBuffer, (ReceiveHandler)onFrameReceived);
auto machine = XBeeStateMachine(Serial1, Serial, xbeeApi);
void onFrameReceived(FrameType type, std::vector<byte>& payload)
{
	Serial.println("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
	Serial.print("<< Handling frame type: ");
	Serial.println(type);
	Serial.println("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
	machine.OnXbeeFrameReceived(type, payload);
}

void setup()
{
	rxBuffer.reserve(300);
	Serial.begin(115200);
	Serial1.begin(9600);
	machine.ChangeState(new XBeeStartupState(machine));
}

void loop()
{
	machine.Loop();
}
