

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

void runtime_errnu(long line, long column, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "ulcer:%ld:%ld: runtime error: ", line, column);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
    exit(-1);
}

void runtime_error(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "ulcer: runtime error: ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
    exit(-1);
}

void error(const char *filename, long line, long column, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    filename ? fprintf(stderr, "%s:%ld:%ld: error: ", filename, line, column) : fprintf(stderr, "ulcer: error: ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
    exit(-1);
}

void warning(const char *filename, long line, long column, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    filename ? fprintf(stderr, "%s:%ld:%ld: warring: ", filename, line, column) : fprintf(stderr, "ulcer: warring: ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
}