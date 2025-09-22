#include <stddef.h>

#include <stm32f4xx.h>

#include <gpio.h>
#include <log.h>

int gpio_configure(struct gpio_config config) {
    if (config.port >= 2) {
        return -1;
    }

    if (config.pin >= 16) {
        return -1;
    }

    if (config.mode >= 3) {
        return -1;
    }

    if (config.pupd >= 4) {
        return -1;
    }

    switch (config.port) {
    case GPIO_CONFIG_PORT_A:
        SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOAEN);
        break;
    case GPIO_CONFIG_PORT_B:
        SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOBEN);
        break;
    default:
        return -1;
    }

    GPIO_TypeDef *gpio = NULL;
    switch (config.port) {
    case GPIO_CONFIG_PORT_A:
        gpio = GPIOA;
        break;
    case GPIO_CONFIG_PORT_B:
        gpio = GPIOB;
        break;
    default:
        return -1;
    }

    {
        const unsigned pos = 2UL * config.pin;
        const unsigned msk = 0x3UL << pos;
        const unsigned val = config.mode << pos;
        MODIFY_REG(gpio->MODER, msk, val);
    }

    {
        const unsigned pos = 2UL * config.pin;
        const unsigned msk = 0x3UL << pos;
        const unsigned val = config.pupd << pos;
        MODIFY_REG(gpio->PUPDR, msk, val);
    }

    return 0;
}
