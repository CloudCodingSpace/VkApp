#pragma once 

#include <stdio.h>
#include <string>

typedef enum {
    LOG_SEVERITY_INFO,
#ifdef _DEBUG
    LOG_SEVERITY_DEBUG,
#endif
    LOG_SEVERITY_WARN,
    LOG_SEVERITY_ERROR,
    LOG_SEVERITY_FATAL
} log_severity;

void log_output(log_severity severity, std::string msg);

#define INFO(msg) log_output(LOG_SEVERITY_INFO, msg);
#define WARN(msg) log_output(LOG_SEVERITY_WARN, msg);
#define ERROR(msg) log_output(LOG_SEVERITY_ERROR, msg);
#define FATAL(msg) log_output(LOG_SEVERITY_FATAL, msg);

#ifdef _DEBUG
#define DEBUG(msg) log_output(LOG_SEVERITY_DEBUG, msg);
#else
#define DEBUG(msg)
#endif