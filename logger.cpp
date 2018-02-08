#include "logger.h"
#include <stdarg.h>

Logger *Logger::theInstance = NULL;

Logger::Logger()
{
	logFile = fopen("Log.txt", "wt");
}

Logger::~Logger()
{
	if (logFile)
	{
		fflush(logFile);
		fclose(logFile);
		logFile = NULL;
	}
}

Logger* Logger::getInstance()
{
	if (theInstance == NULL)
	{
		theInstance = new Logger();
	}
	return theInstance;
}

void Logger::release()
{
	if (theInstance != NULL)
	{
		delete theInstance;
	}
	theInstance = NULL;
}

bool Logger::log(char* _log, ...)
{
	char acText[1024];
	va_list VAList;

	va_start(VAList, _log);
	vsprintf(acText, _log, VAList);
	va_end(VAList);

	fprintf(Logger::logFile, "%s (Line %d): %s \n", __FILE__, __LINE__, acText);

	return true;
}