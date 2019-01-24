/*
 Name:		TA.ino
 Created:	1/23/2019 10:16:41 PM
 Author:	Tim
*/

#include <ArduinoSTL.h>
#include <AdvancedStepper.h>

auto stepGenerator = CounterTimer1StepGenerator();
auto motorSettings = MotorSettings();
//auto settings = PersistentSettings::Load();
auto rotatorMotor = (1, 2, 3, stepGenerator, motorSettings);

// the setup function runs once when you press reset or power the board
void setup() {

}

// the loop function runs over and over again until power down or reset
void loop() {
  
}
