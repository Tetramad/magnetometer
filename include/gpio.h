#ifndef GPIO_H
#define GPIO_H

#define GPIO_CONFIG_PORT_A 0
#define GPIO_CONFIG_PORT_B 1

#define GPIO_CONFIG_PIN_0 0
#define GPIO_CONFIG_PIN_1 1
#define GPIO_CONFIG_PIN_2 2
#define GPIO_CONFIG_PIN_3 3
#define GPIO_CONFIG_PIN_4 4
#define GPIO_CONFIG_PIN_5 5
#define GPIO_CONFIG_PIN_6 6
#define GPIO_CONFIG_PIN_7 7
#define GPIO_CONFIG_PIN_8 8
#define GPIO_CONFIG_PIN_9 9
#define GPIO_CONFIG_PIN_10 10
#define GPIO_CONFIG_PIN_11 11
#define GPIO_CONFIG_PIN_12 12
#define GPIO_CONFIG_PIN_13 13
#define GPIO_CONFIG_PIN_14 14

#define GPIO_CONFIG_MODE_INPUT 0
#define GPIO_CONFIG_MODE_OUTPUT 1
#define GPIO_CONFIG_MODE_AF 2

#define GPIO_CONFIG_PUPD_NONE 0
#define GPIO_CONFIG_PUPD_PU 1
#define GPIO_CONFIG_PUPD_PD 2
#define GPIO_CONFIG_PUPD_PUPD 3

struct gpio_config {
    unsigned port;
    unsigned pin;
    unsigned mode;
    unsigned pupd;
};

int gpio_configure(struct gpio_config config);

#endif /* GPIO_H */
