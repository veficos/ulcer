

#ifndef _ULCER_ERROR_H_
#define _ULCER_ERROR_H_

#include "config.h"

#define expect error
void runtime_error(long line, long column, const char *fmt, ...);
void error(const char *filename, long line, long column, const char *fmt, ...);
void warning(const char *filename, long line, long column, const char *fmt, ...);

#endif
