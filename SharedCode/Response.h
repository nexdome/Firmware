#ifndef _RESPONSE_h
#define _RESPONSE_h

#if defined(ARDUINO) && ARDUINO >= 100
#else
#include "WProgram.h"
#endif

#include "Command.h"

typedef std::string &Response;

class ResponseBuilder
{
public:
    static const char terminator;
    static const char header;
    static std::string Message;

	/*
	 * Returns true if there is a valid response message available
	 */
	static bool available();

    /*
	 * Initializes for a new response
	 */
    static void startResponse(const Command &command);

    /*
        Creates an error response.
    */
    static void Error();

    /*
		Creates a success response by echoing the command verb,
		terminated with a '#'.
	*/
    static void FromSuccessfulCommand(const Command &command);

	/*
	 * Creates a response from a literal string or string value,
	 */
    static void FromString(const Command &command, const std::string &message);


    /*
        Creates a response consisting of the command verb, followed by an integer
    */
    static void FromInteger(const Command &command, const int32_t i);

    /*
     * Creates a response with no message, i.e. nothing is returned to the sender.
     */
    static void NoResponse(const Command &command);
};

#endif
