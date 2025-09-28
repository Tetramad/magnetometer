#include <limits.h>
#include <stdint.h>

#include <stm32f4xx.h>

#include <sleep_and_wait.h>

#define SYSTICK_CLOCK 16000000UL

static volatile uint32_t tick_extended = 0;
static volatile uintptr_t sync = 0UL;

void SysTick_Handler(void) {
    ++tick_extended;
}

tick_t get_ticks(void) {
    uint32_t local_h = 0;
    uint32_t local_l = 0;

    __disable_irq();
    local_h = tick_extended;
    local_l = READ_REG(SysTick->VAL);
    __enable_irq();

    local_l = (1UL << 24U) - local_l;
    local_l = (local_h << 24U) | local_l;
    local_h = local_h >> 8U;

    return ((tick_t)local_h << 32U) | (tick_t)local_l;
}

tick_t get_timeout_tick_ms(uint32_t timeout_ms) {
    return get_ticks() + timeout_ms * (SYSTICK_CLOCK / 8UL / 1000UL);
}

tick_t get_timeout_tick_us(uint32_t timeout_us) {
    return get_ticks() + timeout_us * (SYSTICK_CLOCK / 8UL / 1000000UL);
}

int sleep_and_wait_init(void) {
    NVIC_SetPriority(SysTick_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL);
    WRITE_REG(SysTick->LOAD,
              (0xFFFFFFUL << SysTick_LOAD_RELOAD_Pos) &
                  SysTick_LOAD_RELOAD_Msk);
    WRITE_REG(SysTick->VAL,
              (0UL << SysTick_VAL_CURRENT_Pos) & SysTick_VAL_CURRENT_Msk);
    SET_BIT(SysTick->CTRL, SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk);

    return 0;
}

int busy_wait_ms(uint32_t wait_time_ms) {
    int err = 0;
    const uint32_t quot = wait_time_ms / (UINT32_MAX / 1000UL);
    const uint32_t rem = wait_time_ms % (UINT32_MAX / 1000UL);

    if (quot > 0) {
        for (size_t _ = 0; _ < quot; ++_) {
            err = busy_wait_us(UINT32_MAX);
            if (err < 0) {
                return -1;
            }
        }
    }
    return busy_wait_us(rem * 1000UL);
}

int busy_wait_us(uint32_t wait_time_us) {
    const tick_t timeout_ticks = get_timeout_tick_us(wait_time_us);

    while (get_ticks() < timeout_ticks) {
    }

    return 0;
}

int wait_ms(uint32_t wait_time_ms) {
    static const uint32_t max_wait_time_ms =
        1000UL * 60UL * 60UL; /*< maximum 1 hour wait time */

    if (wait_time_ms >= max_wait_time_ms) {
        return -1;
    }

    if (wait_time_ms == 0) {
        return 0;
    }

    const uint32_t wait_ticks =
        wait_time_ms * ((16000000UL / 1000UL) / 8UL) - 1;

    WRITE_REG(SysTick->LOAD, wait_ticks);
    WRITE_REG(SysTick->VAL, 0UL);
    SET_BIT(SysTick->CTRL, SysTick_CTRL_ENABLE_Msk);
    sync = 0;

    while (!sync) {
        __WFE();
    }

    return 0;
}

int wait_us(uint32_t wait_time_us) {
    static const uint32_t max_wait_time_us =
        1000UL * 60UL; /*< maximum 1 minute wait time */

    if (wait_time_us >= max_wait_time_us) {
        return -1;
    }

    if (wait_time_us == 0) {
        return 0;
    }

    const uint32_t wait_ticks =
        wait_time_us * ((16000000UL / 1000UL / 1000UL) / 8UL) - 1;

    WRITE_REG(SysTick->LOAD, wait_ticks);
    WRITE_REG(SysTick->VAL, 0UL);
    SET_BIT(SysTick->CTRL, SysTick_CTRL_ENABLE_Msk);
    sync = 0;

    while (!sync) {
        __WFE();
    }

    return 0;
}
