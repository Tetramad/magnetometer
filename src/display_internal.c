#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <stm32f4xx.h>

#include <input.h>
#include <log.h>
#include <macros.h>
#include <sleep_and_wait.h>
#include <work_queue.h>

extern signed magnetic_field_magnitude_mt;
extern signed magnetic_field_x_axis_mt;
extern signed magnetic_field_y_axis_mt;
extern signed magnetic_field_z_axis_mt;
extern signed temperature_celsius;

static bool check_busy_flag(void);
static void command_initial(uint8_t i);
static void command(uint8_t i);
static int display_string(const char *str);
static int display_mag_mag(void *payload);
static int display_mag_x(void *payload);
static int display_mag_y(void *payload);
static int display_mag_z(void *payload);
static int display_temperature(void *payload);

int (*displayables[])(void *) = {display_mag_mag,
                                 display_mag_x,
                                 display_mag_y,
                                 display_mag_z,
                                 display_temperature};
size_t displayables_index = 0U;

struct input_context input_context = {0};

LOG_LEVEL_SET(LOG_LEVEL_INF);

void TIM4_IRQHandler(void) {
    int err = 0;

    LOG_DBG("TIM4: IRQ detected");

    err = input_update_context(&input_context);
    if (err < 0) {
        LOG_ERR("TIM4: failed to update input");
    } else {
        if (input_context.left == 1 && input_context.right != 1) {
            displayables_index =
                (displayables_index + (ARRAY_SIZE(displayables) - 1)) %
                ARRAY_SIZE(displayables);
        }
        if (input_context.right == 1 && input_context.left != 1) {
            displayables_index =
                (displayables_index + 1) % ARRAY_SIZE(displayables);
        }
    }

    err = work_queue_enqueue(WORK_PRIORITY_DEFAULT,
                             displayables[displayables_index],
                             NULL);
    if (err < 0) {
        LOG_ERR("TIM4: failed to enqeue work");
    }

    NVIC_ClearPendingIRQ(TIM4_IRQn);
    CLEAR_BIT(TIM4->SR, TIM_SR_UIF);
}

int display_internal_init(void *payload) {
    ARG_UNUSED(payload);

    SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOAEN);

    SET_BIT(GPIOA->BSRR, GPIO_BSRR_BR1);
    SET_BIT(GPIOA->BSRR, GPIO_BSRR_BR2);
    SET_BIT(GPIOA->BSRR, GPIO_BSRR_BR3);
    SET_BIT(GPIOA->BSRR, GPIO_BSRR_BR4);
    SET_BIT(GPIOA->BSRR, GPIO_BSRR_BR5);
    SET_BIT(GPIOA->BSRR, GPIO_BSRR_BR6);
    SET_BIT(GPIOA->BSRR, GPIO_BSRR_BR7);

    MODIFY_REG(GPIOA->MODER, GPIO_MODER_MODER1, GPIO_MODER_MODER1_0);
    MODIFY_REG(GPIOA->MODER, GPIO_MODER_MODER2, GPIO_MODER_MODER2_0);
    MODIFY_REG(GPIOA->MODER, GPIO_MODER_MODER3, GPIO_MODER_MODER3_0);
    MODIFY_REG(GPIOA->MODER, GPIO_MODER_MODER4, GPIO_MODER_MODER4_0);
    MODIFY_REG(GPIOA->MODER, GPIO_MODER_MODER5, GPIO_MODER_MODER5_0);
    MODIFY_REG(GPIOA->MODER, GPIO_MODER_MODER6, GPIO_MODER_MODER6_0);
    MODIFY_REG(GPIOA->MODER, GPIO_MODER_MODER7, GPIO_MODER_MODER7_0);

    busy_wait_ms(40);

    command_initial(0x30);
    command_initial(0x30);
    command_initial(0x30);
    command_initial(0x20);

    command(0x28);
    busy_wait_us(37);
    command(0x0C);
    busy_wait_us(37);
    command(0x01);
    busy_wait_us(1520);
    command(0x06);
    busy_wait_us(37);

    SET_BIT(RCC->APB1ENR, RCC_APB1ENR_TIM4EN);

    /*
     * 5Hz Update Interrupt
     * given PSC = 1599
     * ARR = 16000000 Hz / (1599 + 1) / 5 Hz
     *     = 2000 <= 65535
     */
    WRITE_REG(TIM4->PSC, 1599);
    WRITE_REG(TIM4->ARR, 2000);
    WRITE_REG(TIM4->CNT, 2000);
    SET_BIT(TIM4->CR1, TIM_CR1_DIR);
    SET_BIT(TIM4->DIER, TIM_DIER_UIE);

    NVIC_SetPriority(TIM4_IRQn, (1UL << __NVIC_PRIO_BITS) - 2UL);
    NVIC_EnableIRQ(TIM4_IRQn);

    SET_BIT(TIM4->CR1, TIM_CR1_CEN);

    input_context = input_new_context();

    LOG_INF("DISPLAY: initialize succeeded");

    return 0;
}

