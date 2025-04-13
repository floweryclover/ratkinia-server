//
// Created by floweryclover on 2025-04-12.
//

#ifndef RATKINIASERVER_ERRORS_H
#define RATKINIASERVER_ERRORS_H

#include "MessagePrinter.h"

#define ERR_PRINT_VARARGS(...)                                                                  \
    MessagePrinter::WriteErrorLine(__FUNCTION__, __FILE__, __LINE__, __VA_ARGS__)               \


#define ERR_FAIL_COND(cond)                                                    \
    if ((cond))                                                                \
    {                                                                          \
        ERR_PRINT_VARARGS("에러 조건문", cond, "(이)가 true였습니다.");        \
    }                                                                          \
    else                                                                       \
        ((void*)0)

#endif //RATKINIASERVER_ERRORS_H
