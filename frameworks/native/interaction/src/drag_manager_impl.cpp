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

#include "drag_manager_impl.h"

#include "devicestatus_client.h"
#include "devicestatus_define.h"
#include "drag_data.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "DragManagerImpl" };
} // namespace

int32_t DragManagerImpl::UpdateDragStyle(int32_t style)
{
    CALL_DEBUG_ENTER;
    return DeviceStatusClient::GetInstance().UpdateDragStyle(style);
}

int32_t DragManagerImpl::UpdateDragMessage(const std::u16string &message)
{
    CALL_DEBUG_ENTER;
    return DeviceStatusClient::GetInstance().UpdateDragMessage(message);
}

int32_t DragManagerImpl::StartDrag(const DragData &dragData, std::function<void(int32_t&)> callback)
{
    CALL_DEBUG_ENTER;
    if (dragData.buffer.size() > MAX_BUFFER_SIZE) {
        FI_HILOGE("Invalid buffer, bufferSize:%{public}zu", dragData.buffer.size());
        return RET_ERR;
    }
    if (dragData.pixelMap == nullptr) {
        FI_HILOGE("dragData.pixelMap is nullptr");
        return RET_ERR;
    }
    if (dragData.pixelMap->GetWidth() > MAX_PIXEL_MAP_WIDTH ||
        dragData.pixelMap->GetHeight() > MAX_PIXEL_MAP_HEIGHT) {
        FI_HILOGE("Too big pixelMap, width:%{public}d, height:%{public}d",
            dragData.pixelMap->GetWidth(), dragData.pixelMap->GetHeight());
        return RET_ERR;
    }
    if (callback == nullptr) {
        FI_HILOGE("Callback is nullptr");
        return RET_ERR;
    }
    std::lock_guard<std::mutex> guard(mtx_);
    stopCallback_ = callback;
    return DeviceStatusClient::GetInstance().StartDrag(dragData);
}

int32_t DragManagerImpl::StopDrag(int32_t result)
{
    CALL_DEBUG_ENTER;
    return DeviceStatusClient::GetInstance().StopDrag(result);
}

int32_t DragManagerImpl::GetDragTargetPid()
{
    CALL_DEBUG_ENTER;
    return DeviceStatusClient::GetInstance().GetDragTargetPid();
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