static bool check_busy_flag(void) {
    SET_BIT(GPIOA->BSRR, GPIO_BSRR_BR4);
    SET_BIT(GPIOA->BSRR, GPIO_BSRR_BR5);
    SET_BIT(GPIOA->BSRR, GPIO_BSRR_BR6);
    SET_BIT(GPIOA->BSRR, GPIO_BSRR_BR7);
    MODIFY_REG(GPIOA->MODER, GPIO_MODER_MODER4, 0);
    MODIFY_REG(GPIOA->MODER, GPIO_MODER_MODER5, 0);
    MODIFY_REG(GPIOA->MODER, GPIO_MODER_MODER6, 0);
    MODIFY_REG(GPIOA->MODER, GPIO_MODER_MODER7, 0);

    SET_BIT(GPIOA->BSRR, GPIO_BSRR_BR1);
    SET_BIT(GPIOA->BSRR, GPIO_BSRR_BS2);
    busy_wait_us(0);
    SET_BIT(GPIOA->BSRR, GPIO_BSRR_BS3);
    busy_wait_us(1);

    const bool busy_flag = !!READ_BIT(GPIOA->IDR, GPIO_IDR_ID7);

    busy_wait_us(1);
    SET_BIT(GPIOA->BSRR, GPIO_BSRR_BR3);

    busy_wait_us(2);

    SET_BIT(GPIOA->BSRR, GPIO_BSRR_BR1);
    SET_BIT(GPIOA->BSRR, GPIO_BSRR_BS2);
    busy_wait_us(0);
    SET_BIT(GPIOA->BSRR, GPIO_BSRR_BS3);
    busy_wait_us(1);

    __NOP();

    busy_wait_us(1);
    SET_BIT(GPIOA->BSRR, GPIO_BSRR_BR3);

    busy_wait_us(2);

    SET_BIT(GPIOA->BSRR, GPIO_BSRR_BR1);
    SET_BIT(GPIOA->BSRR, GPIO_BSRR_BR2);
    SET_BIT(GPIOA->BSRR, GPIO_BSRR_BR4);
    SET_BIT(GPIOA->BSRR, GPIO_BSRR_BR5);
    SET_BIT(GPIOA->BSRR, GPIO_BSRR_BR6);
    SET_BIT(GPIOA->BSRR, GPIO_BSRR_BR7);
    MODIFY_REG(GPIOA->MODER, GPIO_MODER_MODER4, GPIO_MODER_MODER4_0);
    MODIFY_REG(GPIOA->MODER, GPIO_MODER_MODER5, GPIO_MODER_MODER5_0);
    MODIFY_REG(GPIOA->MODER, GPIO_MODER_MODER6, GPIO_MODER_MODER6_0);
    MODIFY_REG(GPIOA->MODER, GPIO_MODER_MODER7, GPIO_MODER_MODER7_0);

    return busy_flag;
}

