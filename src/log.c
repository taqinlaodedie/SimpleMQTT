#include "log.h"
#include <stdio.h>
#include <stdarg.h>

static void _write(const char *buf, int len)
{
#if defined(_WIN32) || defined(_WIN64)
    len = len>256 ? 256:len;
    for (int i = 0; i < len; i++) {
        putc(buf[i], stdout);
    }
#endif
}

int MQTT_printf(const char *fmt, ...)
{
    va_list args;
    char buf[256];
    int i;
    va_start(args, fmt);
    i = vsprintf(buf, fmt, args);
    va_end(args);
    _write(buf, i);
    return i;
}