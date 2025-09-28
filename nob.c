#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define NOB_IMPLEMENTATION
#include "nob.h"

#define TOOLCHAIN_BIN_DIR "toolchain/bin/"
#define BUILD_DIR "build/"
#define SRC_DIR "src/"
#define CRT_DIR "toolchain/lib/gcc/arm-none-eabi/10.3.1/thumb/v7e-m+fp/hard/"

struct translation_unit {
    const char *src;
    const char *obj;
};

static int build(void);
static int objcopy(void);
static int flash(void);
static int clean(void);

int main(int argc, char **argv) {
    int err = 0;

    NOB_GO_REBUILD_URSELF(argc, argv);

    if (argc > 1) {
        (void)nob_shift(argv, argc);
        const char *command = nob_shift(argv, argc);

        if (strcmp(command, "build") == 0) {
            err = build();
            if (err < 0) {
                exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS);
        }

        if (strcmp(command, "objcopy") == 0) {
            err = objcopy();
            if (err < 0) {
                exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS);
        }

        if (strcmp(command, "flash") == 0) {
            err = flash();
            if (err < 0) {
                exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS);
        }

        if (strcmp(command, "clean") == 0) {
            err = clean();
            if (err < 0) {
                exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS);
        }

        nob_log(NOB_WARNING, "command \"%s\" not found.", command);
    } else {
        err = build();
        if (err < 0) {
            exit(EXIT_FAILURE);
        }

        err = objcopy();
        if (err < 0) {
            exit(EXIT_FAILURE);
        }

        err = flash();
        if (err < 0) {
            exit(EXIT_FAILURE);
        }

        exit(EXIT_SUCCESS);
    }
}

static int build(void) {
    if (!nob_mkdir_if_not_exists("build/")) {
        return -1;
    }

    Nob_Cmd cmd = {0};

    nob_cmd_append(&cmd,
                   "ccache",
                   "arm-none-eabi-as",
                   "-mcpu=cortex-m4",
                   "-mthumb",
                   "-mfloat-abi=hard",
                   "-mfpu=fpv4-sp-d16",
                   "-c",
                   "-o",
                   BUILD_DIR "startup.o",
                   SRC_DIR "startup.s");
    if (!nob_cmd_run_sync_and_reset(&cmd)) {
        return -1;
    }

    Nob_File_Paths sources = {0};
    nob_read_entire_dir("src", &sources);
    for (size_t i = 0; i < sources.count; ++i) {
        Nob_String_Builder src_path = {0};
        nob_sb_appendf(&src_path, "src/%s", sources.items[i]);

        if (nob_get_file_type(src_path.items) != NOB_FILE_REGULAR) {
            continue;
        }

        Nob_String_View sv = nob_sb_to_sv(src_path);

        if (!nob_sv_end_with(sv, ".c") && !nob_sv_end_with(sv, ".C")) {
            continue;
        }

        Nob_String_View chopped = nob_sv_from_cstr(sources.items[i]);
        chopped = nob_sv_chop_by_delim(&chopped, '.');

        Nob_String_Builder dst_path = {0};
        nob_sb_appendf(&dst_path, "build/" SV_Fmt ".o", SV_Arg(chopped));

        nob_cmd_append(&cmd,
                       "ccache",
                       "arm-none-eabi-gcc",
                       "-std=gnu11",
                       "-Wall",
                       "-Wextra",
                       "-Werror",
                       "-Wundef",
                       "-Wshadow",
                       "-Wdouble-promotion",
                       "-Wformat-truncation",
                       "-fno-common",
                       "-Wconversion",
                       "-g3",
                       "-O0",
                       "-ffunction-sections",
                       "-fdata-sections",
                       "-mcpu=cortex-m4",
                       "-mthumb",
                       "-mfloat-abi=hard",
                       "-mfpu=fpv4-sp-d16",
                       "-isystemstm32f411re/include/",
                       "-isystemtoolchain/arm-none-eabi/include/",
                       "-Iinclude",
                       "-c",
                       "-o",
                       dst_path.items,
                       src_path.items);
        if (!nob_cmd_run_sync_and_reset(&cmd)) {
            return -1;
        }
    }

    nob_cmd_append(
        &cmd,
        "ccache",
        "arm-none-eabi-gcc",
        "-std=gnu11",
        "-Wall",
        "-Wextra",
        "-Werror",
        "-Wundef",
        "-Wshadow",
        "-Wdouble-promotion",
        "-Wformat-truncation",
        "-fno-common",
        "-Wconversion",
        "-g3",
        "-O0",
        "-ffunction-sections",
        "-fdata-sections",
        "-mcpu=cortex-m4",
        "-mthumb",
        "-mfloat-abi=hard",
        "-mfpu=fpv4-sp-d16",
        "-Tstm32f411re/STM32F411RETX_FLASH.ld",
        "-nostartfiles",
        "-nostdlib",
        "--specs",
        "nosys.specs",
        "--specs",
        "nano.specs",
        "-Wl,--gc-sections",
        "-o",
        BUILD_DIR "main",
        BUILD_DIR "main.o",
        BUILD_DIR "startup.o",
        BUILD_DIR "sleep_and_wait.o",
        BUILD_DIR "log.o",
        BUILD_DIR "display_internal.o",
        BUILD_DIR "system.o",
        BUILD_DIR "gpio.o",
        BUILD_DIR "sensor.o",
        BUILD_DIR "i2c.o",
        BUILD_DIR "work_queue.o",
        BUILD_DIR "input.o",
        BUILD_DIR "tim3.o",
        BUILD_DIR "tmag5173.o",
        BUILD_DIR "lis2mdl.o",
        CRT_DIR "crtbegin.o",
        CRT_DIR "crtend.o",
        CRT_DIR "crti.o",
        CRT_DIR "crtn.o",
        CRT_DIR "crtfastmath.o",
        "toolchain/arm-none-eabi/lib/thumb/v7e-m+fp/hard/libc_nano.a",
        "toolchain/arm-none-eabi/lib/thumb/v7e-m+fp/hard/libg_nano.a",
        "toolchain/arm-none-eabi/lib/thumb/v7e-m+fp/hard/libm.a",
        "toolchain/arm-none-eabi/lib/thumb/v7e-m+fp/hard/libnosys.a");
    if (!nob_cmd_run_sync_and_reset(&cmd)) {
        return -1;
    }

    return 0;
}

static int objcopy(void) {
    Nob_Cmd objcopy = {0};

    nob_cmd_append(&objcopy,
                   "./toolchain/bin/arm-none-eabi-objcopy",
                   "-O",
                   "binary",
                   "./build/main",
                   "./build/firmware.bin");

    if (!nob_cmd_run_sync_and_reset(&objcopy)) {
        return -1;
    }
    return 0;
}

static int flash(void) {
    Nob_Cmd flash = {0};

    nob_cmd_append(&flash,
                   "st-flash",
                   "--reset",
                   "--connect-under-reset",
                   "write",
                   "./build/firmware.bin",
                   "0x8000000");

    if (!nob_cmd_run_sync_and_reset(&flash)) {
        return -1;
    }
    return 0;
}

static int clean(void) {
    Nob_Cmd clean = {0};

    nob_cmd_append(&clean, "rm", "-rf", "build");

    if (!nob_cmd_run_sync_and_reset(&clean)) {
        return -1;
    }

    nob_cmd_append(&clean, "rm", "-f", "nob.old", "compile_commands.json");

    if (!nob_cmd_run_sync_and_reset(&clean)) {
        return -1;
    }

    return 0;
}
