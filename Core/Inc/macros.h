/*
 * macros.h
 *
 */

#ifndef __MACROS_H
#define __MACROS_H

#define ARG_UNUSED(x) (void)(x)

#define ARRAY_SIZE(array) (sizeof((array)) / sizeof(*(array)))

#define LOOP(n) for (size_t _ = 0; _ < n; ++_)

#define FIRST_ARG(arg0, ...) arg0
#define SECOND_ARG(arg0, arg1, ...) arg1
#define REST_ARGS(arg0, ...) __VA_ARGS__

#define _CONCAT(a, b) a##b
#define CONCAT(a, b) _CONCAT(a, b)

#define _STRINGIFY(a) #a
#define STRINGIFY(a) _STRINGIFY(a)


#endif /* __MACROS_H */
