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

#include <optional>
#include <utility>
#include <vector>

#include <unistd.h>

#include <gtest/gtest.h>
#include "input_manager.h"
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
constexpr int32_t TIME_WAIT_FOR_OP { 100 };
constexpr int32_t TIME_WAIT_FOR_INJECT_MS { 20000 };
constexpr int32_t TIME_WAIT_FOR_PROCESS_CALLBACK { 20000 };
constexpr int32_t TIME_WAIT_FOR_TOUCH_DOWN { 1 };
constexpr int32_t DEFAULT_DEVICE_ID { 0 };
constexpr int32_t MOUSE_POINTER_ID { 0 };
constexpr int32_t TOUCH_POINTER_ID { 1 };
constexpr int32_t DISPLAY_ID { 0 };
constexpr int32_t DRAG_SRC_X { 0 };
constexpr int32_t DRAG_SRC_Y { 0 };
constexpr int32_t DRAG_DST_X { 20 };
constexpr int32_t DRAG_DST_Y { 20 };
constexpr int32_t DRAG_NUM { 1 };
constexpr int32_t MOVE_STEP { 10 };
constexpr bool HAS_CANCELED_ANIMATION { false };
constexpr bool HAS_CUSTOM_ANIMATION { false };
#define INPUT_MANAGER  MMI::InputManager::GetInstance()
} // namespace

class InteractionManagerTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    std::shared_ptr<Media::PixelMap> CreatePixelMap(int32_t width, int32_t height);
    std::optional<DragData> CreateDragData(std::pair<int32_t, int32_t> pixelMapSize, int32_t sourceType,
    int32_t pointerId, int32_t displayId, std::pair<int32_t, int32_t> location);
    MMI::PointerEvent::PointerItem CreatePointerItem(int32_t pointerId,
    int32_t deviceId, std::pair<int, int> displayLocation, bool isPressed);
    std::shared_ptr<MMI::PointerEvent> SetupPointerEvent(std::pair<int, int> displayLocation, int32_t action,
    int32_t sourceType, int32_t pointerId, bool isPressed);
    void SimulateDownEvent(std::pair<int, int> location, int32_t sourceType, int32_t pointerId);
    void SimulateMoveEvent(std::pair<int, int> srcLocation, std::pair<int, int> dstLocation,
    int32_t sourceType, int32_t pointerId, bool isPressed);
    void SimulateUpEvent(std::pair<int, int> location, int32_t sourceType, int32_t pointerId);
    int32_t TestAddMonitor(std::shared_ptr<MMI::IInputEventConsumer> consumer);
    void TestRemoveMonitor(int32_t monitorId);
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

std::shared_ptr<Media::PixelMap> InteractionManagerTest::CreatePixelMap(int32_t width, int32_t height)
{
    CALL_DEBUG_ENTER;
    if (width <= 0 || width > MAX_PIXEL_MAP_WIDTH ||
       height <= 0 || height > MAX_PIXEL_MAP_HEIGHT) {
        FI_HILOGE("Invalid size,width:%{public}d,height:%{public}d", width, height);
        return nullptr;
    }
    OHOS::Media::InitializationOptions opts;
    opts.size.width = width;
    opts.size.height = height;
    std::unique_ptr<Media::PixelMap> uniquePixelMap = Media::PixelMap::Create(opts);
    std::shared_ptr<Media::PixelMap> pixelMap = std::move(uniquePixelMap);
    return pixelMap;
}

std::optional<DragData> InteractionManagerTest::CreateDragData(std::pair<int32_t, int32_t> pixelMapSize,
    int32_t sourceType, int32_t pointerId, int32_t displayId, std::pair<int32_t, int32_t> location)
{
    CALL_DEBUG_ENTER;
    std::shared_ptr<Media::PixelMap> pixelMap = CreatePixelMap(pixelMapSize.first, pixelMapSize.second);
    if (pixelMap == nullptr) {
        FI_HILOGE("CreatePixelMap failed");
        return std::nullopt;
    }
    DragData dragData;
    dragData.shadowInfo.pixelMap = pixelMap;
    dragData.shadowInfo.x = 0;
    dragData.shadowInfo.y = 0;
    dragData.buffer = std::vector<uint8_t>(MAX_BUFFER_SIZE, 0);
    dragData.sourceType = sourceType;
    dragData.pointerId = pointerId;
    dragData.dragNum = DRAG_NUM;
    dragData.displayX = location.first;
    dragData.displayY = location.second;
    dragData.displayId = displayId;
    dragData.hasCanceledAnimation = HAS_CANCELED_ANIMATION;
    return dragData;
}

