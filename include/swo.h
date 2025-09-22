#ifndef SWO_H
#define SWO_H

#include <stddef.h>

#include <stm32f4xx.h>

static inline int enable_swo(void) {
    RCC->AHB1ENR = RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN;

    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

    TPI->CSPSR = 1;
    TPI->SPPR = 2;
    TPI->ACPR = 16000000U / 2000000U - 1U;

    DBGMCU->CR &= ~DBGMCU_CR_TRACE_MODE;
    DBGMCU->CR |= DBGMCU_CR_TRACE_IOEN;

    ITM->LAR = 0xC5ACCE55;
    ITM->TCR = ITM_TCR_ITMENA_Msk | ITM_TCR_SWOENA_Msk | ITM_TCR_SYNCENA_Msk |
               (0x7U << ITM_TCR_TraceBusID_Pos);
    ITM->TPR = 1U;
    ITM->TER = 1U;

    return 0;
}

static inline size_t swo_puts(const char *s) {
    size_t written = 0;

    while (*s) {
        ITM_SendChar(*s++);
        ++written;
    }
    ITM_SendChar('\n');

    return written;
}

#endif /* SWO_H */
