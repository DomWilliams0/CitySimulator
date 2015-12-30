#include <iostream>
#include <algorithm>
#include "services.hpp"

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
		auto l = levels.find(level);
		if (l == levels.end())
			error("Invalid log level %1%", std::to_string(level));

		// todo syslog
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