MMI::PointerEvent::PointerItem InteractionManagerTest::CreatePointerItem(int32_t pointerId,
    int32_t deviceId, std::pair<int, int> displayLocation, bool isPressed)
{
    MMI::PointerEvent::PointerItem item;
    item.SetPointerId(pointerId);
    item.SetDeviceId(deviceId);
    item.SetDisplayX(displayLocation.first);
    item.SetDisplayY(displayLocation.second);
    item.SetPressed(isPressed);
    return item;
}

std::shared_ptr<MMI::PointerEvent> InteractionManagerTest::SetupPointerEvent(std::pair<int, int> displayLocation,
    int32_t action, int32_t sourceType, int32_t pointerId, bool isPressed)
{
    CALL_DEBUG_ENTER;
    auto pointerEvent = MMI::PointerEvent::Create();
    CHKPP(pointerEvent);
    pointerEvent->SetPointerAction(action);
    pointerEvent->SetSourceType(sourceType);
    pointerEvent->SetPointerId(pointerId);
    auto curPointerItem = CreatePointerItem(pointerId, DEFAULT_DEVICE_ID, displayLocation, isPressed);
    pointerEvent->AddPointerItem(curPointerItem);
    return pointerEvent;
}

void InteractionManagerTest::SimulateDownEvent(std::pair<int, int> location, int32_t sourceType, int32_t pointerId)
{
    CALL_DEBUG_ENTER;
    std::shared_ptr<MMI::PointerEvent> pointerEvent =
        SetupPointerEvent(location, MMI::PointerEvent::POINTER_ACTION_DOWN, sourceType, pointerId, true);
    FI_HILOGD("TEST:sourceType:%{public}d, pointerId:%{public}d, pointerAction:%{public}d",
        pointerEvent->GetSourceType(), pointerEvent->GetPointerId(), pointerEvent->GetPointerAction());
    INPUT_MANAGER->SimulateInputEvent(pointerEvent);
}

void InteractionManagerTest::SimulateMoveEvent(std::pair<int, int> srcLocation, std::pair<int, int> dstLocation,
    int32_t sourceType, int32_t pointerId, bool isPressed)
{
    CALL_DEBUG_ENTER;
    int32_t srcX = srcLocation.first;
    int32_t srcY = srcLocation.second;
    int32_t dstX = dstLocation.first;
    int32_t dstY = dstLocation.second;
    std::vector<std::pair<int32_t, int32_t>> coordinates;
    if (dstX == srcX) {
        for (int32_t y = srcY; y <= dstY; y += MOVE_STEP) {
            coordinates.push_back({srcX, y});
        }
    } else {
        double ratio = (dstY - srcY) * 1.0 / (dstX - srcX);
        for (int32_t x = srcX; x < dstX; x += MOVE_STEP) {
            coordinates.push_back({x, srcY + static_cast<int32_t>(ratio * (x - srcX))});
        }
        coordinates.push_back({dstX, dstY});
    }
    for (const auto& pointer : coordinates) {
        std::shared_ptr<MMI::PointerEvent> pointerEvent =
            SetupPointerEvent(pointer, MMI::PointerEvent::POINTER_ACTION_MOVE, sourceType, pointerId, isPressed);
        FI_HILOGD("TEST:sourceType:%{public}d, pointerId:%{public}d, pointerAction:%{public}d",
            pointerEvent->GetSourceType(), pointerEvent->GetPointerId(), pointerEvent->GetPointerAction());
        INPUT_MANAGER->SimulateInputEvent(pointerEvent);
        usleep(TIME_WAIT_FOR_INJECT_MS);
    }
}

