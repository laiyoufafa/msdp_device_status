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

#ifndef OHOS_MSDP_DEVICESTATUS_AGENT_TEST_H
#define OHOS_MSDP_DEVICESTATUS_AGENT_TEST_H

#include <gtest/gtest.h>

#include "devicestatus_agent.h"
#include "devicestatus_data_utils.h"

namespace OHOS {
namespace Msdp {
class DevicestatusAgentTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};
class DevicestatusAgentListenerMockFirstClient : public DeviceStatusAgent::DeviceStatusAgentEvent {
public:
    virtual ~DevicestatusAgentListenerMockFirstClient() {};
    bool OnEventResult(const DevicestatusDataUtils::DevicestatusData& devicestatusData) override;
};
class DevicestatusAgentListenerMockSecondClient : public DeviceStatusAgent::DeviceStatusAgentEvent {
public:
    virtual ~DevicestatusAgentListenerMockSecondClient() {};
    bool OnEventResult(const DevicestatusDataUtils::DevicestatusData& devicestatusData) override;
};
} // namespace Msdp
} // namespace OHOS
#endif // OHOS_MSDP_DEVICESTATUS_AGENT_TEST_H