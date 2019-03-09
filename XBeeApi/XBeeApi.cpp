/*
 Name:		XBeeApi.cpp
 Created:	3/9/2019 12:31:46 AM
 Author:	Tim
 Editor:	http://www.visualmicro.com
*/

#include "XBeeApi.h"

XBeeApi::XBeeApi(Stream& channel, std::vector<byte> & buffer, ReceiveHandler onFrameReceivedHandler) 
	: channel(channel), buffer(buffer), onFrameReceived(onFrameReceivedHandler)
{
}

void XBeeApi::loop()
{
	if (channel.available() < 1) return;
	while (channel.available() > 0)
	{
		auto rxi = channel.read();
		if (rxi < 0)
			continue;
		byte rxb = (byte)rxi;
		handleReceivedByte(rxb);
	}
}

/*
Resets the receiver so that it is ready to receive the next frame, aborting any partially received  data.
*/
void XBeeApi::reset()
{
	rxState = WaitFrameStart;
	buffer.clear();
	escapeNextCharacter = false;
	checksum = 0;
	frameLength = 0;
}

uint64_t XBeeApi::GetRemoteAddress(std::vector<byte>& payload)
{
	auto start = payload.begin();
	auto end = start + 8;
	uint64_t address;
	for (auto iterator = start ; iterator != end; iterator++)
	{
		address <<= 8;
		address |= (uint64_t)(*iterator);
	}
	return address;
}

ModemStatus XBeeApi::GetModemStatus(std::vector<byte>& payload)
{
	return ModemStatus(payload[0]);
}

/*
Receive state machine.
Handles escaped data bytes and checksum validation.
*/
void XBeeApi::handleReceivedByte(byte rxb)
{
	// Unescaped frame start character always causes a reset.
	if (rxb == API_FRAME_START)
		reset();

	// If the previous character was the escape marker, then we
	// need to unescape this character.
	if (escapeNextCharacter)
	{
		escapeNextCharacter = false;
		rxb = 0x20U ^ rxb;
	}

	// If we receive an escape marker (except when waiting for frame start)
	// then set a flag that the next character should be unescaped.
	// No further action - escape marker does not affect the checksum.
	if (rxState != WaitFrameStart && rxb == API_ESCAPE)
	{
		escapeNextCharacter = true;
		return;
	}

	// Having fully processed escape sequences and frame markers,
	// all subsequent characters are processed according to the receive state.

	switch (rxState)
	{
	case WaitFrameStart:
		if (rxb == API_FRAME_START)
			rxState = ReceiveLengthMsb;
		break;
	case ReceiveLengthMsb:
		frameLength = ((uint16_t)rxb) << 8;
		rxState = ReceiveLengthLsb;
		break;
	case ReceiveLengthLsb:
		frameLength |= (uint16_t)rxb;
		rxState = ReceiveApiId;
		break;
	case ReceiveApiId:
		frameType = FrameType(rxb);
		checksum = rxb;
		rxState = ReceivePayload;
		break;
	case ReceivePayload:
		buffer.push_back(rxb);
		if (buffer.size() == frameLength)
			rxState = ReceiveChecksum;
		break;
	case ReceiveChecksum:
		checksum += rxb;
		if (checksum == 0xFFu)
		{
			rxState = Complete;
			onFrameReceived(frameType, buffer);
		}
		else
			reset();	// Invalid checksum silently discards the frame
		break;
	default:
		break;
	}
}

