/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#ifndef CLIENT_MSG_HANDLER_H
#define CLIENT_MSG_HANDLER_H

#include "nocopyable.h"

#include "msg_handler.h"
#include "uds_client.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
typedef std::function<int32_t(const UDSClient&, NetPacket&)> ClientMsgFun;
class ClientMsgHandler final : public MsgHandler<MmiMessageId, ClientMsgFun> {
public:
    ClientMsgHandler() = default;
    DISALLOW_COPY_AND_MOVE(ClientMsgHandler);
    ~ClientMsgHandler() override = default;

    void Init();
    void InitProcessedCallback();
    void OnMsgHandler(const UDSClient& client, NetPacket& pkt);

protected:
    int32_t OnCooperationListener(const UDSClient& client, NetPacket& pkt);
    int32_t OnCooperationMessage(const UDSClient& client, NetPacket& pkt);
    int32_t OnCooperationState(const UDSClient& client, NetPacket& pkt);

private:
    static void OnDispatchEventProcessed(int32_t eventId);

private:
    std::function<void(int32_t)> dispatchCallback_ { nullptr };
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // CLIENT_MSG_HANDLER_H
