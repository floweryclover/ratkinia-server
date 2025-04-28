//
// Created by floweryclover on 2025-04-12.
//

#ifndef RATKINIASERVER_ERRORS_H
#define RATKINIASERVER_ERRORS_H

#include "MessagePrinter.h"

#define ERR_PRINT(msg) \
    MessagePrinter::WriteErrorLine(__FUNCTION__, __FILE__, __LINE__, msg)

#define ERR_PRINT_VARARGS(...)                                                                  \
    MessagePrinter::WriteErrorLine(__FUNCTION__, __FILE__, __LINE__, __VA_ARGS__)               \

#define ERR_FAIL_COND(cond)                                                                                                    \
    if (!!(cond))                                                                                                         \
    {                                                                                                                           \
        ERR_PRINT_VARARGS(__FUNCTION__, " ", __FILE__, " ", __LINE__, " Condition \"" , #cond, "\" is true.");                        \
        return;                                                                                                                 \
    } else                                                                                                                      \
        ((void)0)

#define ERR_FAIL_COND_V(cond, retval)                                                                                                   \
    if (!!(cond)) {                                                                                                                \
        ERR_PRINT_VARARGS(__FUNCTION__, " ", __FILE__, " ", __LINE__, " Condition \"", #cond, "\" is true. Returning: ", #retval);          \
        return retval;                                                                                                                   \
    } else                                                                                                                               \
        ((void)0)

#define ERR_FAIL_NULL_V(param, retval)                                                                                                          \
    if (!!(!(param))) {                                                                                                               \
        ERR_PRINT_VARARGS(__FUNCTION__, " ", __FILE__, " ", __LINE__, " ", " Parameter \"", #param, "\" is null. Returning: ", #retval);               \
        return retval;                                                                                                                  \
    } else                                                                                                                               \
        ((void)0)

#define ERR_CONTINUE_COND(cond)                                                                                                    \
    if (!!(cond))                                                                                                         \
    {                                                                                                                           \
        ERR_PRINT_VARARGS(__FUNCTION__, " ", __FILE__, " ", __LINE__, " ", " Condition \"", #cond, "\" is true.");                        \
        continue;                                                                                                                 \
    } else                                                                                                                      \
        ((void)0)

#define ERR_CONTINUE_NULL(param)                                                                                                    \
    if (!!(!(param)))                                                                                                         \
    {                                                                                                                           \
        ERR_PRINT_VARARGS(__FUNCTION__, " ", __FILE__, " ", __LINE__, " ", " Parameter \"", #param, "\"  is null. Continuing.");                        \
        continue;                                                                                                                 \
    } else                                                                                                                      \
        ((void)0)

#define ERR_CONTINUE_MSG(cond, msg)                                                                                            \
    if (!!(cond)) {                                                                                                        \
        ERR_PRINT_VARARGS(__FUNCTION__, " ", __FILE__, " ", __LINE__, " ", " Condition \"" #cond "\" is true. Continuing.", msg); \
        continue;                                                                                                                  \
    } else                                                                                                                         \
        ((void)0)

#define ERR_FAIL_NULL(param)                                                                                                   \
    if (!!(!(param)))                                                                                             \
    {                                                                                                                           \
        ERR_PRINT_VARARGS(__FUNCTION__, " ", __FILE__, " ", __LINE__, "Parameter \"" #param "\" is null.");                         \
        return;                                                                                                                 \
    } else                                                                                                                      \
        ((void)0)

#endif //RATKINIASERVER_ERRORS_H
