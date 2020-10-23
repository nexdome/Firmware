#include <sstream>
#include "Response.h"
#include "CommandProcessor.h"

/*
 * Originally, this was a builder that returned a reference to the built response.
 * However, this led to a proliferation of references and was over-complex.
 * There is no need to ever have more than one response at any moment, so
 * we now have a single statically allocated response string, which is set
 * by one or more of the helper methods. This string can then be accessed
 * directly in the global scope, when a response needs to be sent somewhere.
 * We are single-threaded run-to-completion, so there should be no issues
 * with multiple commands in progress overwriting each others responses.
 */
std::string ResponseBuilder::Message;

void ResponseBuilder::Error()
	{
	Message = "Err";
	}

bool ResponseBuilder::available() { return Message.length() > 0; }

void ResponseBuilder::startResponse(const Command &command) {
	Message.clear();
    Message.append(command.Verb);
    Message.push_back(command.TargetDevice);
}

void ResponseBuilder::FromSuccessfulCommand(const Command& command)
	{
    startResponse(command);
	}

void ResponseBuilder::FromString(const Command& command, const std::string& message)
{
    startResponse(command);
    Message.append(message);
}

void ResponseBuilder::FromInteger(const Command& command, const int32_t i)
	{
    startResponse(command);
	std::ostringstream converter;
	converter << i;
    Message.append(converter.str());
	}

void ResponseBuilder::NoResponse(const Command &command)
    {
        Message.clear();
    }

const char ResponseBuilder::terminator = '#';
const char ResponseBuilder::header = ':';