void InteractionManagerTest::SimulateUpEvent(std::pair<int, int> location, int32_t sourceType, int32_t pointerId)
{
    CALL_DEBUG_ENTER;
    std::shared_ptr<MMI::PointerEvent> pointerEvent =
        SetupPointerEvent(location, MMI::PointerEvent::POINTER_ACTION_UP, sourceType, pointerId, false);
    FI_HILOGD("TEST:sourceType:%{public}d, pointerId:%{public}d, pointerAction:%{public}d",
        pointerEvent->GetSourceType(), pointerEvent->GetPointerId(), pointerEvent->GetPointerAction());
    INPUT_MANAGER->SimulateInputEvent(pointerEvent);
}

int32_t InteractionManagerTest::TestAddMonitor(std::shared_ptr<MMI::IInputEventConsumer> consumer)
{
    return MMI::InputManager::GetInstance()->AddMonitor(consumer);
}

void InteractionManagerTest::TestRemoveMonitor(int32_t monitorId)
{
    MMI::InputManager::GetInstance()->RemoveMonitor(monitorId);
}

class InputEventCallbackTest : public MMI::IInputEventConsumer {
public:
    virtual void OnInputEvent(std::shared_ptr<MMI::KeyEvent> keyEvent) const override {};
    virtual void OnInputEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent) const override;
    virtual void OnInputEvent(std::shared_ptr<MMI::AxisEvent> axisEvent) const override {};
};

void InputEventCallbackTest::OnInputEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent) const
{
    CALL_DEBUG_ENTER;
    ASSERT_TRUE(pointerEvent != nullptr);
    ASSERT_TRUE(!pointerEvent->GetBuffer().empty());
    ASSERT_TRUE(pointerEvent->GetPointerAction() == MMI::PointerEvent::POINTER_ACTION_PULL_MOVE);
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
 * @tc.name: InteractionManagerTest_StartDrag_Mouse
 * @tc.desc: Start Drag
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InteractionManagerTest, InteractionManagerTest_StartDrag_Mouse, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    auto callback = [](const DragNotifyMsg& notifyMessage) {
        FI_HILOGD("displayX:%{public}d, displayY:%{public}d, result:%{public}d, target:%{public}d",
            notifyMessage.displayX, notifyMessage.displayY, notifyMessage.result, notifyMessage.targetPid);
    };
    SimulateDownEvent({ DRAG_SRC_X, DRAG_SRC_Y }, MMI::PointerEvent::SOURCE_TYPE_MOUSE, MOUSE_POINTER_ID);
    std::optional<DragData> dragData = CreateDragData({ MAX_PIXEL_MAP_WIDTH, MAX_PIXEL_MAP_HEIGHT },
        MMI::PointerEvent::SOURCE_TYPE_MOUSE, MOUSE_POINTER_ID, DISPLAY_ID, { DRAG_SRC_X, DRAG_SRC_Y });
    ASSERT_TRUE(dragData);
    int32_t ret = InteractionManager::GetInstance()->StartDrag(dragData.value(), callback);
    ASSERT_EQ(ret, RET_OK);
    SimulateMoveEvent({ DRAG_SRC_X, DRAG_SRC_Y }, { DRAG_DST_X, DRAG_DST_Y },
        MMI::PointerEvent::SOURCE_TYPE_MOUSE, MOUSE_POINTER_ID, true);
    SimulateUpEvent({ DRAG_DST_X, DRAG_DST_Y }, MMI::PointerEvent::SOURCE_TYPE_MOUSE, MOUSE_POINTER_ID);
    InteractionManager::GetInstance()->StopDrag(DragResult::DRAG_SUCCESS, HAS_CUSTOM_ANIMATION);
}