static void command_initial(uint8_t i) {
    SET_BIT(GPIOA->BSRR, GPIO_BSRR_BR1);
    SET_BIT(GPIOA->BSRR, GPIO_BSRR_BR2);
    busy_wait_us(0);
    SET_BIT(GPIOA->BSRR, GPIO_BSRR_BS3);
    busy_wait_us(1);
    SET_BIT(GPIOA->BSRR, i & (1 << 4) ? GPIO_BSRR_BS4 : GPIO_BSRR_BR4);
    SET_BIT(GPIOA->BSRR, i & (1 << 5) ? GPIO_BSRR_BS5 : GPIO_BSRR_BR5);
    SET_BIT(GPIOA->BSRR, i & (1 << 6) ? GPIO_BSRR_BS6 : GPIO_BSRR_BR6);
    SET_BIT(GPIOA->BSRR, i & (1 << 7) ? GPIO_BSRR_BS7 : GPIO_BSRR_BR7);
    busy_wait_us(1);
    SET_BIT(GPIOA->BSRR, GPIO_BSRR_BR3);
    busy_wait_us(1);
    SET_BIT(GPIOA->BSRR, GPIO_BSRR_BR4);
    SET_BIT(GPIOA->BSRR, GPIO_BSRR_BR5);
    SET_BIT(GPIOA->BSRR, GPIO_BSRR_BR6);
    SET_BIT(GPIOA->BSRR, GPIO_BSRR_BR7);

    busy_wait_us(160);
}

static void command(uint8_t i) {
    for (size_t _ = 0; _ < 8 && check_busy_flag(); ++_) {
        busy_wait_us(37);
    }

    SET_BIT(GPIOA->BSRR, GPIO_BSRR_BR1);
    SET_BIT(GPIOA->BSRR, GPIO_BSRR_BR2);
    busy_wait_us(0);
    SET_BIT(GPIOA->BSRR, GPIO_BSRR_BS3);
    busy_wait_us(1);
    SET_BIT(GPIOA->BSRR, i & (1 << 4) ? GPIO_BSRR_BS4 : GPIO_BSRR_BR4);
    SET_BIT(GPIOA->BSRR, i & (1 << 5) ? GPIO_BSRR_BS5 : GPIO_BSRR_BR5);
    SET_BIT(GPIOA->BSRR, i & (1 << 6) ? GPIO_BSRR_BS6 : GPIO_BSRR_BR6);
    SET_BIT(GPIOA->BSRR, i & (1 << 7) ? GPIO_BSRR_BS7 : GPIO_BSRR_BR7);
    busy_wait_us(1);
    SET_BIT(GPIOA->BSRR, GPIO_BSRR_BR3);

    busy_wait_us(2);

    i <<= 4;

    SET_BIT(GPIOA->BSRR, GPIO_BSRR_BS3);
    busy_wait_us(1);
    SET_BIT(GPIOA->BSRR, i & (1 << 4) ? GPIO_BSRR_BS4 : GPIO_BSRR_BR4);
    SET_BIT(GPIOA->BSRR, i & (1 << 5) ? GPIO_BSRR_BS5 : GPIO_BSRR_BR5);
    SET_BIT(GPIOA->BSRR, i & (1 << 6) ? GPIO_BSRR_BS6 : GPIO_BSRR_BR6);
    SET_BIT(GPIOA->BSRR, i & (1 << 7) ? GPIO_BSRR_BS7 : GPIO_BSRR_BR7);
    busy_wait_us(1);
    SET_BIT(GPIOA->BSRR, GPIO_BSRR_BR3);

    busy_wait_us(2);
}

static void write(char c) {
    for (size_t i = 0; i < 8 && check_busy_flag(); ++i) {
        busy_wait_us(37);
    }

    SET_BIT(GPIOA->BSRR, GPIO_BSRR_BS1);
    SET_BIT(GPIOA->BSRR, GPIO_BSRR_BR2);
    busy_wait_us(0);
    SET_BIT(GPIOA->BSRR, GPIO_BSRR_BS3);
    busy_wait_us(1);
    SET_BIT(GPIOA->BSRR, c & (1 << 4) ? GPIO_BSRR_BS4 : GPIO_BSRR_BR4);
    SET_BIT(GPIOA->BSRR, c & (1 << 5) ? GPIO_BSRR_BS5 : GPIO_BSRR_BR5);
    SET_BIT(GPIOA->BSRR, c & (1 << 6) ? GPIO_BSRR_BS6 : GPIO_BSRR_BR6);
    SET_BIT(GPIOA->BSRR, c & (1 << 7) ? GPIO_BSRR_BS7 : GPIO_BSRR_BR7);
    busy_wait_us(1);
    SET_BIT(GPIOA->BSRR, GPIO_BSRR_BR3);

    busy_wait_us(2);

    c <<= 4;

    SET_BIT(GPIOA->BSRR, GPIO_BSRR_BS3);
    busy_wait_us(1);
    SET_BIT(GPIOA->BSRR, c & (1 << 4) ? GPIO_BSRR_BS4 : GPIO_BSRR_BR4);
    SET_BIT(GPIOA->BSRR, c & (1 << 5) ? GPIO_BSRR_BS5 : GPIO_BSRR_BR5);
    SET_BIT(GPIOA->BSRR, c & (1 << 6) ? GPIO_BSRR_BS6 : GPIO_BSRR_BR6);
    SET_BIT(GPIOA->BSRR, c & (1 << 7) ? GPIO_BSRR_BS7 : GPIO_BSRR_BR7);
    busy_wait_us(1);
    SET_BIT(GPIOA->BSRR, GPIO_BSRR_BR3);

    busy_wait_us(2);

    SET_BIT(GPIOA->BSRR, GPIO_BSRR_BR1);
    SET_BIT(GPIOA->BSRR, GPIO_BSRR_BR2);
}

