/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FUSION_FRAMEWORKS_EXPORT_H
#define FUSION_FRAMEWORKS_EXPORT_H
#include <cinttypes>

#include "fusion_data_binding.h"

#ifdef __cplusplus
extern "C" {
#endif

int32_t fusion_alloc_socket_fd(const char *program_name, int32_t module_type,
    int32_t *client_fd, int32_t *token_type);

int32_t fusion_start_drag(CDragData *dragData);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif // FUSION_FRAMEWORKS_EXPORT_H