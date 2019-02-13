/*
 Name:		XBeeTerminal.ino
 Created:	1/31/2019 7:39:43 AM
 Author:	Tim Long, Tigra Astronomy

This project makes teh Arduino Leonardo act like a dumb terminal for communicating with the XBee module.
This was needed for experimentation with the XBee configuration.
Any and all characters sent to the Arduino are relayed to the XBee, and vice versa.
The Arduino serial port is configured for 115,200 baud N-8-1
*/

auto& xbee = Serial1;
auto& host = Serial;

// the setup function runs once when you press reset or power the board
void setup()
	{
	xbee.begin(9600);
	host.begin(115200);
	}

// the loop function runs over and over again until power down or reset
void loop()
	{
	auto xbee_available = xbee.available();
	if (xbee_available > 0)
		{
		const char input = xbee.read();
		host.write(input);
		}
	auto host_available = host.available();
	if (host_available > 0)
		{
		const char input = host.read();
		xbee.write(input);
		}
	}
