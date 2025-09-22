#ifndef I2C_H
#define I2C_H

#include <errno.h>
#include <stddef.h>

#include <gpio.h>
#include <macros.h>

#define I2C_ERRNO_BASE 0x5400
#define EI2CIO 0x5401
#define EI2CSTART 0x5402
#define EI2CACK 0x5403
#define EI2CTX 0x5404
#define EI2CRX 0x5405

#define TRANSACTION_TYPE_WRITE 0
#define TRANSACTION_TYPE_READ 1

struct i2c_config {
    struct gpio_config scl;
    struct gpio_config sda;
};

struct i2c_transaction_data {
    int type;
    unsigned char *buffer;
    size_t size;
};

int i2c_configure(void);
int i2c_transaction(unsigned char address,
                    const struct i2c_transaction_data *data,
                    size_t n);

static inline int standard_reg_read(unsigned char address,
                                    unsigned char reg,
                                    unsigned char *buffer,
                                    size_t n) {
    unsigned char register_buffer = reg;
    const struct i2c_transaction_data transactions[2] = {
        [0] = {.type = TRANSACTION_TYPE_WRITE,
               .buffer = &register_buffer,
               .size = 1U},
        [1] = {.type = TRANSACTION_TYPE_READ, .buffer = buffer, .size = n}};

    return i2c_transaction(address, transactions, ARRAY_SIZE(transactions));
}

static inline int
standard_write(unsigned char address, unsigned char *buffer, size_t n) {
    const struct i2c_transaction_data transaction = {.type =
                                                         TRANSACTION_TYPE_WRITE,
                                                     .buffer = buffer,
                                                     .size = n};

    return i2c_transaction(address, &transaction, 1U);
}

#endif /* I2C_H */
