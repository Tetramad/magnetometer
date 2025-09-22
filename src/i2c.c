#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <stm32f4xx.h>

#include <gpio.h>
#include <i2c.h>
#include <log.h>
#include <macros.h>
#include <sleep_and_wait.h>

LOG_LEVEL_SET(LOG_LEVEL_INF);

static int wait_busy_clear(void);
static int transmit(unsigned char address,
                    const unsigned char *buffer,
                    size_t n,
                    int with_stop);
static int
receive(unsigned char address, unsigned char *buffer, size_t n, int with_stop);

int i2c_configure(void) {
    int err = 0;
    const struct gpio_config scl = {
        .port = GPIO_CONFIG_PORT_B,
        .pin = GPIO_CONFIG_PIN_8,
        .mode = GPIO_CONFIG_MODE_AF,
        .pupd = GPIO_CONFIG_PUPD_PU,
    };
    const struct gpio_config sda = {
        .port = GPIO_CONFIG_PORT_B,
        .pin = GPIO_CONFIG_PIN_9,
        .mode = GPIO_CONFIG_MODE_AF,
        .pupd = GPIO_CONFIG_PUPD_PU,
    };

    err = gpio_configure(scl);
    if (err < 0) {
        return -EI2CIO;
    }

    err = gpio_configure(sda);
    if (err < 0) {
        return -EI2CIO;
    }

    MODIFY_REG(GPIOB->AFR[1], GPIO_AFRH_AFSEL8, GPIO_AFRH_AFSEL8_2);
    MODIFY_REG(GPIOB->AFR[1], GPIO_AFRH_AFSEL9, GPIO_AFRH_AFSEL9_2);

    SET_BIT(RCC->APB1ENR, RCC_APB1ENR_I2C1EN);

    SET_BIT(I2C1->CR1, I2C_CR1_SWRST);
    CLEAR_BIT(I2C1->CR1, I2C_CR1_SWRST);

    MODIFY_REG(I2C1->CR2, I2C_CR2_FREQ, I2C_CR2_FREQ_4);
    MODIFY_REG(I2C1->CCR,
               I2C_CCR_CCR_Msk,
               (40UL << I2C_CCR_CCR_Pos) & I2C_CCR_CCR_Msk);
    MODIFY_REG(I2C1->TRISE,
               I2C_TRISE_TRISE_Msk,
               (17UL << I2C_TRISE_TRISE_Pos) & I2C_TRISE_TRISE_Msk);

    SET_BIT(I2C1->CR1, I2C_CR1_PE);

    return 0;
}

int i2c_transaction(unsigned char address,
                    const struct i2c_transaction_data *data,
                    size_t n) {
    int err = 0;

    if (data == NULL) {
        return -EINVAL;
    }

    if (n == 0) {
        return 0;
    }

    err = wait_busy_clear();
    if (err < 0) {
        return err;
    }

    for (size_t i = 0; i < n; ++i) {
        if (data[i].type == TRANSACTION_TYPE_WRITE) {
            err = transmit(address, data[i].buffer, data[i].size, i == n - 1);
        } else if (data[i].type == TRANSACTION_TYPE_READ) {
            err = receive(address, data[i].buffer, data[i].size, i == n - 1);
        } else {
            err = -EINVAL;
        }
        if (err < 0) {
            break;
        }
    }

    return err;
}

static int wait_busy_clear(void) {
    int busy = false;
    tick_t timeout = get_ticks() + 1000 * 2000;

    while ((busy = READ_BIT(I2C1->SR2, I2C_SR2_BUSY)) &&
           get_ticks() < timeout) {
    }

    return busy ? -EBUSY : 0;
}

static int transmit(unsigned char address,
                    const unsigned char *buffer,
                    size_t n,
                    int with_stop) {
    tick_t timeout = 0UL;

    SET_BIT(I2C1->CR1, I2C_CR1_START);

    timeout = get_ticks() + 100 * 2000;
    while (!READ_BIT(I2C1->SR1, I2C_SR1_SB) && get_ticks() < timeout) {
    }
    if (!(get_ticks() < timeout)) {
        SET_BIT(I2C1->CR1, I2C_CR1_STOP);
        return -EI2CSTART;
    }

    WRITE_REG(I2C1->DR, ((unsigned)address << 1U) | 0UL);

    timeout = get_ticks() + 100 * 2000;
    while (!READ_BIT(I2C1->SR1, I2C_SR1_ADDR) && get_ticks() < timeout) {
    }
    if (!(get_ticks() < timeout)) {
        if (READ_BIT(I2C1->SR1, I2C_SR1_AF)) {
            CLEAR_BIT(I2C1->SR1, I2C_SR1_AF);
        }
        SET_BIT(I2C1->CR1, I2C_CR1_STOP);
        return -EI2CIO;
    }

    (void)READ_REG(I2C1->SR2);

    if (n == 0) {
        SET_BIT(I2C1->CR1, I2C_CR1_STOP);

        return 0;
    }

    for (size_t i = 0; i < n; ++i) {
        timeout = get_ticks() + 100 * 2000;
        while (!READ_BIT(I2C1->SR1, I2C_SR1_TXE) && get_ticks() < timeout) {
        }
        if (!(get_ticks() < timeout)) {
            if (READ_BIT(I2C1->SR1, I2C_SR1_AF)) {
                CLEAR_BIT(I2C1->SR1, I2C_SR1_AF);
            }
            SET_BIT(I2C1->CR1, I2C_CR1_STOP);
            return -EI2CTX;
        }

        {
            unsigned local_copy = buffer[i];
            WRITE_REG(I2C1->DR, local_copy);
        }

        if (READ_BIT(I2C1->SR1, I2C_SR1_BTF) && i + 1 < n) {
            {
                unsigned local_copy = buffer[++i];
                WRITE_REG(I2C1->DR, local_copy);
            }
        }

        timeout = get_ticks() + 100 * 2000;
        while (!READ_BIT(I2C1->SR1, I2C_SR1_BTF) && get_ticks() < timeout) {
        }
        if (!(get_ticks() < timeout)) {
            if (READ_BIT(I2C1->SR1, I2C_SR1_AF)) {
                CLEAR_BIT(I2C1->SR1, I2C_SR1_AF);
            }
            SET_BIT(I2C1->CR1, I2C_CR1_STOP);

            return -EI2CTX;
        }
    }

    if (with_stop) {
        SET_BIT(I2C1->CR1, I2C_CR1_STOP);
    }

    return 0;
}

