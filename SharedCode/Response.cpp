#include "CommandProcessor.h"

Response Response::Error()
	{
	auto error = Response{ "Err" };
	error.success = false;
	return error;
	}

Response Response::FromSuccessfulCommand(Command& command)
	{
	std::string message;
	message.append(command.Verb);
	message.append(Response::Terminator);
	return Response{ message };
	}

Response Response::FromPosition(Command& command, uint32_t position)
	{
	std::string message;
	String positionString = (String)position;
	message.append(command.Verb);
	message.append(positionString.begin(), positionString.length());
	message.append(Response::Terminator);
	return Response{ message };
	}

Response Response::FromInteger(Command& command, int i)
	{
	return Response::FromPosition(command, i);
	}

const std::string Response::Terminator = "#";
std::string Response::messageBuffer;

