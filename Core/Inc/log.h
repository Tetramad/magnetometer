/*
 * log.h
 *
 *      Author: Tetramad
 */

#ifndef __LOG_H
#define __LOG_H

#include <stddef.h>

#define LOG_LEVEL_DBG 4
#define LOG_LEVEL_INF 3
#define LOG_LEVEL_WRN 2
#define LOG_LEVEL_ERR 1
#define LOG_LEVEL_OFF 0

#define LOG_LEVEL_SET(level)                                                   \
    _Pragma("GCC diagnostic push") _Pragma(                                    \
        "GCC diagnostic ignored \"-Wshadow\"") __attribute__((unused)) static int __log_level = level; \
    _Pragma("GCC diagnostic pop")

#define LOG_DBG(...)                                                           \
    do {                                                                       \
        if (__log_level >= LOG_LEVEL_DBG) {                                    \
            Log_Format(LOG_LEVEL_DBG, NULL, __VA_ARGS__);                      \
        }                                                                      \
    } while (0)

#define LOG_INF(...)                                                           \
    do {                                                                       \
        if (__log_level >= LOG_LEVEL_INF) {                                    \
            Log_Format(LOG_LEVEL_INF, NULL, __VA_ARGS__);                      \
        }                                                                      \
    } while (0)

#define LOG_WRN(...)                                                           \
    do {                                                                       \
        if (__log_level >= LOG_LEVEL_WRN) {                                    \
            Log_Format(LOG_LEVEL_WRN, NULL, __VA_ARGS__);                      \
        }                                                                      \
    } while (0)

#define LOG_ERR(...)                                                           \
    do {                                                                       \
        if (__log_level >= LOG_LEVEL_ERR) {                                    \
            Log_Format(LOG_LEVEL_ERR, NULL, __VA_ARGS__);                      \
        }                                                                      \
    } while (0)

int Log_Format(int level, const char *prefix, const char *fmt, ...);

#endif /* __LOG_H */
