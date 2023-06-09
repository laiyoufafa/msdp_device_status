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

#include "devicestatus_agent_test.h"

#include "devicestatus_common.h"

using namespace testing::ext;
using namespace OHOS::Msdp;
using namespace OHOS;
using namespace std;

static std::shared_ptr<DeviceStatusAgent> agent1_;
static std::shared_ptr<DeviceStatusAgent> agent2_;

void DevicestatusAgentTest::SetUpTestCase()
{
}

void DevicestatusAgentTest::TearDownTestCase()
{
}

void DevicestatusAgentTest::SetUp()
{
    agent1_ = std::make_shared<DeviceStatusAgent>();
    agent2_ = std::make_shared<DeviceStatusAgent>();
}

void DevicestatusAgentTest::TearDown()
{
}

bool DevicestatusAgentListenerMockFirstClient::OnEventResult(
    const DevicestatusDataUtils::DevicestatusData& devicestatusData)
{
    GTEST_LOG_(INFO) << "agent type: " << devicestatusData.type;
    GTEST_LOG_(INFO) << "agent value: " << devicestatusData.value;
    EXPECT_EQ(true, devicestatusData.type == DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN);
    return true;
}

bool DevicestatusAgentListenerMockSecondClient::OnEventResult(
    const DevicestatusDataUtils::DevicestatusData& devicestatusData)
{
    GTEST_LOG_(INFO) << "agent type: " << devicestatusData.type;
    GTEST_LOG_(INFO) << "agent value: " << devicestatusData.value;
    EXPECT_EQ(true, devicestatusData.type == DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN);
    return true;
}

namespace {
/**
 * @tc.name: DevicestatusAgentTest001
 * @tc.desc: test subscribing lid open event
 * @tc.type: FUNC
 */
HWTEST_F (DevicestatusAgentTest, DevicestatusAgentTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DevicestatusAgentTest001 start";
    std::shared_ptr<DevicestatusAgentListenerMockFirstClient> agentEvent =
        std::make_shared<DevicestatusAgentListenerMockFirstClient>();
    int32_t ret = agent1_->SubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN, agentEvent);
    EXPECT_EQ(true, ret == ERR_OK);
    GTEST_LOG_(INFO) << "Open and close the lid, and event will report";
    sleep(10);
    agent1_->UnSubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN);
    GTEST_LOG_(INFO) << "DevicestatusAgentTest001 end";
}

/**
 * @tc.name: DevicestatusAgentTest002
 * @tc.desc: test subscribing lid open event repeatedly
 * @tc.type: FUNC
 */
HWTEST_F (DevicestatusAgentTest, DevicestatusAgentTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DevicestatusAgentTest002 start";
    std::shared_ptr<DevicestatusAgentListenerMockFirstClient> agentEvent =
        std::make_shared<DevicestatusAgentListenerMockFirstClient>();
    int32_t ret = agent1_->SubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN, agentEvent);
    EXPECT_EQ(true, ret == ERR_OK);
    GTEST_LOG_(INFO) << "Open and close the lid, and event will report";
    sleep(5);
    ret = agent1_->UnSubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN);
    EXPECT_EQ(true, ret == ERR_OK);
    GTEST_LOG_(INFO) << "Open and close the lid, and event will not report";
    sleep(5);
    ret = agent1_->SubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN, agentEvent);
    EXPECT_EQ(true, ret == ERR_OK);
    GTEST_LOG_(INFO) << "Open and close the lid, and event will report again";
    sleep(5);
    agent1_->UnSubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN);
    GTEST_LOG_(INFO) << "DevicestatusAgentTest002 end";
}

/**
 * @tc.name: DevicestatusAgentTest003
 * @tc.desc: test subscribing lid open event for 2 client
 * @tc.type: FUNC
 */
HWTEST_F (DevicestatusAgentTest, DevicestatusAgentTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DevicestatusAgentTest003 start";
    std::shared_ptr<DevicestatusAgentListenerMockFirstClient> agentEvent1 =
        std::make_shared<DevicestatusAgentListenerMockFirstClient>();
    std::shared_ptr<DevicestatusAgentListenerMockSecondClient> agentEvent2 =
        std::make_shared<DevicestatusAgentListenerMockSecondClient>();
    int32_t ret = agent1_->SubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN, agentEvent1);
    EXPECT_EQ(true, ret == ERR_OK);
    ret = agent2_->SubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN, agentEvent2);
    EXPECT_EQ(true, ret == ERR_OK);
    GTEST_LOG_(INFO) << "Open and close the lid, and event will report";
    sleep(5);
    GTEST_LOG_(INFO) << "UnSubscribe agentEvent1";
    agent1_->UnSubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN);
    sleep(5);
    GTEST_LOG_(INFO) << "UnSubscribe agentEvent2";
    agent2_->UnSubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN);
    GTEST_LOG_(INFO) << "DevicestatusAgentTest003 end";
}
}