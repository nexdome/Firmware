// This file is part of the TA.NexDome project
// 
// Copyright © 2016-2019 Tigra Astronomy, all rights reserved.
// 
// File: XBeeWaitForOkState.h  Last modified: 2019-02-13@14:22 by Tim Long

#pragma once
#include <XBeeStateMachine.h>
#include <ArduinoSTL.h>

class XBeeWaitForCommandModeState : public IXBeeState
	{
public:
	explicit XBeeWaitForCommandModeState(XBeeStateMachine& machine):IXBeeState(machine){};
	const std::string name() override { return "WaitOk"; };
	void OnTimerExpired() override;
	void OnEnter() override;
	void OnSerialLineReceived(String& rxData) override;
private:
	const String XBeeInitString = XBEE_SHUTTER_INIT_STRING;
	};
