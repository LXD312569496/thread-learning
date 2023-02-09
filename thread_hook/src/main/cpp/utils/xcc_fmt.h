//
// Created by Administrator on 2022/11/22.
//

#ifndef XCC_FMT_H
#define XCC_FMT_H 1

#include <stdint.h>
#include <sys/types.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

size_t xcc_fmt_snprintf(char *buffer, size_t buffer_size, const char *format, ...);
size_t xcc_fmt_vsnprintf(char *buffer, size_t buffer_size, const char *format, va_list args);

#ifdef __cplusplus
}
#endif

#endif