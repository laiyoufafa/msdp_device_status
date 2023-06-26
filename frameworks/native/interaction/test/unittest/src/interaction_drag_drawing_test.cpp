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

#include <future>
#include <optional>
#include <vector>

#include <unistd.h>

#include <gtest/gtest.h>
#include "pointer_event.h"
#include "securec.h"

#include "devicestatus_define.h"
#include "devicestatus_errors.h"
#include "interaction_manager.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
using namespace testing::ext;
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "InteractionDragDrawingTest" };
constexpr uint32_t DEFAULT_ICON_COLOR { 0xFF };
constexpr int32_t TIME_WAIT_FOR_OP_MS { 20 };
constexpr int32_t PIXEL_MAP_WIDTH { 300 };
constexpr int32_t PIXEL_MAP_HEIGHT { 300 };
constexpr int32_t MAX_PIXEL_MAP_WIDTH { 600 };
constexpr int32_t MAX_PIXEL_MAP_HEIGHT { 600 };
constexpr int32_t POINTER_ID { 0 };
constexpr int32_t DISPLAY_ID { 0 };
constexpr int32_t DISPLAY_X { 300 };
constexpr int32_t DISPLAY_Y { 300 };
constexpr int32_t DRAG_NUM_ONE { 1 };
constexpr int32_t DRAG_NUM_MULTIPLE { 10 };
constexpr int32_t INT32_BYTE { 4 };
constexpr int32_t PROMISE_WAIT_SPAN_MS { 2000 };
constexpr int32_t TIME_WAIT_FOR_UPDATE_DRAG_STYLE { 50 };
constexpr int32_t TIME_WAIT_FOR_ANIMATION_END { 1000 };
constexpr bool HAS_CANCELED_ANIMATION { true };
constexpr bool HAS_CUSTOM_ANIMATION { true };
constexpr bool NOT_HAS_CUSTOM_ANIMATION { false };
constexpr bool DRAG_WINDOW_VISIBLE { true };
const std::string UD_KEY { "Unified data key" };
} // namespace

class InteractionDragDrawingTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static std::shared_ptr<Media::PixelMap> CreatePixelMap(int32_t width, int32_t height);
    static std::optional<DragData> CreateDragData(int32_t sourceType, int32_t pointerId, int32_t dragNum);
};

void InteractionDragDrawingTest::SetUpTestCase() {}

void InteractionDragDrawingTest::SetUp() {}

void InteractionDragDrawingTest::TearDown()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_WAIT_FOR_OP_MS));
}

std::shared_ptr<Media::PixelMap> InteractionDragDrawingTest::CreatePixelMap(int32_t width, int32_t height)
{
    CALL_DEBUG_ENTER;
    if (width <= 0 || width > MAX_PIXEL_MAP_WIDTH || height <= 0 || height > MAX_PIXEL_MAP_HEIGHT) {
        FI_HILOGE("Invalid size, width:%{public}d, height:%{public}d", width, height);
        return nullptr;
    }
    Media::InitializationOptions opts;
    opts.size.width = width;
    opts.size.height = height;
    opts.pixelFormat = Media::PixelFormat::BGRA_8888;
    opts.alphaType = Media::AlphaType::IMAGE_ALPHA_TYPE_OPAQUE;
    opts.scaleMode = Media::ScaleMode::FIT_TARGET_SIZE;

    int32_t colorLen = width * height;
    uint32_t *colorPixels = new (std::nothrow) uint32_t[colorLen];
    CHKPP(colorPixels);
    int32_t colorByteCount = colorLen * INT32_BYTE;
    errno_t ret = memset_s(colorPixels, colorByteCount, DEFAULT_ICON_COLOR, colorByteCount);
    if (ret != EOK) {
        FI_HILOGE("memset_s failed");
        delete[] colorPixels;
        return nullptr;
    }
    std::shared_ptr<Media::PixelMap> pixelMap = Media::PixelMap::Create(colorPixels, colorLen, opts);
    CHKPL(pixelMap);
    delete[] colorPixels;
    return pixelMap;
}

std::optional<DragData> InteractionDragDrawingTest::CreateDragData(int32_t sourceType,
    int32_t pointerId, int32_t dragNum)
{
    CALL_DEBUG_ENTER;
    std::shared_ptr<Media::PixelMap> pixelMap = CreatePixelMap(PIXEL_MAP_WIDTH, PIXEL_MAP_HEIGHT);
    if (pixelMap == nullptr) {
        FI_HILOGE("Create pixelmap failed");
        return std::nullopt;
    }
    DragData dragData;
    dragData.shadowInfo.pixelMap = pixelMap;
    dragData.shadowInfo.x = 0;
    dragData.shadowInfo.y = 0;
    dragData.buffer = std::vector<uint8_t>(MAX_BUFFER_SIZE, 0);
    dragData.udKey = UD_KEY;
    dragData.sourceType = sourceType;
    dragData.pointerId = pointerId;
    dragData.dragNum = dragNum;
    dragData.displayX = DISPLAY_X;
    dragData.displayY = DISPLAY_Y;
    dragData.displayId = DISPLAY_ID;
    dragData.hasCanceledAnimation = HAS_CANCELED_ANIMATION;
    return dragData;
}

/**
 * @tc.name: InteractionDragDrawingTest_Mouse_DragNum_One
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InteractionDragDrawingTest, InteractionDragDrawingTest_Mouse_DragNum_One, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    std::promise<bool> promiseFlag;
    std::future<bool> futureFlag = promiseFlag.get_future();
    auto callback = [&promiseFlag](const DragNotifyMsg& notifyMessage) {
        FI_HILOGD("displayX:%{public}d, displayY:%{public}d, result:%{public}d, target:%{public}d",
            notifyMessage.displayX, notifyMessage.displayY, notifyMessage.result, notifyMessage.targetPid);
        promiseFlag.set_value(true);
    };
    std::optional<DragData> dragData = CreateDragData(
        MMI::PointerEvent::SOURCE_TYPE_MOUSE, POINTER_ID, DRAG_NUM_ONE);
    ASSERT_TRUE(dragData);
    int32_t ret = InteractionManager::GetInstance()->StartDrag(dragData.value(), callback);
    ASSERT_EQ(ret, RET_OK);
    ret = InteractionManager::GetInstance()->SetDragWindowVisible(DRAG_WINDOW_VISIBLE);
    ASSERT_EQ(ret, RET_OK);
    ret = InteractionManager::GetInstance()->UpdateDragStyle(DragCursorStyle::COPY);
    ASSERT_EQ(ret, RET_OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_WAIT_FOR_UPDATE_DRAG_STYLE));
    ret = InteractionManager::GetInstance()->UpdateDragStyle(DragCursorStyle::FORBIDDEN);
    ASSERT_EQ(ret, RET_OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_WAIT_FOR_UPDATE_DRAG_STYLE));
    ret = InteractionManager::GetInstance()->UpdateDragStyle(DragCursorStyle::MOVE);
    ASSERT_EQ(ret, RET_OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_WAIT_FOR_UPDATE_DRAG_STYLE));
    ret = InteractionManager::GetInstance()->UpdateDragStyle(DragCursorStyle::DEFAULT);
    ASSERT_EQ(ret, RET_OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_WAIT_FOR_UPDATE_DRAG_STYLE));
    ret = InteractionManager::GetInstance()->StopDrag(DragResult::DRAG_SUCCESS, HAS_CUSTOM_ANIMATION);
    ASSERT_EQ(ret, RET_OK);
    ASSERT_TRUE(futureFlag.wait_for(std::chrono::milliseconds(PROMISE_WAIT_SPAN_MS)) != std::future_status::timeout);
}

/**
 * @tc.name: InteractionDragDrawingTest_Mouse_DragNum_Multiple
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InteractionDragDrawingTest, InteractionDragDrawingTest_Mouse_DragNum_Multiple, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    std::promise<bool> promiseFlag;
    std::future<bool> futureFlag = promiseFlag.get_future();
    auto callback = [&promiseFlag](const DragNotifyMsg& notifyMessage) {
        FI_HILOGD("displayX:%{public}d, displayY:%{public}d, result:%{public}d, target:%{public}d",
            notifyMessage.displayX, notifyMessage.displayY, notifyMessage.result, notifyMessage.targetPid);
        promiseFlag.set_value(true);
    };
    std::optional<DragData> dragData = CreateDragData(
        MMI::PointerEvent::SOURCE_TYPE_MOUSE, POINTER_ID, DRAG_NUM_MULTIPLE);
    ASSERT_TRUE(dragData);
    int32_t ret = InteractionManager::GetInstance()->StartDrag(dragData.value(), callback);
    ASSERT_EQ(ret, RET_OK);
    ret = InteractionManager::GetInstance()->SetDragWindowVisible(DRAG_WINDOW_VISIBLE);
    ASSERT_EQ(ret, RET_OK);
    ret = InteractionManager::GetInstance()->UpdateDragStyle(DragCursorStyle::COPY);
    ASSERT_EQ(ret, RET_OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_WAIT_FOR_UPDATE_DRAG_STYLE));
    ret = InteractionManager::GetInstance()->UpdateDragStyle(DragCursorStyle::FORBIDDEN);
    ASSERT_EQ(ret, RET_OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_WAIT_FOR_UPDATE_DRAG_STYLE));
    ret = InteractionManager::GetInstance()->UpdateDragStyle(DragCursorStyle::DEFAULT);
    ASSERT_EQ(ret, RET_OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_WAIT_FOR_UPDATE_DRAG_STYLE));
    ret = InteractionManager::GetInstance()->UpdateDragStyle(DragCursorStyle::MOVE);
    ASSERT_EQ(ret, RET_OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_WAIT_FOR_UPDATE_DRAG_STYLE));
    ret = InteractionManager::GetInstance()->StopDrag(DragResult::DRAG_SUCCESS, HAS_CUSTOM_ANIMATION);
    ASSERT_EQ(ret, RET_OK);
    ASSERT_TRUE(futureFlag.wait_for(std::chrono::milliseconds(PROMISE_WAIT_SPAN_MS)) != std::future_status::timeout);
}

/**
 * @tc.name: InteractionDragDrawingTest_Touchscreen_DragNum_One
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InteractionDragDrawingTest, InteractionDragDrawingTest_Touchscreen_DragNum_One, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    std::promise<bool> promiseFlag;
    std::future<bool> futureFlag = promiseFlag.get_future();
    auto callback = [&promiseFlag](const DragNotifyMsg& notifyMessage) {
        FI_HILOGD("displayX:%{public}d, displayY:%{public}d, result:%{public}d, target:%{public}d",
            notifyMessage.displayX, notifyMessage.displayY, notifyMessage.result, notifyMessage.targetPid);
        promiseFlag.set_value(true);
    };
    std::optional<DragData> dragData = CreateDragData(
        MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN, POINTER_ID, DRAG_NUM_ONE);
    ASSERT_TRUE(dragData);
    int32_t ret = InteractionManager::GetInstance()->StartDrag(dragData.value(), callback);
    ASSERT_EQ(ret, RET_OK);
    ret = InteractionManager::GetInstance()->SetDragWindowVisible(DRAG_WINDOW_VISIBLE);
    ASSERT_EQ(ret, RET_OK);
    ret = InteractionManager::GetInstance()->UpdateDragStyle(DragCursorStyle::COPY);
    ASSERT_EQ(ret, RET_OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_WAIT_FOR_UPDATE_DRAG_STYLE));
    ret = InteractionManager::GetInstance()->UpdateDragStyle(DragCursorStyle::FORBIDDEN);
    ASSERT_EQ(ret, RET_OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_WAIT_FOR_UPDATE_DRAG_STYLE));
    ret = InteractionManager::GetInstance()->UpdateDragStyle(DragCursorStyle::MOVE);
    ASSERT_EQ(ret, RET_OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_WAIT_FOR_UPDATE_DRAG_STYLE));
    ret = InteractionManager::GetInstance()->UpdateDragStyle(DragCursorStyle::DEFAULT);
    ASSERT_EQ(ret, RET_OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_WAIT_FOR_UPDATE_DRAG_STYLE));
    ret = InteractionManager::GetInstance()->StopDrag(DragResult::DRAG_SUCCESS, HAS_CUSTOM_ANIMATION);
    ASSERT_EQ(ret, RET_OK);
    ASSERT_TRUE(futureFlag.wait_for(std::chrono::milliseconds(PROMISE_WAIT_SPAN_MS)) != std::future_status::timeout);
}

/**
 * @tc.name: InteractionDragDrawingTest_Touchscreen_DragNum_Multiple
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InteractionDragDrawingTest, InteractionDragDrawingTest_Touchscreen_DragNum_Multiple, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    std::promise<bool> promiseFlag;
    std::future<bool> futureFlag = promiseFlag.get_future();
    auto callback = [&promiseFlag](const DragNotifyMsg& notifyMessage) {
        FI_HILOGD("displayX:%{public}d, displayY:%{public}d, result:%{public}d, target:%{public}d",
            notifyMessage.displayX, notifyMessage.displayY, notifyMessage.result, notifyMessage.targetPid);
        promiseFlag.set_value(true);
    };
    std::optional<DragData> dragData = CreateDragData(
        MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN, POINTER_ID, DRAG_NUM_MULTIPLE);
    ASSERT_TRUE(dragData);
    int32_t ret = InteractionManager::GetInstance()->StartDrag(dragData.value(), callback);
    ASSERT_EQ(ret, RET_OK);
    ret = InteractionManager::GetInstance()->SetDragWindowVisible(DRAG_WINDOW_VISIBLE);
    ASSERT_EQ(ret, RET_OK);
    ret = InteractionManager::GetInstance()->UpdateDragStyle(DragCursorStyle::COPY);
    ASSERT_EQ(ret, RET_OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_WAIT_FOR_UPDATE_DRAG_STYLE));
    ret = InteractionManager::GetInstance()->UpdateDragStyle(DragCursorStyle::FORBIDDEN);
    ASSERT_EQ(ret, RET_OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_WAIT_FOR_UPDATE_DRAG_STYLE));
    ret = InteractionManager::GetInstance()->UpdateDragStyle(DragCursorStyle::MOVE);
    ASSERT_EQ(ret, RET_OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_WAIT_FOR_UPDATE_DRAG_STYLE));
    ret = InteractionManager::GetInstance()->UpdateDragStyle(DragCursorStyle::DEFAULT);
    ASSERT_EQ(ret, RET_OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_WAIT_FOR_UPDATE_DRAG_STYLE));
    ret = InteractionManager::GetInstance()->StopDrag(DragResult::DRAG_SUCCESS, HAS_CUSTOM_ANIMATION);
    ASSERT_EQ(ret, RET_OK);
    ASSERT_TRUE(futureFlag.wait_for(std::chrono::milliseconds(PROMISE_WAIT_SPAN_MS)) != std::future_status::timeout);
}

/**
 * @tc.name: InteractionDragDrawingTest_Mouse_Animation
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InteractionDragDrawingTest, InteractionDragDrawingTest_Mouse_Animation, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    std::promise<bool> promiseFlag;
    std::future<bool> futureFlag = promiseFlag.get_future();
    auto callback = [&promiseFlag](const DragNotifyMsg& notifyMessage) {
        FI_HILOGD("displayX:%{public}d, displayY:%{public}d, result:%{public}d, target:%{public}d",
            notifyMessage.displayX, notifyMessage.displayY, notifyMessage.result, notifyMessage.targetPid);
        promiseFlag.set_value(true);
    };
    std::optional<DragData> dragData = CreateDragData(
        MMI::PointerEvent::SOURCE_TYPE_MOUSE, POINTER_ID, DRAG_NUM_ONE);
    ASSERT_TRUE(dragData);
    int32_t ret = InteractionManager::GetInstance()->StartDrag(dragData.value(), callback);
    ASSERT_EQ(ret, RET_OK);
    ret = InteractionManager::GetInstance()->SetDragWindowVisible(DRAG_WINDOW_VISIBLE);
    ASSERT_EQ(ret, RET_OK);
    ret = InteractionManager::GetInstance()->UpdateDragStyle(DragCursorStyle::COPY);
    ASSERT_EQ(ret, RET_OK);
    ret = InteractionManager::GetInstance()->StopDrag(DragResult::DRAG_SUCCESS, NOT_HAS_CUSTOM_ANIMATION);
    ASSERT_EQ(ret, RET_OK);
    ASSERT_TRUE(futureFlag.wait_for(std::chrono::milliseconds(PROMISE_WAIT_SPAN_MS)) != std::future_status::timeout);
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_WAIT_FOR_ANIMATION_END));
}

/**
 * @tc.name: InteractionDragDrawingTest_Touchscreen_Animation
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InteractionDragDrawingTest, InteractionDragDrawingTest_Touchscreen_Animation, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    std::promise<bool> promiseFlag;
    std::future<bool> futureFlag = promiseFlag.get_future();
    auto callback = [&promiseFlag](const DragNotifyMsg& notifyMessage) {
        FI_HILOGD("displayX:%{public}d, displayY:%{public}d, result:%{public}d, target:%{public}d",
            notifyMessage.displayX, notifyMessage.displayY, notifyMessage.result, notifyMessage.targetPid);
        promiseFlag.set_value(true);
    };
    std::optional<DragData> dragData = CreateDragData(
        MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN, POINTER_ID, DRAG_NUM_ONE);
    ASSERT_TRUE(dragData);
    int32_t ret = InteractionManager::GetInstance()->StartDrag(dragData.value(), callback);
    ASSERT_EQ(ret, RET_OK);
    ret = InteractionManager::GetInstance()->SetDragWindowVisible(DRAG_WINDOW_VISIBLE);
    ASSERT_EQ(ret, RET_OK);
    ret = InteractionManager::GetInstance()->UpdateDragStyle(DragCursorStyle::COPY);
    ASSERT_EQ(ret, RET_OK);
    ret = InteractionManager::GetInstance()->StopDrag(DragResult::DRAG_FAIL, NOT_HAS_CUSTOM_ANIMATION);
    ASSERT_EQ(ret, RET_OK);
    ASSERT_TRUE(futureFlag.wait_for(std::chrono::milliseconds(PROMISE_WAIT_SPAN_MS)) != std::future_status::timeout);
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_WAIT_FOR_ANIMATION_END));
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS