/*
 * Copyright 2022 
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
#ifndef _VX_LOG__H_
#define _VX_LOG__H_

#include <stdio.h>
#include <stdlib.h>

#include "vx_type.h"

#define MODULE_TAG "mpp_log"



void _vx_log(const char *tag, const char *fmt, const char *func, ...);
void _vx_err(const char *tag, const char *fmt, const char *func, ...);

/*
 * vx runtime log system usage:
 * vx_err is for error status message, it will print for sure.
 * vx_log is for important message like open/close/reset/flush, it will print too.
 * vx_dbg is for all optional message. it can be controlled by debug and flag.
 */

#define vx_log(fmt, ...)   _vx_log(MODULE_TAG, fmt, NULL, ## __VA_ARGS__)
#define vx_err(fmt, ...)   _vx_err(MODULE_TAG, fmt, NULL, ## __VA_ARGS__)

#define _vx_dbg(debug, flag, fmt, ...) \
             do { \
                if (debug & flag) \
                    vx_log(fmt, ## __VA_ARGS__); \
             } while (0)

#define vx_dbg(flag, fmt, ...) _vx_dbg(mpp_debug, flag, fmt, ## __VA_ARGS__)

/*
 * _f function will add function name to the log
 */
#define vx_log_f(fmt, ...)  _vx_log(MODULE_TAG, fmt, __FUNCTION__, ## __VA_ARGS__)
#define vx_err_f(fmt, ...)  _vx_err(MODULE_TAG, fmt, __FUNCTION__, ## __VA_ARGS__)
#define _vx_dbg_f(debug, flag, fmt, ...) \
            do { \
               if (debug & flag) \
                   vx_log_f(fmt, ## __VA_ARGS__); \
            } while (0)

#define mpp_dbg_f(flag, fmt, ...) _vx_dbg_f(mpp_debug, flag, fmt, ## __VA_ARGS__)


#define MPP_DBG_INFO                    (0x00000001)
#define mpp_dbg_info(fmt, ...)          mpp_dbg(MPP_DBG_INFO, fmt, ## __VA_ARGS__)

#define  VX_ABORT                     (0x10000000)

/*
 * mpp_dbg usage:
 *
 * in h264d module define module debug flag variable like: h265d_debug
 * then define h265d_dbg macro as follow :
 *
 * extern RK_U32 h265d_debug;
 *
 * #define H265D_DBG_FUNCTION          (0x00000001)
 * #define H265D_DBG_VPS               (0x00000002)
 * #define H265D_DBG_SPS               (0x00000004)
 * #define H265D_DBG_PPS               (0x00000008)
 * #define H265D_DBG_SLICE_HDR         (0x00000010)
 *
 * #define h265d_dbg(flag, fmt, ...) mpp_dbg(h265d_debug, flag, fmt, ## __VA_ARGS__)
 *
 * finally use environment control the debug flag
 *
 * mpp_get_env_u32("h264d_debug", &h265d_debug, 0)
 *
 */
/*
 * sub-module debug flag usage example:
 * +------+-------------------+
 * | 8bit |      24bit        |
 * +------+-------------------+
 *  0~15 bit: software debug print
 * 16~23 bit: hardware debug print
 * 24~31 bit: information print format
 */

#define vx_abort() do {                \
    if (vx_debug & VX_ABORT) {        \
        abort();                        \
    }                                   \
} while (0)

#define VX_STRINGS(x)      VX_TO_STRING(x)
#define VX_TO_STRING(x)    #x

#define vx_assert(cond) do {                                           \
    if (!(cond)) {                                                      \
        vx_err("Assertion %s failed at %s:%d\n",                       \
               VX_STRINGS(cond), __FUNCTION__, __LINE__);              \
        vx_abort();                                                    \
    }                                                                   \
} while (0)


#endif