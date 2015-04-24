//
// Created by Cristian Marastoni on 23/04/15.
//

#include "Debug.h"
#include <stdarg.h>
#include <stdio.h>

void Debug::Log(const char *ctx, const char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    vprintf(fmt, va);
    va_end(va);
}

void Debug::Error(const char *ctx, const char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    vprintf(fmt, va);
    va_end(va);
}
