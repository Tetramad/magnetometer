#ifndef TIM3_H
#define TIM3_H

int tim3_initialize(void);
int tim3_register_isr(void (*callback)(void *), void *payload);
int tim3_enable(void);
int tim3_disable(void);

#endif /* TIM3_H */
