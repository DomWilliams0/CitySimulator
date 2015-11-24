#pragma once
#include <iostream>

namespace Logger
{
	enum Level
	{
		INFO,
		DEBUG,
		WARNING,
		ERROR
	};

	void createLogger(std::ostream &os, Level loggingLevel);
	void logInfo(const std::string &msg);
	void logDebug(const std::string &msg);
	void logWarning(const std::string &msg);
	void logError(const std::string &msg);

	void pushIndent();
	void popIndent();
}
