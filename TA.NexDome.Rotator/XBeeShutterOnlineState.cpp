// 
// 
// 

#include "XBeeShutterOnlineState.h"


void XBeeShutterOnlineState::SendMessage(Tx64Request& message)
	{
	machine.SendXbeeApiFrame(message);
	}
