/*
 * SafeSerial implementation for Arduino Leonardo.
 * Overrides the Serial_::write() method and checks that there is enough room
 * in the output buffer. If not, the output is discarded.
 * The intent is to prevent serial output from blocking when no host is listening.
 * All other operations are handed off to the underlying Serial_ instance.
 */

#include "SafeSerial.h"

size_t SafeSerial::write(const uint8_t* buffer, size_t size)
	{
	/*
	 * Check if we can write without blocking. If we need to block,
	 * then we assume that the host has disconnected.
	 */

	if (availableForWrite() < size)
		return 0;

	return Serial_::write(buffer, size);
	}

