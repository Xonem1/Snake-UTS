/*
   Copyright 2017 Andrew Cassidy

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
 */

#ifndef ISQRT_H
#define ISQRT_H

#include <stdint.h>

uint16_t isqrt(uint16_t a_nInput) {
    uint16_t op = a_nInput;
    uint16_t res = 0;
    uint16_t one = 1uL << 14; // The second-to-top bit is set: use 1u << 14 for uint16_t type; use 1uL<<30 for uint32_t type

    // "one" starts at the highest power of four <= than the argument.
    while (one > op) {
        one >>= 2;
    }

    while (one != 0) {
        if (op >= res + one) {
            op = op - (res + one);
            res = res + 2 * one;
        }
        res >>= 1;
        one >>= 2;
    }

    /* Do arithmetic rounding to nearest integer */
    if (op > res) {
        res++;
    }

    return res;
}

#endif

