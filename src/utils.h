#ifndef _UTILS_H
#define _UTILS_H

#include "os.h"
#include <stdbool.h>
#include <stdint.h>

extern uint32_t atoi(const char*);
extern uint64_t atol(const char*);
extern bool adjust_decimals(char *src, uint32_t srcLength, char *target,
                    uint32_t targetLength, uint8_t decimals);

#endif
