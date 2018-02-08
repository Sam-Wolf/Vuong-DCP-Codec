// log.h; the header file which defines Log(); and LogErr();

#define LOGFILE	"h:\\vuong-dcp\\lag.log"     // all Log(); messages will be appended to this file

extern bool LogCreated;      // keeps track whether the log file is created or not

void Log(char *message);    // logs a message to LOGFILE
void LogErr(char *message); // logs a message; execution is interrupted#pragma once

/*
std::stringstream stream;
stream << std::dec << prob_ranges[a + 1] - prob_ranges[a];
std::string result(stream.str());

std::stringstream streami;
streami << std::dec << a;
std::string resulti(streami.str());

Log("prob_ranges[");
Log((char*)&resulti);
Log("] = ");
Log((char*)&result);
Log("\n");
*/