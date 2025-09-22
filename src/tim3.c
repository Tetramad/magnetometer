#include <stddef.h>

#include <stm32f4xx.h>

#include <log.h>
#include <work_queue.h>

LOG_LEVEL_SET(LOG_LEVEL_INF);

static void (*isr)(void *) = NULL;
static void *isr_payload = NULL;

void TIM3_IRQHandler(void) {
    LOG_DBG("TIM3: IRQ detected");

    if (isr) {
        isr(isr_payload);
    }

    NVIC_ClearPendingIRQ(TIM3_IRQn);
    CLEAR_BIT(TIM3->SR, TIM_SR_UIF);
}

int tim3_initialize(void) {
    SET_BIT(RCC->APB1ENR, RCC_APB1ENR_TIM3EN);

    /*
     * 8Hz Update Interrupt
     * ARR = 16000000 Hz / (199 + 1) / 8 Hz
     *     = 10000 <= 65535
     */
    WRITE_REG(TIM3->PSC, 199);
    WRITE_REG(TIM3->ARR, 10000);
    WRITE_REG(TIM3->CNT, 10000);
    SET_BIT(TIM3->CR1, TIM_CR1_DIR);
    SET_BIT(TIM3->DIER, TIM_DIER_UIE);

    NVIC_SetPriority(TIM3_IRQn, (1UL << __NVIC_PRIO_BITS) - 2UL);
    NVIC_EnableIRQ(TIM3_IRQn);

    return 0;
}

int tim3_register_isr(void (*callback)(void *), void *payload) {
    isr = callback;
    isr_payload = payload;

    return 0;
}

int tim3_enable(void) {
    SET_BIT(TIM3->CR1, TIM_CR1_CEN);

    return 0;
}

int tim3_disable(void) {
    CLEAR_BIT(TIM3->CR1, TIM_CR1_CEN);

    return 0;
}
