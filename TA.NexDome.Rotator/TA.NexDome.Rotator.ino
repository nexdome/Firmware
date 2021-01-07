#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <ArduinoSTL.h>
#include <SafeSerial.h>
#include <AdvancedStepper.h>
#include <XBeeApi.h>
#include <Timer.h>
#include "RainSensor.h"
#include "NexDome.h"
#include "PersistentSettings.h"
#include "HomeSensor.h"
#include "CommandProcessor.h"
#include "XBeeStartupState.h"

constexpr Duration SerialInactivityTimeout = Timer::Minutes(10);

// Forward declarations
void onXbeeFrameReceived(FrameType type, std::vector<byte> &payload);
void onMotorStopped();

// Global scope data
auto stepGenerator = CounterTimer1StepGenerator();
auto settings = PersistentSettings::Load();
auto stepper = MicrosteppingMotor(MOTOR_STEP_PIN, MOTOR_ENABLE_PIN, MOTOR_DIRECTION_PIN, stepGenerator, settings.motor);
auto &xbeeSerial = Serial1;
SafeSerial host;
std::string hostReceiveBuffer;
std::vector<byte> xbeeApiRxBuffer;
auto xbeeApi = XBeeApi(xbeeSerial, xbeeApiRxBuffer, ReceiveHandler(onXbeeFrameReceived));
auto machine = XBeeStateMachine(xbeeSerial, xbeeApi);
auto commandProcessor = CommandProcessor(stepper, settings, machine);
auto home = HomeSensor(&stepper, &settings.home, HOME_INDEX_PIN, commandProcessor);
Timer periodicTasks;
Timer serialInactivityTimer;
auto rain = RainSensor(RAIN_SENSOR_PIN);

// cin and cout for ArduinoSTL
namespace std
{
ohserialstream cout(host);
ihserialstream cin(host);
} // namespace std

void DispatchCommand(const Command& command)
	{
	//std::cout << command.RawCommand << "V=" << command.Verb << ", T=" << command.TargetDevice << ", P=" << command.StepPosition << std::endl;
	commandProcessor.HandleCommand(command);
	}

/*
 * Handles receive data from the host serial interface.
 * Attempts to receive whole commands delimited by @ and \r and/or \n
 * and passes the command to DispatchCommand.
 */
void HandleSerialCommunications()
{
	if (!host || host.available() <= 0)
		return; // No data available.
	const auto rx = host.read();
	if (rx < 0)
		return; // No data available.

	serialInactivityTimer.SetDuration(SerialInactivityTimeout);
	const char rxChar = char(rx);
	switch (rxChar)
	{
	case '\n': // newline - dispatch the command
	case '\r': // carriage return - dispatch the command
		if (hostReceiveBuffer.length() > 1)
			{
			const auto command = Command(hostReceiveBuffer);
			DispatchCommand(command);
			hostReceiveBuffer.clear();
			if (ResponseBuilder::available())
				std::cout
					<< ResponseBuilder::header
					<< ResponseBuilder::Message
					<< ResponseBuilder::terminator
					<< std::endl; // send response, if there is one.
			}
		break;
	case '@': // Start of new command
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
void setup()
{
	stepper.releaseMotor();
	stepper.registerStopHandler(onMotorStopped);
	pinMode(CLOCKWISE_BUTTON_PIN, INPUT_PULLUP);
	pinMode(COUNTERCLOCKWISE_BUTTON_PIN, INPUT_PULLUP);
	hostReceiveBuffer.reserve(HOST_SERIAL_RX_BUFFER_SIZE);
	xbeeApiRxBuffer.reserve(API_MAX_FRAME_LENGTH);
	host.begin(115200);
	// Connect cin and cout to our SafeSerial instance
	ArduinoSTL_Serial.connect(host);
	xbeeSerial.begin(9600);
	delay(1000); // Let the USB/serial stack warm up a bit longer.
	xbeeApi.reset();
	periodicTasks.SetDuration(1000);
	HomeSensor::init();
	rain.init(Timer::Seconds(30));
	pinMode(LED_BUILTIN, OUTPUT);
	interrupts();
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

void heartbeat()
{
	static bool state = false;
	state = !state;
	digitalWrite(LED_BUILTIN, state ? HIGH : LOW);
}

// the loop function runs over and over again until power down or reset
void loop()
{
	stepper.loop();
	HandleSerialCommunications();
	machine.Loop();
	if (periodicTasks.Expired())
	{
		periodicTasks.SetDuration(250);
		heartbeat();
		if (stepper.isMoving())
			std::cout << "P" << std::dec << commandProcessor.getPositionInWholeSteps() << std::endl;
		ProcessManualControls();
		rain.loop();
		// Release stepper holding torque if there has been no serial communication for "a long time".
		if (serialInactivityTimer.Expired())
			stepper.releaseMotor();
	}
}

// Handle the received XBee API frame by passing it to the XBee state machine.
void onXbeeFrameReceived(FrameType type, std::vector<byte> &payload)
{
	machine.onXbeeFrameReceived(type, payload);
}

// Handle the motor stop event from the stepper driver.
void onMotorStopped()
{
	//std::cout << "STOP" << std::endl;
	// First, "normalize" the step position
	settings.motor.currentPosition = commandProcessor.getNormalizedPositionInMicrosteps();
	home.onMotorStopped();
	commandProcessor.sendStatus();
}
