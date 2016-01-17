

#ifndef _ULCER_ERROR_H_
#define _ULCER_ERROR_H_

#include "config.h"

void runtime_errnu(long line, long column, const char *fmt, ...);
void runtime_error(const char* fmt, ...);

void error(const char *filename, long line, long column, const char *fmt, ...);
void warning(const char *filename, long line, long column, const char *fmt, ...);

#endif
