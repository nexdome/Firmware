#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <ArduinoSTL.h>
#include <AdvancedStepper.h>
#include <XBeeApi.h>
#include "NexDome.h"
#include "XBeeStateMachine.h"
#include "XBeeStartupState.h"
#include "XBeeOnlineState.h"
#include "CommandProcessor.h"
#include "PersistentSettings.h"

auto stepGenerator = CounterTimer1StepGenerator();
auto settings = PersistentSettings::Load();
auto stepper = MicrosteppingMotor(M1_STEP_PIN, M1_ENABLE_PIN, M1_DIRECTION_PIN, stepGenerator, settings.motor);
auto commandProcessor = CommandProcessor(stepper, settings);
auto& xbeeSerial = Serial1;
auto& host = Serial;
std::vector<byte> receiveBuffer;
void HandleFrameRecveived(FrameType type, std::vector<byte> payload);	// forward reference

auto xbee = XBeeApi(xbeeSerial, receiveBuffer, (ReceiveHandler) HandleFrameRecveived);
auto machine = XBeeStateMachine(xbeeSerial, host, xbee);

void HandleFrameRecveived(FrameType type, std::vector<byte> payload)
{
	machine.OnXbeeFrameReceived(type, payload);
}

void HandleSerialCommunications()
{
	static char rxBuffer[SERIAL_RX_BUFFER_SIZE];
	static unsigned int rxIndex = 0;

	if (host.available() <= 0)
		return;	// No data available.
	auto rx = host.read();
	if (rx < 0)
		return;	// No data available.
	char rxChar = (char)rx;
	switch (rx)
	{
	case '@':	// Start of new command
		rxIndex = 0;
		break;
	case '\n':	// newline - dispatch the command
	case '\r':	// carriage return - dispatch the command
		if (rxIndex > 0)
		{
			auto response = DispatchCommand(rxBuffer, rxIndex);
			host.println(response.Message);
		}
		rxIndex = 0;
		break;
	default:	// collect received characters into the command buffer
		if (rxIndex < (SERIAL_RX_BUFFER_SIZE - 1))	// Allow room for null terminator
		{
			rxBuffer[rxIndex++] = rxChar;
			rxBuffer[rxIndex] = '\0';	// Ensure that the buffer is always null-terminated.
		}
		break;
	}
}

Response DispatchCommand(Command& command)
{
	auto response = commandProcessor.HandleCommand(command);
	return response;
}

Response DispatchCommand(String verb, char targetDevice, int32_t stepPosition)
{
	//BUG: suspected faulty initializer, copy the ovserload below.
	//auto command = Command{ verb,targetDevice,stepPosition };
	Command command;
	command.StepPosition = stepPosition;
	command.TargetDevice = targetDevice;
	command.Verb = verb;
	return commandProcessor.HandleCommand(command);
}

Response DispatchCommand(char* buffer, unsigned int charCount)
{
	if (charCount < 1)
		return Response::Error();
	Command command;
	command.StepPosition = 0;
	command.Verb.concat(buffer[0]);
	if (charCount > 1)
		command.Verb.concat(buffer[1]);
	// If there is no device address then use '0', the default device.
	if (charCount < 3)
	{
		command.TargetDevice = '0';
		return commandProcessor.HandleCommand(command);
	}
	// Use the device address from the command
	command.TargetDevice = buffer[2];
	// If the parameter was present, then parse it as an integer; otherwise use 0.
	if (charCount > 4 && buffer[3] == ',')
	{
		auto wholeSteps = std::strtoul(buffer + 4, NULL, 10);
		command.StepPosition = wholeSteps;
	}
	auto response = commandProcessor.HandleCommand(command);
	return response;
}

// the setup function runs once when you press reset or power the board
void setup() {
	stepper.ReleaseMotor();
	host.begin(115200);
	xbeeSerial.begin(9600);
	interrupts();

	machine.ChangeState(new XBeeStartupState(machine));
}

// the loop function runs over and over again until power down or reset
void loop() {
	if (millis() % 200 != 0) return;
	stepper.Loop();
	HandleSerialCommunications();
	machine.Loop();
	while (millis() % 200 == 0);
}
