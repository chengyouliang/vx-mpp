/*
 * Copyright 2015 Rockchip Electronics Co. LTD
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

#ifndef __OS_ENV_H__
#define __OS_ENV_H__

#include "mpp_type.h"

#ifdef __cplusplus
extern "C" {
#endif

OMX_S32 os_get_env_u32(const char *name, OMX_U32 *value, OMX_U32 default_value);
OMX_S32 os_get_env_str(const char *name, const char **value, const char *default_value);

OMX_S32 os_set_env_u32(const char *name, OMX_U32 value);
OMX_S32 os_set_env_str(const char *name, char *value);

#ifdef __cplusplus
}
#endif

#endif /*__OS_ENV_H__*/

