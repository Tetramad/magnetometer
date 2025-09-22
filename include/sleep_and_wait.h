#ifndef SLEEP_AND_WAIT_H
#define SLEEP_AND_WAIT_H

#include <stdint.h>

typedef uint64_t tick_t;

int sleep_and_wait_init(void);

tick_t get_ticks(void);

int busy_wait_ms(uint32_t wait_time_ms);
int busy_wait_us(uint32_t wait_time_us);
__attribute__((deprecated)) int wait_ms(uint32_t wait_time_ms);
__attribute__((deprecated)) int wait_us(uint32_t wait_time_us);

#endif /* SLEEP_AND_WAIT_H */
