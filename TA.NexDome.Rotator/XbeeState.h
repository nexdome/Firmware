// XbeeState.h

#ifndef _XBEESTATE_h
#define _XBEESTATE_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "Command.h"

class XbeeStateTriggers
	{
protected:
	virtual void TransmitCommand(Command& command);
	
	};

class XbeeState : XbeeStateTriggers
	{
public:
	virtual ~XbeeState() = default;
	virtual void on_enter();
	virtual void on_exit();
	virtual void TransmitCommand(Command& command) override { }
	};

#endif

