#pragma once 

#include <stdio.h>
#include <string>

typedef enum {
    LOG_SEVERITY_TRACE,
    LOG_SEVERITY_INFO,
    LOG_SEVERITY_DEBUG,
    LOG_SEVERITY_WARN,
    LOG_SEVERITY_ERROR,
    LOG_SEVERITY_FATAL
} log_severity;

void log_output(log_severity severity, std::string msg);

#define TRACE(msg) log_output(LOG_SEVERITY_TRACE, msg);
#define INFO(msg) log_output(LOG_SEVERITY_INFO, msg);
#define DEBUG(msg) log_output(LOG_SEVERITY_DEBUG, msg);
#define WARN(msg) log_output(LOG_SEVERITY_WARN, msg);
#define ERROR(msg) log_output(LOG_SEVERITY_ERROR, msg);
#define FATAL(msg) log_output(LOG_SEVERITY_FATAL, msg);
