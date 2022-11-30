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
#include "fd_listener.h"

#include <cinttypes>

#include "fi_log.h"
#include "stream_buffer.h"
#include "uds_socket.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "FdListener" };
}

using namespace AppExecFwk;
FdListener::FdListener(IClientPtr client) : iClient_(client) {}

void FdListener::OnReadable(int32_t fd)
{
    if (fd < 0) {
        FI_HILOGE("Invalid fd:%{public}d", fd);
        return;
    }
    CHKPV(iClient_);
    char szBuf[MAX_PACKET_BUF_SIZE] = {};
    for (int32_t i = 0; i < MAX_RECV_LIMIT; i++) {
        ssize_t size = recv(fd, szBuf, MAX_PACKET_BUF_SIZE, MSG_DONTWAIT | MSG_NOSIGNAL);
        if (size > 0) {
            iClient_->OnRecvMsg(szBuf, size);
        } else if (size < 0) {
            if (errno == EAGAIN || errno == EINTR || errno == EWOULDBLOCK) {
                FI_HILOGW("Continue for errno EAGAIN|EINTR|EWOULDBLOCK size:%{public}zu errno:%{public}d",
                    size, errno);
                continue;
            }
            FI_HILOGE("Recv return %{public}zu errno:%{public}d", size, errno);
            break;
        } else {
            FI_HILOGD("[Do nothing here]The service side disconnect with the client. size:0 count:%{public}d "
                "errno:%{public}d", i, errno);
            break;
        }
        if (size < MAX_PACKET_BUF_SIZE) {
            break;
        }
    }
}

void FdListener::OnShutdown(int32_t fd)
{
    CHK_PID_AND_TID();
    if (fd < 0) {
        FI_HILOGE("Invalid fd:%{public}d", fd);
    }
    CHKPV(iClient_);
    iClient_->OnDisconnect();
}

void FdListener::OnException(int32_t fd)
{
    CHK_PID_AND_TID();
    if (fd < 0) {
        FI_HILOGE("Invalid fd:%{public}d", fd);
    }
    CHKPV(iClient_);
    iClient_->OnDisconnect();
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
