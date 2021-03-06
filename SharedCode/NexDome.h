﻿#ifndef NEXDOME_H
#define NEXDOME_H

#include <Arduino.h>

/*
 * limits.h appears to have values that are not consistent
 * with reality.
 */

constexpr int32_t MaxStepPosition = 2000000000L;
constexpr int32_t MinStepPosition = -2000000000L;

// Motor Parameters
#define MICROSTEPS_PER_STEP (8) // Should match DIP switches on stepper driver
#define MIN_SPEED (250)         // Minimum speed that can be timed by the hardware timer
#define MIN_RAMP_TIME (100)     // Minimum ramp up/down time in milliseconds

// Motor defaults
#define MOTOR_STEP_PIN (12)
#define MOTOR_DIRECTION_PIN (11)
#define MOTOR_ENABLE_PIN (10)
#define MOTOR_RAMP_TIME (1500) // milliseconds to accelerate to full speed
#define MOTOR_MAX_SPEED (25000 * MICROSTEPS_PER_STEP)
#define ROTATOR_DEFAULT_SPEED (600 * MICROSTEPS_PER_STEP)
#define SHUTTER_DEFAULT_SPEED (800 * MICROSTEPS_PER_STEP)
#define SHUTTER_FULL_OPEN_DEFAULT (46000UL * MICROSTEPS_PER_STEP)
#define SHUTTER_LIMIT_STOPPING_DISTANCE (100 * MICROSTEPS_PER_STEP)
#define ROTATOR_FULL_REVOLUTION_MICROSTEPS (440640)
#define ROTATOR_MAX_POSITION (MaxStepPosition)
#define ROTATOR_HOME_POSITION (0)
#define ROTATOR_DEFAULT_DEADZONE (75 * MICROSTEPS_PER_STEP)	// default dead-zone in microsteps (~0.5°)

#define HOST_SERIAL_RX_BUFFER_SIZE (16) // Receive buffer for PC/USB communications

// Other hardware assignments
#define HOME_INDEX_PIN (PIN2)
#define OPEN_LIMIT_SWITCH_PIN (PIN3)
#define CLOSED_LIMIT_SWITCH_PIN (PIN2)
#define OPEN_BUTTON_PIN (PIN5)
#define CLOSE_BUTTON_PIN (PIN6)
#define CLOCKWISE_BUTTON_PIN (OPEN_BUTTON_PIN)
#define COUNTERCLOCKWISE_BUTTON_PIN (CLOSE_BUTTON_PIN)
#define RAIN_SENSOR_PIN (PIN7)

#endif // NEXDOME_H
