// 
// 
// 

#include "XBeeShutterReadyState.h"


void XBeeShutterReadyState::SendMessage(Tx64Request& message)
	{
	machine.SendXbeeApiFrame(message);
	}
