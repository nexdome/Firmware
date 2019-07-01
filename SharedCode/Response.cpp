
#include <sstream>
#include "CommandProcessor.h"

/*
 * Stream extraction operator: outputs the response if and only if the message
 * is not empty. An empty message indicates no response, which produces no output.
 */
std::ostream& operator<<(std::ostream& os, const Response& obj)
	{
	if (obj.Message.length() > 0)
		os << ':' << obj.Message << Response::terminator << std::endl;
	return os;
	}

Response Response::Error()
	{
	auto error = Response{ "Err" };
	return error;
	}

Response Response::FromSuccessfulCommand(Command& command)
	{
	std::ostringstream converter;
	converter << command.Verb << command.TargetDevice;
	return Response{ converter.str() };
	}

Response Response::FromInteger(Command& command, int i)
	{
	std::ostringstream converter;
	converter << command.Verb << command.TargetDevice << i;
	return Response{ converter.str() };
	}

Response Response::NoResponse(Command& command)
	{
	return Response{ "" };
	}

const std::string Response::terminator = "#";
const std::string Response::header = ":";

