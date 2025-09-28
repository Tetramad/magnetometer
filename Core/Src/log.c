/*
 * log.c
 *
 *      Author: Tetramad
 */

#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <stm32f4xx_hal.h>

#include <log.h>
#include <macros.h>

#define INF_PREFIX "I: "
#define INF_PREFIX_STRLEN (sizeof("I: ") - 1UL)

static char LogLevelToCharacter(int level);

static char buffer[256] = { 0 };

int LogString(int level, const char *prefix, const char *str) {
	const uint32_t ticks = HAL_GetTick();
	int err = 0;

	err = snprintf(buffer, sizeof(buffer),
			"[%06" PRIu32 ".%03" PRIu32 "] %c: %s%s%s", ticks / 1000U,
			ticks % 1000U, LogLevelToCharacter(level), prefix ? prefix : "",
			prefix ? ": " : "", str);

	puts(buffer);

	return -(err < 0);
}

int LogSigned(int level, const char *prefix, signed val) {
	const uint32_t ticks = HAL_GetTick();
	int err = 0;

	err = snprintf(buffer, sizeof(buffer),
			"[%06" PRIu32 ".%03" PRIu32 "] %c: %s%s%d", ticks / 1000U,
			ticks % 1000U, LogLevelToCharacter(level), prefix ? prefix : "",
			prefix ? ": " : "", val);

	puts(buffer);

	return -(err < 0);
}

int LogUnsigned(int level, const char *prefix, unsigned val) {
	const uint32_t ticks = HAL_GetTick();
	int err = 0;

	err = snprintf(buffer, sizeof(buffer),
			"[%06" PRIu32 ".%03" PRIu32 "] %c: %s%s%u", ticks / 1000U,
			ticks % 1000U, LogLevelToCharacter(level), prefix ? prefix : "",
			prefix ? ": " : "", val);

	puts(buffer);

	return -(err < 0);
}

int LogInfPuts(const char *str) {
	return LogString(LOG_LEVEL_INF, INF_PREFIX, str);
}

int LogInfInt(int val, const char *name) {
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

	return LogSigned(LOG_LEVEL_INF, prefix, val);
}

int LogInfUnsigned(unsigned val, const char *name) {
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

	return LogUnsigned(LOG_LEVEL_INF, prefix, val);
}

static char LogLevelToCharacter(int level) {
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