static int display_string(const char *str) {
    command(0x01);
    busy_wait_us(1520);
    command(0x80);
    busy_wait_us(37);
    for (size_t i = 0; str[i]; ++i) {
        write(str[i]);
        busy_wait_us(37);
    }

    return 0;
}

static int display_mag_mag(void *payload) {
    ARG_UNUSED(payload);

    char buffer[32] = "";

    if (magnetic_field_magnitude_mt > 999999 ||
        magnetic_field_magnitude_mt < -999999) {
        sprintf(buffer,
                "M%cOvrflw",
                magnetic_field_magnitude_mt > 0 ? '+' : '-');
    } else {
        sprintf(buffer,
                "M% 4d.%03d",
                magnetic_field_magnitude_mt / 1000,
                abs(magnetic_field_magnitude_mt % 1000));
    }

    display_string(buffer);

    return 0;
}

static int display_mag_x(void *payload) {
    ARG_UNUSED(payload);

    char buffer[32] = "";

    if (magnetic_field_x_axis_mt > 99999 || magnetic_field_x_axis_mt < -99999) {
        sprintf(buffer, "X%cOvrflw", magnetic_field_x_axis_mt > 0 ? '+' : '-');
    } else {
        sprintf(buffer,
                "X% 3d.%03d",
                magnetic_field_x_axis_mt / 1000,
                abs(magnetic_field_x_axis_mt % 1000));
    }

    display_string(buffer);

    return 0;
}

static int display_mag_y(void *payload) {
    ARG_UNUSED(payload);

    char buffer[32] = "";

    if (magnetic_field_y_axis_mt > 99999 || magnetic_field_y_axis_mt < -99999) {
        sprintf(buffer, "Y%cOvrflw", magnetic_field_y_axis_mt > 0 ? '+' : '-');
    } else {
        sprintf(buffer,
                "Y% 3d.%03d",
                magnetic_field_y_axis_mt / 1000,
                abs(magnetic_field_y_axis_mt % 1000));
    }

    display_string(buffer);

    return 0;
}

static int display_mag_z(void *payload) {
    ARG_UNUSED(payload);

    char buffer[32] = "";

    if (magnetic_field_z_axis_mt > 99999 || magnetic_field_z_axis_mt < -99999) {
        sprintf(buffer, "Z%cOvrflw", magnetic_field_z_axis_mt > 0 ? '+' : '-');
    } else {
        sprintf(buffer,
                "Z% 3d.%03d",
                magnetic_field_z_axis_mt / 1000,
                abs(magnetic_field_z_axis_mt % 1000));
    }

    display_string(buffer);

    return 0;
}

static int display_temperature(void *payload) {
    ARG_UNUSED(payload);

    char buffer[32] = "";

    if (temperature_celsius > 999999 || temperature_celsius < -999999) {
        sprintf(buffer, "T%cOvrflw", temperature_celsius > 0 ? '+' : '-');
    } else {
        sprintf(buffer,
                "T% 4d.%02d",
                temperature_celsius / 1000,
                abs(temperature_celsius % 1000) / 10);
    }

    display_string(buffer);

    return 0;
}
