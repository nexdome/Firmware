#pragma once

#define FIRMWARE_MAJOR_VERSION "0"
#define FIRMWARE_MINOR_VERSION "0"

// Motor Parameters
#define MICROSTEPS_PER_STEP (8) // Should match DIP switches on stepper driver
#define MIN_SPEED (250)         // Minimum speed that can be timed by the hardware timer
#define MIN_RAMP_TIME (100)     // Minimum ramp up/down time in milliseconds

// Motor defaults
#define MOTOR_STEP_PIN (12)
#define MOTOR_DIRECTION_PIN (11)
#define MOTOR_ENABLE_PIN (10)
#define MOTOR_RAMP_TIME (2500) // milliseconds to accelerate to full speed
#define MOTOR_MAX_SPEED (4000 * MICROSTEPS_PER_STEP)
#define MOTOR_DEFAULT_SPEED (3500 * MICROSTEPS_PER_STEP)
#define SHUTTER_FULL_OPEN_DEFAULT (44000UL * MICROSTEPS_PER_STEP)
#define ROTATOR_FULL_REVOLUTION_MICROSTEPS (432000)
#define ROTATOR_MAX_POSITION (ROTATOR_FULL_REVOLUTION_MICROSTEPS * 2UL)
#define ROTATOR_HOME_POSITION (0)

#define SERIAL_RX_BUFFER_SIZE (16) // Receive buffer for PC/USB communications

// Other hardware assignments
#define OPEN_LIMIT_SWITCH_PIN (PIN3)
#define CLOSED_LIMIT_SWITCH_PIN (PIN2)
#define OPEN_BUTTON_PIN (PIN5)
#define CLOSE_BUTTON_PIN (PIN6)
#define CLOCKWISE_BUTTON_PIN (OPEN_BUTTON_PIN)
#define COUNTERCLOCKWISE_BUTTON_PIN (CLOSE_BUTTON_PIN)


