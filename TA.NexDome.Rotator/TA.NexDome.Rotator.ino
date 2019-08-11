#if defined(ARDUINO) && ARDUINO >= 100
#include "RainSensor.h"
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <ArduinoSTL.h>
#include <AdvancedStepper.h>
#include <XBeeApi.h>
#include "NexDome.h"
#include "PersistentSettings.h"
#include "HomeSensor.h"
#include "CommandProcessor.h"
#include "XBeeStartupState.h"

// Forward declarations
void onXbeeFrameReceived(FrameType type, std::vector<byte> & payload);
void onMotorStopped();

// Global scope data
auto stepGenerator = CounterTimer1StepGenerator();
auto settings = PersistentSettings::Load();
auto stepper = MicrosteppingMotor(MOTOR_STEP_PIN, MOTOR_ENABLE_PIN, MOTOR_DIRECTION_PIN, stepGenerator, settings.motor);
auto& xbeeSerial = Serial1;
auto& host = Serial;
std::string hostReceiveBuffer;
std::vector<byte> xbeeApiRxBuffer;
auto xbeeApi = XBeeApi(xbeeSerial, xbeeApiRxBuffer, ReceiveHandler(onXbeeFrameReceived));
auto machine = XBeeStateMachine(xbeeSerial, xbeeApi);
auto commandProcessor = CommandProcessor(stepper, settings, machine);
auto home = HomeSensor(&stepper, &settings.home, HOME_INDEX_PIN, commandProcessor);
Timer periodicTasks;
auto rain = RainSensor(RAIN_SENSOR_PIN);

Response DispatchCommand(const std::string& buffer)
	{
	const auto charCount = buffer.length();
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
		const auto wholeSteps = std::strtoul(position.begin(), NULL, 10);
		command.StepPosition = wholeSteps;
		}
	auto response = commandProcessor.HandleCommand(command);
	return response;
	}

/*
 * Handles receive data from the host serial interface.
 * Attempts to receive whole commands delimited by @ and \r and/or \n
 * and passes the command to DispatchCommand.
 */
void HandleSerialCommunications()
	{
	if (host.available() <= 0)
		return;	// No data available.
	const auto rx = host.read();
	if (rx < 0)
		return;	// No data available.
	const char rxChar = char(rx);
	switch (rxChar)
		{
		case '\n':	// newline - dispatch the command
		case '\r':	// carriage return - dispatch the command
			if (hostReceiveBuffer.length() > 1)
				{
				hostReceiveBuffer.push_back(rxChar);	// include the EOL in the receive buffer.
				const auto response = DispatchCommand(hostReceiveBuffer);
				std::cout << response;	// send a fully formatted response, or nothing if there is no response.
				hostReceiveBuffer.clear();
				}
			break;
		case '@':	// Start of new command
			hostReceiveBuffer.clear();
		default:
			if (hostReceiveBuffer.length() < HOST_SERIAL_RX_BUFFER_SIZE)
				{
				hostReceiveBuffer.push_back(rxChar);
				}
			break;
		}
	}

// the setup function runs once when you press reset or power the board
void setup() {
	stepper.releaseMotor();
	stepper.registerStopHandler(onMotorStopped);
	pinMode(CLOCKWISE_BUTTON_PIN, INPUT_PULLUP);
	pinMode(COUNTERCLOCKWISE_BUTTON_PIN, INPUT_PULLUP);
	hostReceiveBuffer.reserve(HOST_SERIAL_RX_BUFFER_SIZE);
	xbeeApiRxBuffer.reserve(API_MAX_FRAME_LENGTH);
	host.begin(115200);
	xbeeSerial.begin(9600);
	//while (!Serial);	// Wait for Leonardo software USB stack to become active
	delay(1000);		// Let the USB/serial stack warm up a bit longer.
	xbeeApi.reset();
	periodicTasks.SetDuration(1000);
	HomeSensor::init();
	rain.init(Timer::Seconds(30));
	interrupts();
	std::cout << F("Init") << std::endl;
	machine.ChangeState(new XBeeStartupState(machine));
	}

void ProcessManualControls()
	{
	static bool clockwiseButtonLastState = false;
	static bool counterclockwiseButtonLastState = false;
	const bool clockwiseButtonPressed = digitalRead(CLOCKWISE_BUTTON_PIN) == 0;
	const bool clockwiseButtonChanged = clockwiseButtonPressed != clockwiseButtonLastState;
	const auto position = stepper.getCurrentPosition();
	if (clockwiseButtonChanged && clockwiseButtonPressed)
		{
		CommandProcessor::sendDirection(+1);
		stepper.moveToPosition(position + settings.home.microstepsPerRotation);
		}
	if (clockwiseButtonChanged && !clockwiseButtonPressed)
		{
		stepper.SoftStop();
		}
	clockwiseButtonLastState = clockwiseButtonPressed;
	const bool counterclockwiseButtonPressed = digitalRead(COUNTERCLOCKWISE_BUTTON_PIN) == 0;
	const bool counterclockwiseButtonChanged = counterclockwiseButtonPressed != counterclockwiseButtonLastState;
	if (counterclockwiseButtonChanged && counterclockwiseButtonPressed)
		{
		CommandProcessor::sendDirection(-1);
		stepper.moveToPosition(position - settings.home.microstepsPerRotation);
		}
	if (counterclockwiseButtonChanged && !counterclockwiseButtonPressed)
		{
		stepper.SoftStop();
		}
	counterclockwiseButtonLastState = counterclockwiseButtonPressed;
	}

// the loop function runs over and over again until power down or reset
void loop() {
	stepper.loop();
	HandleSerialCommunications();
	machine.Loop();
	if (periodicTasks.Expired())
		{
		periodicTasks.SetDuration(250);
		if (stepper.isMoving())
			std::cout << "P" << std::dec << commandProcessor.getPositionInWholeSteps() << std::endl;
		ProcessManualControls();
		rain.loop();
		}
	}

// Handle the received XBee API frame by passing it to the XBee state machine.
void onXbeeFrameReceived(FrameType type, std::vector<byte>& payload)
	{
	machine.onXbeeFrameReceived(type, payload);
	}

// Handle the motor stop event from the stepper driver.
void onMotorStopped()
	{
	settings.motor.currentPosition = commandProcessor.getNormalizedPositionInMicrosteps();
	commandProcessor.sendStatus();
	home.onMotorStopped();
	}
