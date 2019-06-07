// XBeeReadFirmwareVersionState.h

#ifndef _XBEEREADFIRMWAREVERSIONSTATE_h
#define _XBEEREADFIRMWAREVERSIONSTATE_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include "XBeeStateMachine.h"

class XBeeReadFirmwareVersionState : public IXBeeState
	{
	public:
		explicit XBeeReadFirmwareVersionState(XBeeStateMachine& machine) : IXBeeState(machine) {};
		std::string name() override { return "Read Firmware Version"; }
		void OnTimerExpired() override;
		void OnEnter() override;
		void OnSerialLineReceived(const std::string& rxData) override;

	};

#endif

