#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "NexDome.h"
#include <ArduinoSTL.h>
#include <AdvancedStepper.h>
#include <XBeeApi.h>
#include "CommandProcessor.h"
#include "PersistentSettings.h"
#include "XBeeStartupState.h"
#include "XBeeShutterOnlineState.h"
#include "XBeeApiDetectShutterState.h"


auto stepGenerator = CounterTimer1StepGenerator();
auto settings = PersistentSettings::Load();
auto stepper = MicrosteppingMotor(MOTOR_STEP_PIN, MOTOR_ENABLE_PIN, MOTOR_DIRECTION_PIN, stepGenerator, settings.motor);
auto &xbeeSerial = Serial1;
auto& host = Serial;
const std::vector<String> xbeeInitSequence = { "CE1","ID6FBF","CH0C","MYD0","DH0","DHFFFF","A25","SM0","AP2" };
std::string hostReceiveBuffer;
std::vector<byte> xbeeApiRxBuffer;
void onXbeeFrameReceived(FrameType type, std::vector<byte>& payload);
auto xbeeApi = XBeeApi(xbeeSerial, xbeeApiRxBuffer, (ReceiveHandler)onXbeeFrameReceived);
auto machine = XBeeStateMachine(xbeeSerial, host, xbeeApi);
auto commandProcessor = CommandProcessor(stepper, settings, machine);
Timer periodicTasks;


Response DispatchCommand(const std::string& buffer)
{
	auto charCount = buffer.length();
	if (charCount < 2)
		return Response::Error();
	Command command;
	command.RawCommand = buffer;
	command.StepPosition = 0;
	command.Verb.push_back(buffer[1]);
	if (charCount > 2)
		command.Verb.push_back(buffer[2]);
	// If there is no device address then use '0', the default device.
	if (charCount < 4)
	{
		command.TargetDevice = '0';
		return commandProcessor.HandleCommand(command);
	}
	// Use the device address from the command
	command.TargetDevice = buffer[3];
	// If the parameter was present, then parse it as an integer; otherwise use 0.
	if (charCount > 5 && buffer[4] == ',')
	{
		auto position = buffer.substr(5);
		auto wholeSteps = std::strtoul(position.begin(), NULL, 10);
		command.StepPosition = wholeSteps;
	}
	auto response = commandProcessor.HandleCommand(command);
	return response;
}

void HandleSerialCommunications()
{
	if (host.available() <= 0)
		return;	// No data available.
	auto rx = host.read();
	if (rx < 0)
		return;	// No data available.
	char rxChar = (char)rx;
	switch (rxChar)
	{
	case '\n':	// newline - dispatch the command
	case '\r':	// carriage return - dispatch the command
		if (hostReceiveBuffer.length() > 1)
		{
			hostReceiveBuffer.push_back(rxChar);	// include the EOL in the receive buffer.
			auto response = DispatchCommand(hostReceiveBuffer);
			std::cout << response.Message << std::endl;
			hostReceiveBuffer.clear();
		}
		break;
	case '@':	// Start of new command
		hostReceiveBuffer.clear();
	default:
		if (hostReceiveBuffer.length() < SERIAL_RX_BUFFER_SIZE)
		{
			hostReceiveBuffer.push_back(rxChar);
		}
		break;
	}
}


// the setup function runs once when you press reset or power the board
void setup() {
	stepper.ReleaseMotor();
	hostReceiveBuffer.reserve(SERIAL_RX_BUFFER_SIZE);
	xbeeApiRxBuffer.reserve(API_MAX_FRAME_LENGTH);
	host.begin(115200);
	xbeeSerial.begin(9600);
	while (!Serial);	// Wait for Leonardo software USB stack to become active
	delay(1000);		// Let the USB/serial stack warm up a bit longer.
	xbeeApi.reset();
	periodicTasks.SetDuration(1000);
	interrupts();
	std::cout << "Init" << std::endl;
	machine.ChangeState(new XBeeStartupState(machine));
}

// the loop function runs over and over again until power down or reset
void loop() {
	stepper.Loop();
	HandleSerialCommunications();
	machine.Loop();
	if (periodicTasks.Expired())
	{
		periodicTasks.SetDuration(250);
		if (stepper.CurrentVelocity() != 0.0)
			std::cout << "P" << stepper.CurrentPosition() << std::endl;
	}
}

// Handle the received XBee API frame by passing it to the XBee state machine.
void onXbeeFrameReceived(FrameType type, std::vector<byte>& payload)
{
	machine.OnXbeeFrameReceived(type, payload);
}
