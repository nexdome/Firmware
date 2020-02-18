/*
 * SafeSerial - only writes data to a serial stream if there is enough space in the output buffer.
 * Otherwise the data is discarded. This hopefully ensures that serial I/O is always non-blocking.
 * Obviously there is a risk of data loss so that needs to be considered before using this class.
 */

#pragma once
#include <Arduino.h>

class SafeSerial : public Serial_
	{
	size_t write(const uint8_t* buffer, size_t size) override;
	};