static int
receive(unsigned char address, unsigned char *buffer, size_t n, int with_stop) {
    tick_t timeout = 0UL;

    CLEAR_BIT(I2C1->CR1, I2C_CR1_POS);

    SET_BIT(I2C1->CR1, I2C_CR1_START);

    timeout = get_ticks() + 100 * 2000;
    while (!READ_BIT(I2C1->SR1, I2C_SR1_SB) && get_ticks() < timeout) {
    }
    if (!(get_ticks() < timeout)) {
        SET_BIT(I2C1->CR1, I2C_CR1_STOP);
        return -EI2CSTART;
    }

    WRITE_REG(I2C1->DR, ((unsigned)address << 1U) | 1UL);

    timeout = get_ticks() + 100 * 2000;
    while (!READ_BIT(I2C1->SR1, I2C_SR1_ADDR) && get_ticks() < timeout) {
    }
    if (!(get_ticks() < timeout)) {
        if (READ_BIT(I2C1->SR1, I2C_SR1_AF)) {
            CLEAR_BIT(I2C1->SR1, I2C_SR1_AF);
        }
        SET_BIT(I2C1->CR1, I2C_CR1_STOP);
        return -EI2CACK;
    }

    if (n == 0UL) {
        (void)READ_REG(I2C1->SR2);
        if (with_stop) {
            SET_BIT(I2C1->CR1, I2C_CR1_STOP);
        }
    } else if (n == 1UL) {
        CLEAR_BIT(I2C1->CR1, I2C_CR1_ACK);
        (void)READ_REG(I2C1->SR2);
        if (with_stop) {
            SET_BIT(I2C1->CR1, I2C_CR1_STOP);
        }
    } else if (n == 2UL) {
        CLEAR_BIT(I2C1->CR1, I2C_CR1_ACK);
        SET_BIT(I2C1->CR1, I2C_CR1_POS);
        (void)READ_REG(I2C1->SR2);
    } else {
        SET_BIT(I2C1->CR1, I2C_CR1_ACK);
        (void)READ_REG(I2C1->SR2);
    }

    unsigned local_copy = 0U;
    for (size_t i = 0; i < n; ++i) {
        if (n - i == 1UL) {
            timeout = get_ticks() + 100 * 2000;
            while (!READ_BIT(I2C1->SR1, I2C_SR1_RXNE) &&
                   get_ticks() < timeout) {
            }
            if (!(get_ticks() < timeout)) {
                return -EI2CRX;
            }

            local_copy = READ_REG(I2C1->DR);
            buffer[i] = local_copy & 0xFFUL;
        } else if (n - i == 2UL) {
            timeout = get_ticks() + 100 * 2000;
            while (!READ_BIT(I2C1->SR1, I2C_SR1_BTF) && get_ticks() < timeout) {
            }
            if (!(get_ticks() < timeout)) {
                return -EI2CRX;
            }

            if (with_stop) {
                SET_BIT(I2C1->CR1, I2C_CR1_STOP);
            }

            local_copy = READ_REG(I2C1->DR);
            buffer[i] = local_copy & 0xFFUL;
            local_copy = READ_REG(I2C1->DR);
            buffer[++i] = local_copy & 0xFFUL;
        } else if (n - i == 3UL) {
            timeout = get_ticks() + 100 * 2000;
            while (!READ_BIT(I2C1->SR1, I2C_SR1_BTF) && get_ticks() < timeout) {
            }
            if (!(get_ticks() < timeout)) {
                return -EI2CRX;
            }

            CLEAR_BIT(I2C1->CR1, I2C_CR1_ACK);

            local_copy = READ_REG(I2C1->DR);
            buffer[i] = local_copy & 0xFFUL;

            timeout = get_ticks() + 100 * 2000;
            while (!READ_BIT(I2C1->SR1, I2C_SR1_BTF) && get_ticks() < timeout) {
            }
            if (!(get_ticks() < timeout)) {
                return -EI2CRX;
            }

            if (with_stop) {
                SET_BIT(I2C1->CR1, I2C_CR1_STOP);
            }

            local_copy = READ_REG(I2C1->DR);
            buffer[++i] = local_copy & 0xFFUL;
            local_copy = READ_REG(I2C1->DR);
            buffer[++i] = local_copy & 0xFFUL;
        } else {
            timeout = get_ticks() + 100 * 2000;
            while (!READ_BIT(I2C1->SR1, I2C_SR1_RXNE) &&
                   get_ticks() < timeout) {
            }
            if (!(get_ticks() < timeout)) {
                return -EI2CRX;
            }

            local_copy = READ_REG(I2C1->DR);
            buffer[i] = local_copy & 0xFFUL;

            if (READ_BIT(I2C1->SR1, I2C_SR1_BTF)) {
                if (n - i - 1 == 3UL) {
                    CLEAR_BIT(I2C1->CR1, I2C_CR1_ACK);
                }

                local_copy = READ_REG(I2C1->DR);
                buffer[++i] = local_copy & 0xFFUL;
            }
        }
    }

    return 0;
}
