//
// Created by floweryclover on 2025-04-12.
//

#ifndef RATKINIASERVER_ERRORS_H
#define RATKINIASERVER_ERRORS_H

#include "MessagePrinter.h"

#define _STR(x) #x

#define _MKSTR(x) _STR(x)

#define ERR_PRINT(msg) \
    MessagePrinter::WriteErrorLine("Function: ", __FUNCTION__, " at ", __FILE__, " line: ", __LINE__, ", Message: ", msg)

#define ERR_PRINT_VARARGS(...)                                                                  \
    MessagePrinter::WriteErrorLine("Function: ", __FUNCTION__, " at ", __FILE__, " line: ", __LINE__, " ", __VA_ARGS__)               \

#define ERR_FAIL_COND(cond)                                                                                                    \
    if (!!(cond))                                                                                                         \
    {                                                                                                                           \
        ERR_PRINT_VARARGS("Condition \"" , _MKSTR(cond), "\" is true.");                        \
        return;                                                                                                                 \
    } else                                                                                                                      \
        ((void)0)

#define ERR_FAIL_COND_V(cond, retval)                                                                                                   \
    if (!!(cond)) {                                                                                                                \
        ERR_PRINT_VARARGS("Condition \"", _MKSTR(cond), "\" is true. Returning: ", _MKSTR(retval);          \
        return retval;                                                                                                                   \
    } else                                                                                                                               \
        ((void)0)

#define ERR_FAIL_COND_V_MSG(cond, retval, msg)                                                                                                   \
    if (!!(cond)) {                                                                                                                \
        ERR_PRINT_VARARGS("Condition \"", _MKSTR(cond), "\" is true. Returning: ", _MKSTR(retval), ". Message: ", msg);          \
        return retval;                                                                                                                   \
    }\
    else                                                                                                                               \
        ((void)0)

#define ERR_FAIL_NULL_V(param, retval)                                                                                                          \
    if (!!(!(param))) {                                                                                                               \
        ERR_PRINT_VARARGS("Parameter \"", _MKSTR(param), "\" is null. Returning: ", _MKSTR(retval));               \
        return retval;                                                                                                                  \
    } else                                                                                                                               \
        ((void)0)

#define ERR_CONTINUE_COND(cond)                                                                                                    \
    if (!!(cond))                                                                                                         \
    {                                                                                                                           \
        ERR_PRINT_VARARGS("Condition \"", _MKSTR(cond), "\" is true.");                        \
        continue;                                                                                                                 \
    } else                                                                                                                      \
        ((void)0)

#define ERR_CONTINUE_NULL(param)                                                                                                    \
    if (!!(!(param)))                                                                                                         \
    {                                                                                                                           \
        ERR_PRINT_VARARGS("Parameter \"", _MKSTR(param), "\"  is null. Continuing.");                        \
        continue;                                                                                                                 \
    } else                                                                                                                      \
        ((void)0)

#define ERR_CONTINUE_MSG(cond, msg)                                                                                            \
    if (!!(cond)) {                                                                                                        \
        ERR_PRINT_VARARGS("Condition \"", _MKSTR(cond), "\" is true. Continuing. Message: ", msg); \
        continue;                                                                                                                  \
    } else                                                                                                                         \
        ((void)0)

#define ERR_FAIL_NULL(param)                                                                                                   \
    if (!!(!(param)))                                                                                             \
    {                                                                                                                           \
        ERR_PRINT_VARARGS("Parameter \"", _MKSTR(param), "\" is null.");                         \
        return;                                                                                                                 \
    } else                                                                                                                      \
        ((void)0)

#define CRASH_NOW_MSG(msg) \
    do  \
    {   \
        ERR_PRINT(msg); \
        std::abort();   \
    }   \
    while (0)   \

#define CRASH_COND(cond)                                                                \
    if (!!(cond))                                                                       \
    {                                                                                   \
        ERR_PRINT_VARARGS("FATAL: Condition \"", _MKSTR(cond), "\" is true.");          \
        std::abort();                                                                   \
    }                                                                                   \
    else                                                                                \
        ((void)0)

#define CRASH_COND_MSG(cond, msg)                                                        \
    if (!!(cond))                                                                       \
    {                                                                                   \
        ERR_PRINT_VARARGS("FATAL: Condition \"", _MKSTR(cond), "\" is true. Message: ", msg);          \
        std::abort();                                                                   \
    }                                                                                   \
    else                                                                                \
        ((void)0)

#endif //RATKINIASERVER_ERRORS_H
