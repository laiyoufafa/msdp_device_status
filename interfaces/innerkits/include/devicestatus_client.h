/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef DEVICESTATUS_CLIENT_H
#define DEVICESTATUS_CLIENT_H

#include <functional>
#include <map>
#include <singleton.h>

#include "devicestatus_common.h"
#include "devicestatus_data_utils.h"
#include "drag_data.h"
#include "i_coordination_listener.h"
#include "idevicestatus.h"
#include "idevicestatus_callback.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class DeviceStatusClient final : public DelayedRefSingleton<DeviceStatusClient> {
    DECLARE_DELAYED_REF_SINGLETON(DeviceStatusClient)

public:
    std::map<Type, int32_t> GetTypeMap()
    {
        return typeMap_;
    }
    DISALLOW_COPY_AND_MOVE(DeviceStatusClient);

    void SubscribeCallback(Type type, ActivityEvent event, ReportLatencyNs latency,
        sptr<IRemoteDevStaCallback> callback);
    void UnsubscribeCallback(Type type, ActivityEvent event, sptr<IRemoteDevStaCallback> callback);
    Data GetDeviceStatusData(const Type type);
    void RegisterDeathListener(std::function<void()> deathListener);

    int32_t RegisterCoordinationListener();
    int32_t UnregisterCoordinationListener();
    int32_t EnableCoordination(int32_t userData, bool enabled);
    int32_t StartCoordination(int32_t userData, const std::string &sinkDeviceId, int32_t srcDeviceId);
    int32_t StopCoordination(int32_t userData);
    int32_t GetCoordinationState(int32_t userData, const std::string &deviceId);

    int32_t StartDrag(const DragData &dragData);
    int32_t StopDrag(int32_t dragResult);
    int32_t UpdateDragStyle(int32_t style);
    int32_t UpdateDragMessage(const std::u16string &message);
    int32_t GetDragTargetPid();

    int32_t AllocSocketPair(const int32_t moduleType);
    int32_t GetClientSocketFdOfAllocedSocketPair() const;
private:
    class DeviceStatusDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        DeviceStatusDeathRecipient() = default;
        ~DeviceStatusDeathRecipient() = default;
        void OnRemoteDied(const wptr<IRemoteObject>& remote);
    private:
        DISALLOW_COPY_AND_MOVE(DeviceStatusDeathRecipient);
    };

    ErrCode Connect();
    sptr<Idevicestatus> devicestatusProxy_ {nullptr};
    sptr<IRemoteObject::DeathRecipient> deathRecipient_ {nullptr};
    void ResetProxy(const wptr<IRemoteObject>& remote);
    std::mutex mutex_;
        int32_t tokenType_ { -1 };
    int32_t socketFd_ { -1 };
    std::map<Type, int32_t> typeMap_ = {};
    std::function<void()> deathListener_ { nullptr };
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // DEVICESTATUS_CLIENT_H
