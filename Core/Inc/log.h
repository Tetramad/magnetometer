/*
 * log.h
 *
 *      Author: Tetramad
 */

#ifndef __LOG_H
#define __LOG_H

#include <stddef.h>

#include <macros.h>

#define LOG_LEVEL_DBG 4
#define LOG_LEVEL_INF 3
#define LOG_LEVEL_WRN 2
#define LOG_LEVEL_ERR 1
#define LOG_LEVEL_OFF 0

#define LOG_ENABLE(level)                                                           \
    _Pragma(                                                                        \
        "GCC warning                                                        \
            \"LOG_ENABLED was deprecated. Use LOG_LEVEL_SET instead.\"") static int \
        __log_level = level;
#define LOG_LEVEL_SET(level)                                                   \
    _Pragma("GCC diagnostic push") _Pragma(                                    \
        "GCC diagnostic ignored \"-Wshadow\"") __attribute__((unused)) static int __log_level = level; \
    _Pragma("GCC diagnostic pop")

#define LOG_GENERIC(...)                                                       \
    _Generic((FIRST_ARG(__VA_ARGS__)),\
                const char *: LogString,\
                char *: LogString,\
                short: LogSigned,\
                int: LogSigned,\
                unsigned char: LogUnsigned,\
                unsigned short: LogUnsigned,\
                unsigned: LogUnsigned,\
                long unsigned int: LogUnsigned)(__log_level,\
                                                 (SECOND_ARG(__VA_ARGS__, NULL)),\
                                                 (FIRST_ARG(__VA_ARGS__)))

#define LOG_DBG(...)                                                           \
    do {                                                                       \
        if (__log_level >= LOG_LEVEL_DBG) {                                    \
            LOG_GENERIC(__VA_ARGS__);                                          \
        }                                                                      \
    } while (0)

#define LOG_INF(...)                                                           \
    do {                                                                       \
        if (__log_level >= LOG_LEVEL_INF) {                                    \
            LOG_GENERIC(__VA_ARGS__);                                          \
        }                                                                      \
    } while (0)

#define LOG_WRN(...)                                                           \
    do {                                                                       \
        if (__log_level >= LOG_LEVEL_WRN) {                                    \
            LOG_GENERIC(__VA_ARGS__);                                          \
        }                                                                      \
    } while (0)

#define LOG_ERR(...)                                                           \
    do {                                                                       \
        if (__log_level >= LOG_LEVEL_ERR) {                                    \
            LOG_GENERIC(__VA_ARGS__);                                          \
        }                                                                      \
    } while (0)

__attribute__((deprecated)) int LogInfPuts(const char *str);
__attribute__((deprecated)) int LogInfInt(int val, const char *name);
__attribute__((deprecated)) int LogInfUnsigned(unsigned val,
                                                 const char *name);

int LogString(int level, const char *prefix, const char *str);
int LogSigned(int level, const char *prefix, signed val);
int LogUnsigned(int level, const char *prefix, unsigned val);

#endif /* __LOG_H */
