#ifndef MACROS_H
#define MACROS_H

#define ARG_UNUSED(x) (void)(x)

#define ARRAY_SIZE(array) (sizeof((array)) / sizeof(*(array)))

#define FIRST_ARG(arg0, ...) arg0
#define SECOND_ARG(arg0, arg1, ...) arg1
#define REST_ARGS(arg0, ...) __VA_ARGS__

#endif /* MACROS_H */
