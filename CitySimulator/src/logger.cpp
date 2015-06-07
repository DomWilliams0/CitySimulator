#pragma once
#include <iostream>
#include <string>
#include "logger.hpp"

struct _Logger
{
	Logger::Level level;
	std::ostream *stream;

	void log(std::string msg, Logger::Level level)
	{
		std::string l;
		switch (level)
		{
		case Logger::INFO: l = "INFO";
			break;
		case Logger::DEBUG: l = "DEBUG";
			break;
		case Logger::WARNING: l = "WARNING";
			break;
		case Logger::ERROR: l = "ERROR";
			break;
		}

		(*stream) << l << ": " << msg << std::endl;
	}
};

_Logger logger;

void Logger::createLogger(std::ostream &os, Level loggingLevel)
{
	logger.stream = &os;
	logger.level = loggingLevel;
}

void Logger::logInfo(const std::string &msg)
{
	logger.log(msg, Level::INFO);
}

void Logger::logDebug(const std::string &msg)
{
	logger.log(msg, Level::DEBUG);
}

void Logger::logWarning(const std::string &msg)
{
	logger.log(msg, Level::WARNING);
}

void Logger::logError(const std::string &msg)
{
	logger.log(msg, Level::ERROR);
}