/**
 * @tc.name: InteractionManagerTest_StopDrag_Mouse
 * @tc.desc: Stop drag
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InteractionManagerTest, InteractionManagerTest_StopDrag_Mouse, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    bool stopCallbackFlag = false;
    auto callback = [&stopCallbackFlag](const DragNotifyMsg& notifyMessage) {
        FI_HILOGD("displayX:%{public}d, displayY:%{public}d, result:%{public}d, target:%{public}d",
            notifyMessage.displayX, notifyMessage.displayY, notifyMessage.result, notifyMessage.targetPid);
        stopCallbackFlag = true;
        FI_HILOGD("set stopCallbackFlag to true");
    };
    SimulateDownEvent({ DRAG_SRC_X, DRAG_SRC_Y }, MMI::PointerEvent::SOURCE_TYPE_MOUSE, MOUSE_POINTER_ID);
    std::optional<DragData> dragData = CreateDragData({ MAX_PIXEL_MAP_WIDTH, MAX_PIXEL_MAP_HEIGHT },
        MMI::PointerEvent::SOURCE_TYPE_MOUSE, MOUSE_POINTER_ID, DISPLAY_ID, { DRAG_SRC_X, DRAG_SRC_Y });
    ASSERT_TRUE(dragData);
    InteractionManager::GetInstance()->StartDrag(dragData.value(), callback);
    SimulateUpEvent({ DRAG_DST_X, DRAG_DST_Y }, MMI::PointerEvent::SOURCE_TYPE_MOUSE, MOUSE_POINTER_ID);
    int32_t ret = InteractionManager::GetInstance()->StopDrag(DragResult::DRAG_SUCCESS, HAS_CUSTOM_ANIMATION);
    ASSERT_EQ(ret, RET_OK);
    usleep(TIME_WAIT_FOR_PROCESS_CALLBACK);
    ASSERT_TRUE(stopCallbackFlag);
}

/**
 * @tc.name: InteractionManagerTest_StartDrag_Touch
 * @tc.desc: Start Drag
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InteractionManagerTest, InteractionManagerTest_StartDrag_Touch, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    auto callback = [](const DragNotifyMsg& notifyMessage) {
        FI_HILOGD("displayX:%{public}d, displayY:%{public}d, result:%{public}d, target:%{public}d",
            notifyMessage.displayX, notifyMessage.displayY, notifyMessage.result, notifyMessage.targetPid);
    };
    SimulateDownEvent({ DRAG_SRC_X, DRAG_SRC_Y }, MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN, TOUCH_POINTER_ID);
    sleep(TIME_WAIT_FOR_TOUCH_DOWN);
    std::optional<DragData> dragData = CreateDragData({ MAX_PIXEL_MAP_WIDTH, MAX_PIXEL_MAP_HEIGHT },
        MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN, TOUCH_POINTER_ID, DISPLAY_ID, { DRAG_SRC_X, DRAG_SRC_Y });
    ASSERT_TRUE(dragData);
    int32_t ret = InteractionManager::GetInstance()->StartDrag(dragData.value(), callback);
    ASSERT_EQ(ret, RET_OK);
    SimulateMoveEvent({ DRAG_SRC_X, DRAG_SRC_Y }, { DRAG_DST_X, DRAG_DST_Y },
        MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN, TOUCH_POINTER_ID, true);
    SimulateUpEvent({ DRAG_DST_X, DRAG_DST_Y }, MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN, TOUCH_POINTER_ID);
    InteractionManager::GetInstance()->StopDrag(DragResult::DRAG_SUCCESS, HAS_CUSTOM_ANIMATION);
}

/**
 * @tc.name: InteractionManagerTest_StopDrag_Touch
 * @tc.desc: Stop drag
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InteractionManagerTest, InteractionManagerTest_StopDrag_Touch, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    bool stopCallbackFlag = false;
    auto callback = [&stopCallbackFlag](const DragNotifyMsg& notifyMessage) {
        FI_HILOGD("displayX:%{public}d, displayY:%{public}d, result:%{public}d, target:%{public}d",
            notifyMessage.displayX, notifyMessage.displayY, notifyMessage.result, notifyMessage.targetPid);
        stopCallbackFlag = true;
        FI_HILOGD("set stopCallbackFlag to true");
    };
    SimulateDownEvent({ DRAG_SRC_X, DRAG_SRC_Y }, MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN, TOUCH_POINTER_ID);
    sleep(TIME_WAIT_FOR_TOUCH_DOWN);
    std::optional<DragData> dragData = CreateDragData({ MAX_PIXEL_MAP_WIDTH, MAX_PIXEL_MAP_HEIGHT },
        MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN, TOUCH_POINTER_ID, DISPLAY_ID, { DRAG_SRC_X, DRAG_SRC_Y });
    ASSERT_TRUE(dragData);
    InteractionManager::GetInstance()->StartDrag(dragData.value(), callback);
    SimulateUpEvent({ DRAG_DST_X, DRAG_DST_Y }, MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN, TOUCH_POINTER_ID);
    int32_t ret = InteractionManager::GetInstance()->StopDrag(DragResult::DRAG_SUCCESS, HAS_CUSTOM_ANIMATION);
    ASSERT_EQ(ret, RET_OK);
    usleep(TIME_WAIT_FOR_PROCESS_CALLBACK);
    ASSERT_TRUE(stopCallbackFlag);
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
    int32_t targetPid = InteractionManager::GetInstance()->GetDragTargetPid();
    FI_HILOGD("target:%{public}d", targetPid);
    ASSERT_TRUE(targetPid >= -1);
}

/**
* @tc.name: InteractionManagerTest_TouchEventDispatch
* @tc.desc: Get Drag Target Pid
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(InteractionManagerTest, TouchEventDispatch, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    auto callback = [](const DragNotifyMsg& notifyMessage) {
        FI_HILOGD("displayX:%{public}d, displayY:%{public}d, result:%{public}d, target:%{public}d",
            notifyMessage.displayX, notifyMessage.displayY, notifyMessage.result, notifyMessage.targetPid);
    };
    SimulateDownEvent({ DRAG_SRC_X, DRAG_SRC_Y }, MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN, TOUCH_POINTER_ID);
    sleep(TIME_WAIT_FOR_TOUCH_DOWN);
    std::optional<DragData> dragData = CreateDragData({ MAX_PIXEL_MAP_WIDTH, MAX_PIXEL_MAP_HEIGHT },
        MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN, TOUCH_POINTER_ID, DISPLAY_ID, { DRAG_SRC_X, DRAG_SRC_Y });
    ASSERT_TRUE(dragData);
    int32_t ret = InteractionManager::GetInstance()->StartDrag(dragData.value(), callback);
    ASSERT_EQ(ret, RET_OK);

    auto callbackPtr = std::make_shared<InputEventCallbackTest>();
    ASSERT_TRUE(callbackPtr != nullptr);
    int32_t monitorId = TestAddMonitor(callbackPtr);
    SimulateMoveEvent({ DRAG_SRC_X, DRAG_SRC_Y }, { DRAG_SRC_X, DRAG_SRC_Y },
        MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN, TOUCH_POINTER_ID, true);

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    TestRemoveMonitor(monitorId);
    InteractionManager::GetInstance()->StopDrag(DragResult::DRAG_SUCCESS, HAS_CUSTOM_ANIMATION);
}

/**
* @tc.name: InteractionManagerTest_MouseEventDispatch
* @tc.desc: Get Drag Target Pid
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(InteractionManagerTest, MouseEventDispatch, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    auto callback = [](const DragNotifyMsg& notifyMessage) {
        FI_HILOGD("displayX:%{public}d, displayY:%{public}d, result:%{public}d, target:%{public}d",
            notifyMessage.displayX, notifyMessage.displayY, notifyMessage.result, notifyMessage.targetPid);
    };
    SimulateDownEvent({ DRAG_SRC_X, DRAG_SRC_Y }, MMI::PointerEvent::SOURCE_TYPE_MOUSE, 0);
    std::optional<DragData> dragData = CreateDragData({ MAX_PIXEL_MAP_WIDTH, MAX_PIXEL_MAP_HEIGHT },
        MMI::PointerEvent::SOURCE_TYPE_MOUSE, 0, DISPLAY_ID, { DRAG_SRC_X, DRAG_SRC_Y });
    ASSERT_TRUE(dragData);
    int32_t ret = InteractionManager::GetInstance()->StartDrag(dragData.value(), callback);
    ASSERT_EQ(ret, RET_OK);

    auto callbackPtr = std::make_shared<InputEventCallbackTest>();
    ASSERT_TRUE(callbackPtr != nullptr);
    int32_t monitorId = TestAddMonitor(callbackPtr);
    SimulateMoveEvent({ DRAG_SRC_X, DRAG_SRC_Y }, { DRAG_SRC_X, DRAG_SRC_Y },
        MMI::PointerEvent::SOURCE_TYPE_MOUSE, 0, true);

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    TestRemoveMonitor(monitorId);
    InteractionManager::GetInstance()->StopDrag(DragResult::DRAG_SUCCESS, HAS_CUSTOM_ANIMATION);
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
