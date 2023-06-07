/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "devicestatus_define.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
using namespace testing::ext;
namespace {
std::shared_ptr<DeviceStatusAgent> g_agent1;
std::shared_ptr<DeviceStatusAgent> g_agent2;
} // namespace

Type DeviceStatusAgentTest::g_agentTest = Type::TYPE_INVALID;

void DeviceStatusAgentTest::SetUpTestCase() {}

void DeviceStatusAgentTest::TearDownTestCase() {}

void DeviceStatusAgentTest::SetUp()
{
    g_agent1 = std::make_shared<DeviceStatusAgent>();
    g_agent2 = std::make_shared<DeviceStatusAgent>();
}

void DeviceStatusAgentTest::TearDown() {}

bool DeviceStatusAgentListenerMockFirstClient::OnEventResult(
    const Data& devicestatusData)
{
    GTEST_LOG_(INFO) << "agent type: " << devicestatusData.type;
    GTEST_LOG_(INFO) << "agent value: " << devicestatusData.value;
    EXPECT_TRUE(devicestatusData.type == DeviceStatusAgentTest::g_agentTest &&
        (devicestatusData.value >= OnChangedValue::VALUE_INVALID &&
        devicestatusData.value <= OnChangedValue::VALUE_ENTER));
    return true;
}

bool DeviceStatusAgentListenerMockSecondClient::OnEventResult(
    const Data& devicestatusData)
{
    GTEST_LOG_(INFO) << "agent type: " << devicestatusData.type;
    GTEST_LOG_(INFO) << "agent value: " << devicestatusData.value;
    EXPECT_TRUE(devicestatusData.type == DeviceStatusAgentTest::g_agentTest &&
        (devicestatusData.value >= OnChangedValue::VALUE_INVALID &&
        devicestatusData.value <= OnChangedValue::VALUE_ENTER));
    return true;
}

