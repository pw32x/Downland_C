#include <stdarg.h>
#include <stdio.h>

#include "log.h"

void log(fmt)
	char *fmt;
{
	va_list args;
	long a0, a1, a2, a3, a4, a5;

	va_start(args, fmt);
	a0 = va_arg(args, long);
	a1 = va_arg(args, long);
	a2 = va_arg(args, long);
	a3 = va_arg(args, long);
	a4 = va_arg(args, long);
	a5 = va_arg(args, long);
	va_end(args);

	printf(fmt, a0, a1, a2, a3, a4, a5);
}
