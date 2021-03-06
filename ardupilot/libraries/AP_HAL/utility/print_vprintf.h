#ifndef __utility_print_vprintf_h__
#define __utility_print_vprintf_h__


#include <stdarg.h>

#include <AP_HAL/AP_HAL.h>

void print_vprintf(AP_HAL::Print *s, const char *fmt, va_list ap);

#endif /* __utility_print_vprintf_h__ */