/**
 * @tc.name: DeviceStatusAgentTest001
 * @tc.desc: test subscribing lid open event
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAgentTest, DeviceStatusAgentTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusAgentTest001 start";
    g_agentTest = Type::TYPE_LID_OPEN;
    Type type = g_agentTest;
    std::shared_ptr<DeviceStatusAgentListenerMockFirstClient> agentEvent =
        std::make_shared<DeviceStatusAgentListenerMockFirstClient>();
    int32_t ret = g_agent1->SubscribeAgentEvent(type, ActivityEvent::ENTER_EXIT,
        ReportLatencyNs::LONG, agentEvent);
    EXPECT_TRUE(ret == RET_OK);
    GTEST_LOG_(INFO) << "Open and close the lid, and event will report";
    sleep(2);
    g_agent1->UnsubscribeAgentEvent(type, ActivityEvent::ENTER_EXIT);
    GTEST_LOG_(INFO) << "DeviceStatusAgentTest001 end";
}

/**
 * @tc.name: DeviceStatusAgentTest002
 * @tc.desc: test subscribing lid open event repeatedly
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAgentTest, DeviceStatusAgentTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusAgentTest002 start";
    g_agentTest = Type::TYPE_LID_OPEN;
    Type type = g_agentTest;
    std::shared_ptr<DeviceStatusAgentListenerMockFirstClient> agentEvent =
        std::make_shared<DeviceStatusAgentListenerMockFirstClient>();
    int32_t ret = g_agent1->SubscribeAgentEvent(type, ActivityEvent::ENTER_EXIT,
        ReportLatencyNs::LONG, agentEvent);
    EXPECT_TRUE(ret == RET_OK);
    GTEST_LOG_(INFO) << "Open and close the lid, and event will report";
    sleep(2);
    ret = g_agent1->UnsubscribeAgentEvent(type, ActivityEvent::ENTER_EXIT);
    EXPECT_TRUE(ret == RET_OK);
    GTEST_LOG_(INFO) << "Open and close the lid, and event will not report";
    sleep(2);
    ret = g_agent1->SubscribeAgentEvent(type, ActivityEvent::ENTER_EXIT,
        ReportLatencyNs::LONG, agentEvent);
    EXPECT_TRUE(ret == RET_OK);
    GTEST_LOG_(INFO) << "Open and close the lid, and event will report again";
    sleep(2);
    g_agent1->UnsubscribeAgentEvent(type, ActivityEvent::ENTER_EXIT);
    GTEST_LOG_(INFO) << "DeviceStatusAgentTest002 end";
}

/**
 * @tc.name: DeviceStatusAgentTest003
 * @tc.desc: test subscribing lid open event for 2 client
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAgentTest, DeviceStatusAgentTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusAgentTest003 start";
    g_agentTest = Type::TYPE_LID_OPEN;
    Type type = g_agentTest;
    std::shared_ptr<DeviceStatusAgentListenerMockFirstClient> agentEvent1 =
        std::make_shared<DeviceStatusAgentListenerMockFirstClient>();
    std::shared_ptr<DeviceStatusAgentListenerMockSecondClient> agentEvent2 =
        std::make_shared<DeviceStatusAgentListenerMockSecondClient>();
    int32_t ret = g_agent1->SubscribeAgentEvent(type, ActivityEvent::ENTER_EXIT,
        ReportLatencyNs::LONG, agentEvent1);
    EXPECT_TRUE(ret == RET_OK);
    ret = g_agent2->SubscribeAgentEvent(type, ActivityEvent::ENTER_EXIT,
        ReportLatencyNs::LONG, agentEvent2);
    EXPECT_TRUE(ret == RET_OK);
    GTEST_LOG_(INFO) << "Open and close the lid, and event will report";
    sleep(2);
    GTEST_LOG_(INFO) << "Unsubscribe agentEvent1";
    g_agent1->UnsubscribeAgentEvent(type, ActivityEvent::ENTER_EXIT);
    sleep(2);
    GTEST_LOG_(INFO) << "Unsubscribe agentEvent2";
    g_agent2->UnsubscribeAgentEvent(type, ActivityEvent::ENTER_EXIT);
    GTEST_LOG_(INFO) << "DeviceStatusAgentTest003 end";
}

/**
 * @tc.name: DeviceStatusAgentTest004
 * @tc.desc: test subscribing lid open event
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAgentTest, DeviceStatusAgentTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusAgentTest004 start";
    g_agentTest = Type::TYPE_LID_OPEN;
    Type type = g_agentTest;
    std::shared_ptr<DeviceStatusAgentListenerMockFirstClient> agentEvent =
        std::make_shared<DeviceStatusAgentListenerMockFirstClient>();
    int32_t ret = g_agent1->SubscribeAgentEvent(type, ActivityEvent::ENTER_EXIT,
        ReportLatencyNs::LONG, agentEvent);
    EXPECT_TRUE(ret == RET_OK);
    GTEST_LOG_(INFO) << "Open and close the lid, and event will report";
    sleep(2);
    g_agent1->UnsubscribeAgentEvent(type, ActivityEvent::ENTER_EXIT);
    GTEST_LOG_(INFO) << "DeviceStatusAgentTest004 end";
}

/**
 * @tc.name: DeviceStatusAgentTest005
 * @tc.desc: test subscribing lid open event
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAgentTest, DeviceStatusAgentTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusAgentTest005 start";
    std::shared_ptr<DeviceStatusAgentListenerMockFirstClient> agentEvent =
        std::make_shared<DeviceStatusAgentListenerMockFirstClient>();
    int32_t ret = g_agent1->SubscribeAgentEvent(Type::TYPE_INVALID, ActivityEvent::ENTER_EXIT,
        ReportLatencyNs::LONG, agentEvent);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "Open and close the lid, and event will report";
    sleep(2);
    ret = g_agent1->UnsubscribeAgentEvent(Type::TYPE_INVALID, ActivityEvent::ENTER_EXIT);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "DeviceStatusAgentTest005 end";
}

/**
 * @tc.name: DeviceStatusAgentTest006
 * @tc.desc: test subscribing lid open event
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAgentTest, DeviceStatusAgentTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusAgentTest006 start";
    std::shared_ptr<DeviceStatusAgentListenerMockFirstClient> agentEvent =
        std::make_shared<DeviceStatusAgentListenerMockFirstClient>();
    int32_t ret = g_agent1->SubscribeAgentEvent(static_cast<Type>(10), ActivityEvent::ENTER_EXIT,
        ReportLatencyNs::LONG, agentEvent);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "Open and close the lid, and event will report";
    sleep(2);
    ret = g_agent1->UnsubscribeAgentEvent(Type::TYPE_INVALID, ActivityEvent::ENTER_EXIT);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "DeviceStatusAgentTest006 end";
}

/**
 * @tc.name: DeviceStatusAgentTest007
 * @tc.desc: test subscribing lid open event for 2 client
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAgentTest, DeviceStatusAgentTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusAgentTest007 start";
    g_agentTest = Type::TYPE_LID_OPEN;
    Type type = g_agentTest;
    std::shared_ptr<DeviceStatusAgentListenerMockFirstClient> agentEvent =
        std::make_shared<DeviceStatusAgentListenerMockFirstClient>();
    int32_t ret = g_agent1->SubscribeAgentEvent(Type::TYPE_INVALID, ActivityEvent::ENTER_EXIT,
        ReportLatencyNs::LONG, agentEvent);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    ret = g_agent1->SubscribeAgentEvent(type, ActivityEvent::ENTER_EXIT,
        ReportLatencyNs::LONG, agentEvent);
    EXPECT_TRUE(ret == RET_OK);
    GTEST_LOG_(INFO) << "Open and close the lid, and event will report";
    sleep(2);
    ret = g_agent1->UnsubscribeAgentEvent(Type::TYPE_INVALID, ActivityEvent::ENTER_EXIT);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    ret = g_agent1->UnsubscribeAgentEvent(type, ActivityEvent::ENTER_EXIT);
    EXPECT_TRUE(ret == RET_OK);
    GTEST_LOG_(INFO) << "DeviceStatusAgentTest007 end";
}

/**
 * @tc.name: DeviceStatusAgentTest008
 * @tc.desc: test subscribing lid open event
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAgentTest, DeviceStatusAgentTest008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusAgentTest008 start";
    std::shared_ptr<DeviceStatusAgentListenerMockFirstClient> agentEvent = nullptr;
    int32_t ret = g_agent1->SubscribeAgentEvent(Type::TYPE_LID_OPEN, ActivityEvent::ENTER_EXIT,
        ReportLatencyNs::LONG, agentEvent);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "Open and close the lid, and event will report";
    GTEST_LOG_(INFO) << "DeviceStatusAgentTest008 end";
}

/**
 * @tc.name: DeviceStatusAgentTest009
 * @tc.desc: test subscribing lid open event for 3 client
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAgentTest, DeviceStatusAgentTest009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusAgentTest009 start";
    g_agentTest = Type::TYPE_HORIZONTAL_POSITION;
    Type type = g_agentTest;
    std::shared_ptr<DeviceStatusAgentListenerMockFirstClient> agentEvent =
        std::make_shared<DeviceStatusAgentListenerMockFirstClient>();
    std::shared_ptr<DeviceStatusAgentListenerMockFirstClient> agentEvent1 = nullptr;
    int32_t ret = g_agent1->SubscribeAgentEvent(Type::TYPE_INVALID, ActivityEvent::ENTER_EXIT,
        ReportLatencyNs::LONG, agentEvent);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    ret = g_agent1->SubscribeAgentEvent(static_cast<Type>(10), ActivityEvent::ENTER_EXIT, ReportLatencyNs::LONG,
        agentEvent);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    ret = g_agent1->SubscribeAgentEvent(type, ActivityEvent::ENTER_EXIT,
        ReportLatencyNs::LONG, agentEvent1);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "Open and close the lid, and event will report";
    sleep(2);
    ret = g_agent1->UnsubscribeAgentEvent(Type::TYPE_INVALID, ActivityEvent::ENTER_EXIT);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    ret = g_agent1->UnsubscribeAgentEvent(type, ActivityEvent::ENTER_EXIT);
    EXPECT_TRUE(ret == RET_OK);
    ret = g_agent1->UnsubscribeAgentEvent(static_cast<Type>(10), ActivityEvent::ENTER_EXIT);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "DeviceStatusAgentTest009 end";
}

/**
 * @tc.name: DeviceStatusAgentTest0010
 * @tc.desc: test subscribing lid open event
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAgentTest, DeviceStatusAgentTest010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusAgentTest010 start";
    g_agentTest = Type::TYPE_LID_OPEN;
    Type type = g_agentTest;
    std::shared_ptr<DeviceStatusAgentListenerMockFirstClient> agentEvent =
        std::make_shared<DeviceStatusAgentListenerMockFirstClient>();
    int32_t ret = g_agent1->SubscribeAgentEvent(type, ActivityEvent::ENTER_EXIT,
        ReportLatencyNs::LONG, agentEvent);
    EXPECT_TRUE(ret == RET_OK);

    sptr<IRemoteDevStaCallback> callback = new (std::nothrow) DeviceStatusAgent::DeviceStatusAgentCallback(g_agent1);
    ASSERT_TRUE(callback != nullptr);
    Data devicestatusData = {
        Type::TYPE_LID_OPEN,
        OnChangedValue::VALUE_ENTER
    };
    callback->OnDeviceStatusChanged(devicestatusData);
    GTEST_LOG_(INFO) << "DeviceStatusAgentTest010 end";
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
