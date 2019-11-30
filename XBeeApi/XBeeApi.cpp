/*
 Name:		XBeeApi.cpp
 Created:	3/9/2019 12:31:46 AM
 Author:	Tim
 Editor:	http://www.visualmicro.com
*/

#include "XBeeApi.h"

XBeeApi::XBeeApi(Stream& channel, std::vector<byte>& buffer, ReceiveHandler onFrameReceivedHandler)
	: channel(channel), buffer(buffer), onFrameReceived(onFrameReceivedHandler)
	{
	}

void XBeeApi::loop()
	{
	if (channel.available() < 1) return;
	while (channel.available() > 0)
		{
		const auto rxi = channel.read();
		if (rxi < 0)
			continue;
		const auto rxb = byte(rxi);
#ifdef DEBUG_XBEE_API
		std::cout << std::hex << (int)rxb << " ";
#endif
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
	frameType = FrameType(-1);
	}

uint64_t XBeeApi::GetRemoteAddress(std::vector<byte>& payload)
	{
	const auto start = payload.begin();
	const auto end = start + 8;
	uint64_t address = 0;
	for (auto iterator = start; iterator != end; iterator++)
		{
		address <<= 8;
		address |= uint64_t(*iterator);
		}
	return address;
	}

ModemStatus XBeeApi::GetModemStatus() const
	{
	return ModemStatus(buffer[0]);
	}

/*
Receive state machine.
Handles escaped data bytes and checksum validation.
*/
void XBeeApi::handleReceivedByte(byte rxb)
	{
#ifdef DEBUG_XBEE_API
	printRxChar(rxb);
#endif
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
			frameLength = uint16_t(rxb) << 8;
			rxState = ReceiveLengthLsb;
			break;
		case ReceiveLengthLsb:
			frameLength |= uint16_t(rxb);
			rxState = ReceiveApiId;
			break;
		case ReceiveApiId:
			frameType = FrameType(rxb);
			checksum = rxb;
			rxState = ReceivePayload;
#ifdef DEBUG_XBEE_API
			printFrame();
#endif
			break;
		case ReceivePayload:
			buffer.push_back(rxb);
			checksum += rxb;
			if (buffer.size() == (frameLength - 1))
				rxState = ReceiveChecksum;
			break;
		case ReceiveChecksum:
			checksum += rxb;
#ifdef DEBUG_XBEE_API
			printChecksum();
#endif
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

#ifdef DEBUG_XBEE_API
void XBeeApi::printFrame()
	{
	Serial.print(F("Type: "));
	Serial.print(frameType, HEX);
	Serial.print(' ');
	printFrameType();
	Serial.print(F(" Length: "));
	Serial.print(frameLength);
	Serial.println();
	}

void XBeeApi::printModemStatus(ModemStatus status)
	{
	Serial.print(F("Modem status: "));
	Serial.println(status);
	}

void XBeeApi::printChecksum()
	{
	Serial.print("Checksum: ");
	Serial.println(checksum, HEX);
	}

void XBeeApi::printRxChar(byte rxb)
	{
	Serial.print("State: ");
	Serial.print(rxState, HEX);
	Serial.print(" Rx: ");
	Serial.println(rxb, HEX);
	}

void XBeeApi::printFrameType()
	{
	switch(frameType)
		{
		case Rx64Response:
			std::cout << "Rx64";
			break;
		case Rx16Response:
			std::cout << "Rx16";
			break;
		case TxStatusResponse:
			std::cout << "TxStat";
			break;
		case ModemStatusResponse:
			std::cout << "Modem";
			break;
		default:
			std::cout << "Unsupported";
			break;
		}
	}
#endif
