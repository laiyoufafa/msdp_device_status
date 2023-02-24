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

#include <iostream>

#include <gtest/gtest.h>
#include "pointer_event.h"

#include "coordination_message.h"
#include "devicestatus_define.h"
#include "devicestatus_errors.h"
#include "interaction_manager.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
using namespace testing::ext;
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "InteractionManagerTest" };
constexpr int32_t TIME_WAIT_FOR_OP = 100;
} // namespace
class InteractionManagerTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
};

void InteractionManagerTest::SetUpTestCase()
{
}

void InteractionManagerTest::SetUp()
{
}

void InteractionManagerTest::TearDown()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_WAIT_FOR_OP));
}


int32_t CreatePixelMap(int32_t pixelMapWidth, int32_t pixelMapHeight, std::shared_ptr<OHOS::Media::PixelMap> pixelMap)
{
    if (pixelMapWidth <= 0 || pixelMapWidth > MAX_PIXEL_MAP_WIDTH ||
        pixelMapHeight <= 0 || pixelMapHeight > MAX_PIXEL_MAP_HEIGHT) {
        FI_HILOGE("Invalid size, pixelMapWidth:%{public}d, pixelMapHeight:%{public}d", pixelMapWidth, pixelMapHeight);
        return RET_ERR;
    }
    OHOS::Media::ImageInfo info;
    info.size.width = pixelMapWidth;
    info.size.height = pixelMapHeight;
    info.pixelFormat = Media::PixelFormat::RGB_888;
    info.colorSpace = OHOS::Media::ColorSpace::SRGB;
    pixelMap->SetImageInfo(info);
    int32_t bufferSize = pixelMapWidth * pixelMapHeight;
    char *buffer = static_cast<char *>(malloc(bufferSize));
    if (buffer == nullptr) {
        FI_HILOGE("Malloc buffer failed");
        return RET_ERR;
    }
    char *character = buffer;
    for (int32_t i = 0; i < bufferSize; i++) {
        *(character++) = static_cast<char>(i);
    }
    pixelMap->SetPixelsAddr(buffer, nullptr, bufferSize, OHOS::Media::AllocatorType::HEAP_ALLOC, nullptr);
    return RET_OK;
}

int32_t SetParam(int32_t width, int32_t height, DragData& dragData)
{
    auto pixelMap = std::make_shared<OHOS::Media::PixelMap>();
    if (CreatePixelMap(width, height, pixelMap) != RET_OK) {
        FI_HILOGE("CreatePixelMap failed");
        return RET_ERR;
    }
    dragData.pictureResourse.pixelMap = pixelMap;
    dragData.pictureResourse.x = 0;
    dragData.pictureResourse.y = 0;
    dragData.buffer = std::vector<uint8_t>(MAX_BUFFER_SIZE, 0);
    dragData.sourceType = OHOS::MMI::PointerEvent::SOURCE_TYPE_MOUSE;
    dragData.pointerId = 0;
    dragData.dragNum = 1;
    return RET_OK;
}

