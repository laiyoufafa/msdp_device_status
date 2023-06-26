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

#include "adddraglistener_fuzzer.h"

#include "fi_log.h"
#include "interaction_manager.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "AddDragListenerFuzzTest" };
} // namespace

class DragListenerTest : public IDragListener {
public:
    DragListenerTest() : IDragListener() {}
    void OnDragMessage(DragState state) override
    {
        FI_HILOGD("DragListenerTest");
    };
};

void AddDragListenerFuzzTest(const uint8_t* data, size_t size)
{
    CALL_DEBUG_ENTER;
    std::shared_ptr<DragListenerTest> listener = std::make_shared<DragListenerTest>();
    InteractionManager::GetInstance()->AddDraglistener(listener);
    InteractionManager::GetInstance()->RemoveDraglistener(listener);
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t * data, size_t size)
{
    OHOS::Msdp::DeviceStatus::AddDragListenerFuzzTest(data, size);
    return 0;
}