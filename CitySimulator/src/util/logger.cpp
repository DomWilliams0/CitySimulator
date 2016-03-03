#include <iostream>
#include <boost/algorithm/string.hpp>
#include "utils.hpp"
#include "service/logging_service.hpp"

const std::string PREFIX_STRING("    ");

void LoggingService::onEnable()
{
	levels[LOG_DEBUGGIEST] = "DEBUG";
	levels[LOG_DEBUGGIER] = "DEBUG";
	levels[LOG_DEBUG] = "DEBUG";
	levels[LOG_INFO] = "INFO";
	levels[LOG_WARNING] = "WARN";
	levels[LOG_ERROR] = "FATAL";

	// indent all levels equally
	unsigned int longest(0);
	for (auto &pair : levels)
		longest = std::max(longest, static_cast<unsigned int>(pair.second.length()));

	const int padding = 1;
	longest += padding;

	for (auto &pair : levels)
		pair.second += std::string(longest - pair.second.length(), ' ');
}

void LoggingService::log(const std::string &msg, LogLevel level)
{
	if (level < this->level)
		return;

	auto l = levels.find(level);
	if (l == levels.end())
		error("Invalid log level %1%", _str(level));

	stream << l->second << ": " << prefix << msg << std::endl;
}

void LoggingService::pushIndent()
{
	prefix += PREFIX_STRING;
}

void LoggingService::popIndent()
{
	auto currentLength = prefix.length();
	auto prefixLength = PREFIX_STRING.length();

	if (currentLength >= prefixLength)
		prefix = prefix.substr(0, currentLength - prefixLength);
}

LogLevel LoggingService::logLevelFromString(const std::string &s)
{
	std::string input(s);
	boost::algorithm::to_lower(input);

	if (input == "debuggiest")
		return LOG_DEBUGGIEST;
	else if (input == "debuggier")
		return LOG_DEBUGGIER;
	else if (input == "debug")
		return LOG_DEBUG;
	else if (input == "info")
		return LOG_INFO;
	else if (input == "warning")
		return LOG_WARNING;
	else if (input == "error")
		return LOG_ERROR;
	else
		return LOG_UNKNOWN;
}

void LoggingService::setLogLevel(const std::string &s)
{
	LogLevel newLevel = logLevelFromString(s);

	auto it = levels.find(newLevel);

	if (it != levels.end())
	{
		// the string representations of the 3 debug levels are the same
		// hence this hacky fix
		// i wont tell if you dont

		std::string asString;
		switch (newLevel)
		{
			case LOG_DEBUGGIEST:
				asString = "debuggiest";
				break;
			case LOG_DEBUGGIER:
				asString = "debuggier";
				break;
			default:
				asString = boost::trim_right_copy(it->second);
				break;
		}

		logInfo(format("Setting log level to '%1%'", asString));
		level = newLevel;
	}
	else
		logWarning(format("Could not set log level to unknown level '%1%'", s));
}

