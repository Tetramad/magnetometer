#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <log.h>
#include <macros.h>
#include <sleep_and_wait.h>
#include <swo.h>

#define INF_PREFIX "I: "
#define INF_PREFIX_STRLEN (sizeof("I: ") - 1UL)

/* clang-format off */
static char log_level_to_character(int level);
/* clang-format on */

static char buffer[256] = {0};

int log_string(int level, const char *prefix, const char *str) {
    const lldiv_t dm =
        lldiv((long long)(get_ticks() & ~(1ULL << 63)), 2000000LL);
    int err = 0;

    err = snprintf(buffer,
                   sizeof(buffer),
                   "[%06u.%03u] %c: %s%s%s",
                   (unsigned)dm.quot,
                   (unsigned)(dm.rem / 2000LL),
                   log_level_to_character(level),
                   prefix ? prefix : "",
                   prefix ? ": " : "",
                   str);

    swo_puts(buffer);

    return -(err < 0);
}

int log_signed(int level, const char *prefix, signed val) {
    const lldiv_t dm =
        lldiv((long long)(get_ticks() & ~(1ULL << 63)), 2000000LL);
    int err = 0;

    err = snprintf(buffer,
                   sizeof(buffer),
                   "[%06u.%03u] %c: %s%s%d",
                   (unsigned)dm.quot,
                   (unsigned)(dm.rem / 2000LL),
                   log_level_to_character(level),
                   prefix ? prefix : "",
                   prefix ? ": " : "",
                   val);

    swo_puts(buffer);

    return -(err < 0);
}

int log_unsigned(int level, const char *prefix, unsigned val) {
    const lldiv_t dm =
        lldiv((long long)(get_ticks() & ~(1ULL << 63)), 2000000LL);
    int err = 0;

    err = snprintf(buffer,
                   sizeof(buffer),
                   "[%06u.%03u] %c: %s%s%u",
                   (unsigned)dm.quot,
                   (unsigned)(dm.rem / 2000LL),
                   log_level_to_character(level),
                   prefix ? prefix : "",
                   prefix ? ": " : "",
                   val);

    swo_puts(buffer);

    return -(err < 0);
}

int log_inf_puts(const char *str) {
    return log_string(LOG_LEVEL_INF, INF_PREFIX, str);
}

int log_inf_int(int val, const char *name) {
    char prefix[32] = INF_PREFIX;
    size_t len = sizeof(INF_PREFIX) - 1;

    strncat(prefix, name, sizeof(prefix) - 1 - len);
    len = strlen(prefix);
    strncat(prefix, ": ", sizeof(prefix) - 1 - len);
    len = strlen(prefix);

    if (len == 31) {
        prefix[sizeof(prefix) - 1] = '\0';
        prefix[sizeof(prefix) - 2] = ' ';
        prefix[sizeof(prefix) - 3] = ':';
        prefix[sizeof(prefix) - 4] = '.';
        prefix[sizeof(prefix) - 5] = '.';
        prefix[sizeof(prefix) - 6] = '.';
    }

    return log_signed(LOG_LEVEL_INF, prefix, val);
}

int log_inf_unsigned(unsigned val, const char *name) {
    char prefix[32] = INF_PREFIX;
    size_t len = sizeof(INF_PREFIX) - 1;

    strncat(prefix, name, sizeof(prefix) - 1 - len);
    len = strlen(prefix);
    strncat(prefix, ": ", sizeof(prefix) - 1 - len);
    len = strlen(prefix);

    if (len == 31) {
        prefix[sizeof(prefix) - 1] = '\0';
        prefix[sizeof(prefix) - 2] = ' ';
        prefix[sizeof(prefix) - 3] = ':';
        prefix[sizeof(prefix) - 4] = '.';
        prefix[sizeof(prefix) - 5] = '.';
        prefix[sizeof(prefix) - 6] = '.';
    }

    return log_unsigned(LOG_LEVEL_INF, prefix, val);
}

static char log_level_to_character(int level) {
    switch (level) {
    case LOG_LEVEL_ERR:
        return 'E';
    case LOG_LEVEL_WRN:
        return 'W';
    case LOG_LEVEL_INF:
        return 'I';
    case LOG_LEVEL_DBG:
        return 'D';
    default:
        return '?';
    }
}
