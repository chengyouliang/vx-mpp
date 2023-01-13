/*
 * 
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mpp_common.h"

static const OMX_U8 log2_tab[256] = {
    0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
};

OMX_S32 mpp_log2(OMX_U32 v)
{
    OMX_S32 n = 0;
    if (v & 0xffff0000) {
        v >>= 16;
        n += 16;
    }
    if (v & 0xff00) {
        v >>= 8;
        n += 8;
    }
    n += log2_tab[v];

    return n;
}

OMX_S32 mpp_log2_16bit(OMX_U32 v)
{
    OMX_S32 n = 0;
    if (v & 0xff00) {
        v >>= 8;
        n += 8;
    }
    n += log2_tab[v];

    return n;
}

OMX_S32 axb_div_c(OMX_S32 a, OMX_S32 b, OMX_S32 c)
{
    OMX_U32 left = 32;
    OMX_U32 right = 0;
    OMX_U32 shift;
    OMX_S32 sign = 1;
    OMX_S32 tmp;

    if (a == 0 || b == 0)
        return 0;
    else if ((a * b / b) == a && c != 0)
        return (a * b / c);

    if (a < 0) {
        sign = -1;
        a = -a;
    }
    if (b < 0) {
        sign *= -1;
        b = -b;
    }
    if (c < 0) {
        sign *= -1;
        c = -c;
    }

    if (c == 0)
        return 0x7FFFFFFF * sign;

    if (b > a) {
        tmp = b;
        b = a;
        a = tmp;
    }

    for (--left; (((OMX_U32)a << left) >> left) != (OMX_U32)a; --left)
        ;

    left--;

    while (((OMX_U32)b >> right) > (OMX_U32)c)
        right++;

    if (right > left) {
        return 0x7FFFFFFF * sign;
    } else {
        shift = left - right;
        return (OMX_S32)((((OMX_U32)a << shift) /
                         (OMX_U32)c * (OMX_U32)b) >> shift) * sign;
    }
}
