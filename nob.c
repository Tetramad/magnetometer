#include <stddef.h>
#include <stdlib.h>

#define NOB_IMPLEMENTATION
#define NOB_WARN_DEPRECATED
#define NOB_STRIP_PREFIX

#include "nob_config.h"

#include "nob.h"

int main(int argc, char **argv) {
    NOB_GO_REBUILD_URSELF_PLUS(argc, argv, "nob_config.h");

    if (!mkdir_if_not_exists("Debug") || !mkdir_if_not_exists("Debug/Core") ||
        !mkdir_if_not_exists("Debug/Core/Src") ||
        !mkdir_if_not_exists("Debug/Core/Startup") ||
        !mkdir_if_not_exists("Debug/Drivers") ||
        !mkdir_if_not_exists("Debug/Drivers/STM32F4xx_HAL_Driver") ||
        !mkdir_if_not_exists("Debug/Drivers/STM32F4xx_HAL_Driver/Src")) {
        exit(EXIT_FAILURE);
    }

    Cmd cmd = {0};
    size_t mark = 0;

    nob_as(&cmd);
    nob_as_flags(&cmd);
    nob_as_output(&cmd, "Debug/Core/Startup/startup_stm32f411retx.o");
    nob_as_inputs(&cmd, "Core/Startup/startup_stm32f411retx.s");

    if (!cmd_run(&cmd)) {
        exit(EXIT_FAILURE);
    }

    const char *syssrc_names[] = {
        "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_adc",
        "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_adc_ex",
        "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal",
        "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cortex",
        "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma",
        "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma_ex",
        "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_exti",
        "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash",
        "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ex",
        "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ramfunc",
        "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_gpio",
        "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_i2c",
        "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_i2c_ex",
        "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr",
        "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr_ex",
        "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc",
        "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc_ex",
        "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim",
        "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim_ex",
        "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_adc",
        "Core/Src/syscalls",
        "Core/Src/sysmem",
        "Core/Src/system_stm32f4xx",
        "Core/Src/stm32f4xx_hal_msp",
        "Core/Src/stm32f4xx_it",
        "Core/Src/io",
    };

    mark = temp_save();
    for (const char **syssrc_name = &syssrc_names[0];
         syssrc_name <
         &syssrc_names[sizeof(syssrc_names) / sizeof(*syssrc_names)];
         ++syssrc_name) {

        const char *dst_path = temp_sprintf("%s/%s.o", "Debug", *syssrc_name);
        const char *src_path = temp_sprintf("%s.c", *syssrc_name);

        nob_cc(&cmd);
        nob_cc_flags(&cmd);
        cmd_append(&cmd,
                   "-Wno-switch-enum",
                   "-Wno-conversion",
                   "-Wno-unused-parameter");
        nob_cc_output(&cmd, dst_path);
        nob_cc_inputs(&cmd, src_path);

        if (!cmd_run(&cmd)) {
            temp_rewind(mark);
            exit(EXIT_FAILURE);
        }
    }
    temp_rewind(mark);

    const char *src_names[] = {
        "Core/Src/main",
        "Core/Src/log",
        "Core/Src/mode_selector",
        "Core/Src/lis2mdl",
        "Core/Src/stc3100",
    };

    mark = temp_save();
    for (const char **src_name = &src_names[0];
         src_name < &src_names[sizeof(src_names) / sizeof(*src_names)];
         ++src_name) {

        const char *dst_path = temp_sprintf("%s/%s.o", "Debug", *src_name);
        const char *src_path = temp_sprintf("%s.c", *src_name);

        nob_cc(&cmd);
        nob_cc_flags(&cmd);
        nob_cc_output(&cmd, dst_path);
        nob_cc_inputs(&cmd, src_path);

        if (!cmd_run(&cmd)) {
            temp_rewind(mark);
            exit(EXIT_FAILURE);
        }
    }
    temp_rewind(mark);

    nob_ld(&cmd);
    nob_ld_flags(&cmd);
    nob_ld_output(&cmd, "Debug/magnetometer.elf");
    nob_ld_inputs(&cmd, "Debug/Core/Startup/startup_stm32f411retx.o");

    mark = temp_save();
    for (const char **src_name = &src_names[0];
         src_name < &src_names[sizeof(src_names) / sizeof(*src_names)];
         ++src_name) {
        const char *dst_path = temp_sprintf("%s/%s.o", "Debug", *src_name);
        nob_ld_inputs(&cmd, dst_path);
    }

    for (const char **syssrc_name = &syssrc_names[0];
         syssrc_name <
         &syssrc_names[sizeof(syssrc_names) / sizeof(*syssrc_names)];
         ++syssrc_name) {
        const char *dst_path = temp_sprintf("%s/%s.o", "Debug", *syssrc_name);
        nob_ld_inputs(&cmd, dst_path);
    }

    if (!cmd_run(&cmd)) {
        temp_rewind(mark);
        exit(EXIT_FAILURE);
    }
    temp_rewind(mark);

    cmd_append(&cmd,
               "arm-none-eabi-objcopy",
               "-O",
               "ihex",
               "Debug/magnetometer.elf",
               "Debug/magnetometer.hex");
    if (!cmd_run(&cmd)) {
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
