// This file is part of the TA.NexDome project
// 
// Copyright © 2016-2019 Tigra Astronomy, all rights reserved.
// 
// File: XBeeWaitForOkState.h  Last modified: 2019-02-13@14:22 by Tim Long

#pragma once
#include <XBeeStateMachine.h>

class XBeeWaitForCommandModeState : public IXBeeState
	{
public:
	explicit XBeeWaitForCommandModeState(XBeeStateMachine& machine):IXBeeState(machine){};
	std::string name() override { return stateName; }
	void OnTimerExpired() override;
	void OnEnter() override;
	void OnSerialLineReceived(const std::string& rxData) override;
private:
	const char* stateName __ATTR_PROGMEM__ = "WaitAT";
	};
