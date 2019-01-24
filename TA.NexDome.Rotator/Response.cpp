#include "CommandProcessor.h"

Response Response::Error()
	{
	auto error = Response{ "Err" };
	error.success = false;
	return error;
	}

Response Response::FromSuccessfulCommand(Command& command)
	{
	auto message = command.Verb + Response::Terminator;
	return Response{ message };
	}

Response Response::FromPosition(Command& command, uint32_t position)
	{
	String message = command.Verb + (String)position + Response::Terminator;
	return Response{ message };
	}

Response Response::FromInteger(Command& command, int i)
	{
	return Response{ command.Verb + (String)i + Response::Terminator };
	}

const String Response::Terminator = "#";