/**
 * @tc.name: InteractionManagerTest_RegisterCoordinationListener_001
 * @tc.desc: Register coordination listener
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InteractionManagerTest, InteractionManagerTest_RegisterCoordinationListener_001, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    std::shared_ptr<ICoordinationListener> consumer = nullptr;
    int32_t ret = InteractionManager::GetInstance()->RegisterCoordinationListener(consumer);
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    ASSERT_EQ(ret, RET_ERR);
#else
    ASSERT_EQ(ret, ERROR_UNSUPPORT);
#endif // OHOS_BUILD_ENABLE_COORDINATION
}

/**
 * @tc.name: InteractionManagerTest_RegisterCoordinationListener_002
 * @tc.desc: Register coordination listener
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InteractionManagerTest, InteractionManagerTest_RegisterCoordinationListener_002, TestSize.Level1)
{
    CALL_DEBUG_ENTER;
    class CoordinationListenerTest : public ICoordinationListener {
    public:
        CoordinationListenerTest() : ICoordinationListener() {}
        void OnCoordinationMessage(const std::string &deviceId, CoordinationMessage msg) override
        {
            FI_HILOGD("RegisterCoordinationListenerTest");
        };
    };
    std::shared_ptr<CoordinationListenerTest> consumer =
        std::make_shared<CoordinationListenerTest>();
    int32_t ret = InteractionManager::GetInstance()->RegisterCoordinationListener(consumer);
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    ASSERT_EQ(ret, RET_OK);
#else
    ASSERT_EQ(ret, ERROR_UNSUPPORT);
#endif // OHOS_BUILD_ENABLE_COORDINATION
    ret = InteractionManager::GetInstance()->UnregisterCoordinationListener(consumer);
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    ASSERT_EQ(ret, RET_OK);
#else
    ASSERT_EQ(ret, ERROR_UNSUPPORT);
#endif // OHOS_BUILD_ENABLE_COORDINATION
}

/**
 * @tc.name: InteractionManagerTest_UnregisterCoordinationListener
 * @tc.desc: Unregister coordination listener
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InteractionManagerTest, InteractionManagerTest_UnregisterCoordinationListener, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    std::shared_ptr<ICoordinationListener> consumer = nullptr;
    int32_t ret = InteractionManager::GetInstance()->UnregisterCoordinationListener(consumer);
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    ASSERT_EQ(ret, RET_OK);
#else
    ASSERT_EQ(ret, ERROR_UNSUPPORT);
#endif // OHOS_BUILD_ENABLE_COORDINATION
}

/**
 * @tc.name: InteractionManagerTest_EnableCoordination
 * @tc.desc: Enable coordination
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InteractionManagerTest, InteractionManagerTest_EnableCoordination, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    bool enabled = false;
    auto fun = [](std::string listener, CoordinationMessage coordinationMessages) {
        FI_HILOGD("Enable coordination success");
    };
    int32_t ret = InteractionManager::GetInstance()->EnableCoordination(enabled, fun);
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    ASSERT_EQ(ret, RET_OK);
#else
    ASSERT_EQ(ret, ERROR_UNSUPPORT);
#endif // OHOS_BUILD_ENABLE_COORDINATION
}

/**
 * @tc.name: InteractionManagerTest_StartCoordination
 * @tc.desc: Start coordination
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InteractionManagerTest, InteractionManagerTest_StartCoordination, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    std::string sinkDeviceId("");
    int32_t srcDeviceId = -1;
    auto fun = [](std::string listener, CoordinationMessage coordinationMessages) {
        FI_HILOGD("Start coordination success");
    };
    int32_t ret = InteractionManager::GetInstance()->StartCoordination(sinkDeviceId, srcDeviceId, fun);
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    ASSERT_NE(ret, RET_OK);
#else
    ASSERT_EQ(ret, ERROR_UNSUPPORT);
#endif // OHOS_BUILD_ENABLE_COORDINATION
}

/**
 * @tc.name: InteractionManagerTest_StopCoordination
 * @tc.desc: Stop coordination
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InteractionManagerTest, InteractionManagerTest_StopCoordination, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    auto fun = [](std::string listener, CoordinationMessage coordinationMessages) {
        FI_HILOGD("Stop coordination success");
    };
    int32_t ret = InteractionManager::GetInstance()->StopCoordination(fun);
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    ASSERT_NE(ret, ERROR_UNSUPPORT);
#else
    ASSERT_EQ(ret, ERROR_UNSUPPORT);
#endif // OHOS_BUILD_ENABLE_COORDINATION
}

/**
 * @tc.name: InteractionManagerTest_GetCoordinationState
 * @tc.desc: Get coordination state
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InteractionManagerTest, InteractionManagerTest_GetCoordinationState, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    const std::string deviceId("");
    auto fun = [](bool state) {
        FI_HILOGD("Get coordination state success");
    };
    int32_t ret = InteractionManager::GetInstance()->GetCoordinationState(deviceId, fun);
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    ASSERT_EQ(ret, RET_OK);
#else
    ASSERT_EQ(ret, ERROR_UNSUPPORT);
#endif // OHOS_BUILD_ENABLE_COORDINATION
}

/**
 * @tc.name: InteractionManagerTest_StartDrag
 * @tc.desc: Start Drag
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InteractionManagerTest, InteractionManagerTest_StartDrag, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    DragData dragData;
    int32_t ret = SetParam(200, 200, dragData);
    ASSERT_EQ(ret, RET_OK);
    std::function<void(int32_t)> callback = [](int32_t result) {
        FI_HILOGD("StartDrag success");
    };
    ret = InteractionManager::GetInstance()->StartDrag(dragData, callback);
    ASSERT_EQ(ret, RET_OK);
}

/**
 * @tc.name: InteractionManagerTest_StopDrag
 * @tc.desc: Stop drag
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InteractionManagerTest, InteractionManagerTest_StopDrag, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    int32_t result = 0;
    int32_t ret = InteractionManager::GetInstance()->StopDrag(result);
    ASSERT_EQ(ret, RET_OK);
}

 /**
*  @tc.name: InteractionManagerTest_GetDragTargetPid
 * @tc.desc: Get Drag Target Pid
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InteractionManagerTest, InteractionManagerTest_GetDragTargetPid, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    int32_t pid = InteractionManager::GetInstance()->GetDragTargetPid();
    ASSERT_TRUE(pid >= -1);
}

} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS