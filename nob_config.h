#ifndef NOB_CONFIG_H_
#endif /* NOB_CONFIG_H_ */

#define nob_as(cmd) nob_cmd_append(cmd, "ccache", "arm-none-eabi-gcc")
#define nob_as_flags(cmd)                                                      \
    nob_cmd_append(cmd,                                                        \
                   "-mcpu=cortex-m4",                                          \
                   "-g3",                                                      \
                   "-DDEBUG",                                                  \
                   "-c",                                                       \
                   "--language=assembler-with-cpp",                            \
                   "--specs=nano.specs",                                       \
                   "-mfpu=fpv4-sp-d16",                                        \
                   "-mfloat-abi=hard",                                         \
                   "-mthumb")
#define nob_as_output(cmd, output_path) nob_cmd_append(cmd, "-o", (output_path))
#define nob_as_inputs(cmd, ...) nob_cmd_append(cmd, __VA_ARGS__)

#define nob_cc(cmd) nob_cmd_append(cmd, "ccache", "arm-none-eabi-gcc")
#define nob_cc_flags(cmd)                                                      \
    nob_cmd_append(cmd,                                                        \
                   "-mcpu=cortex-m4",                                          \
                   "-std=gnu11",                                               \
                   "-g3",                                                      \
                   "-DDEBUG",                                                  \
                   "-DUSE_HAL_DRIVER",                                         \
                   "-DSTM32F411xE",                                            \
                   "-c",                                                       \
                   "-ICore/Inc",                                               \
                   "-Og",                                                      \
                   "-ffunction-sections",                                      \
                   "-fdata-sections",                                          \
                   "-Wall",                                                    \
                   "-Wextra",                                                  \
                   "-Werror",                                                  \
                   "-Wswitch-enum",                                            \
                   "-Wconversion",                                             \
                   "-isystemDrivers/STM32F4xx_HAL_Driver/Inc",                 \
                   "-isystemDrivers/STM32F4xx_HAL_Driver/Inc/Legacy",          \
                   "-isystemDrivers/CMSIS/Device/ST/STM32F4xx/Include",        \
                   "-isystemDrivers/CMSIS/Include",                            \
                   "-fstack-usage",                                            \
                   "--specs=nano.specs",                                       \
                   "-mfpu=fpv4-sp-d16",                                        \
                   "-mfloat-abi=hard",                                         \
                   "-mthumb")
#define nob_cc_output(cmd, output_path) nob_cmd_append(cmd, "-o", (output_path))
#define nob_cc_inputs(cmd, ...) nob_cmd_append(cmd, __VA_ARGS__)

#define nob_ld(cmd) nob_cmd_append(cmd, "arm-none-eabi-gcc")
#define nob_ld_flags(cmd)                                                      \
    nob_cmd_append(                                                            \
        cmd,                                                                   \
        "-mcpu=cortex-m4",                                                     \
        "-TSTM32F411RETX_FLASH.ld",       \
        "--specs=nosys.specs",                                                 \
        "-Wl,--gc-sections",                                                   \
        "-static",                                                             \
        "--specs=nano.specs",                                                  \
        "-mfpu=fpv4-sp-d16",                                                   \
        "-mfloat-abi=hard",                                                    \
        "-mthumb",                                                             \
        "-Wl,--start-group",                                                   \
        "-lc",                                                                 \
        "-lm",                                                                 \
        "-Wl,--end-group")
#define nob_ld_output(cmd, output_path) nob_cmd_append(cmd, "-o", (output_path))
#define nob_ld_inputs(cmd, ...) nob_cmd_append(cmd, __VA_ARGS__)

#ifdef NOB_STRIP_PREFIX
#define as nob_as
#define as_flags nob_as_flags
#define as_output nob_as_output
#define as_inputs nob_as_inputs
#define cc nob_cc
#define cc_flags nob_cc_flags
#define cc_output nob_cc_output
#define cc_inputs nob_cc_inputs
#define ld nob_ld
#define ld_flags nob_ld_flags
#define ld_output nob_ld_output
#define ld_inputs nob_ld_inputs
#define ld nob_ld
#endif // NOB_STRIP_PREFIX
