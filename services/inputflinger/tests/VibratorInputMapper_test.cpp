/*
 * Copyright 2024 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "VibratorInputMapper.h"

#include <chrono>
#include <list>
#include <variant>
#include <vector>

#include <EventHub.h>
#include <NotifyArgs.h>
#include <gtest/gtest.h>
#include <input/Input.h>

#include "InputMapperTest.h"

namespace android {

class VibratorInputMapperTest : public InputMapperTest {
protected:
    void SetUp() override { InputMapperTest::SetUp(DEVICE_CLASSES | InputDeviceClass::VIBRATOR); }
};

TEST_F(VibratorInputMapperTest, GetSources) {
    VibratorInputMapper& mapper = constructAndAddMapper<VibratorInputMapper>();

    ASSERT_EQ(AINPUT_SOURCE_UNKNOWN, mapper.getSources());
}

TEST_F(VibratorInputMapperTest, GetVibratorIds) {
    VibratorInputMapper& mapper = constructAndAddMapper<VibratorInputMapper>();

    ASSERT_EQ(mapper.getVibratorIds().size(), 2U);
}

TEST_F(VibratorInputMapperTest, Vibrate) {
    constexpr uint8_t DEFAULT_AMPLITUDE = 192;
    constexpr int32_t VIBRATION_TOKEN = 100;
    VibratorInputMapper& mapper = constructAndAddMapper<VibratorInputMapper>();

    VibrationElement pattern(2);
    VibrationSequence sequence(2);
    pattern.duration = std::chrono::milliseconds(200);
    pattern.channels = {{/*vibratorId=*/0, DEFAULT_AMPLITUDE / 2},
                        {/*vibratorId=*/1, DEFAULT_AMPLITUDE}};
    sequence.addElement(pattern);
    pattern.duration = std::chrono::milliseconds(500);
    pattern.channels = {{/*vibratorId=*/0, DEFAULT_AMPLITUDE / 4},
                        {/*vibratorId=*/1, DEFAULT_AMPLITUDE}};
    sequence.addElement(pattern);

    std::vector<int64_t> timings = {0, 1};
    std::vector<uint8_t> amplitudes = {DEFAULT_AMPLITUDE, DEFAULT_AMPLITUDE / 2};

    ASSERT_FALSE(mapper.isVibrating());
    // Start vibrating
    std::list<NotifyArgs> out = mapper.vibrate(sequence, /*repeat=*/-1, VIBRATION_TOKEN);
    ASSERT_TRUE(mapper.isVibrating());
    // Verify vibrator state listener was notified.
    mReader->loopOnce();
    ASSERT_EQ(1u, out.size());
    const NotifyVibratorStateArgs& vibrateArgs = std::get<NotifyVibratorStateArgs>(*out.begin());
    ASSERT_EQ(DEVICE_ID, vibrateArgs.deviceId);
    ASSERT_TRUE(vibrateArgs.isOn);
    // Stop vibrating
    out = mapper.cancelVibrate(VIBRATION_TOKEN);
    ASSERT_FALSE(mapper.isVibrating());
    // Verify vibrator state listener was notified.
    mReader->loopOnce();
    ASSERT_EQ(1u, out.size());
    const NotifyVibratorStateArgs& cancelArgs = std::get<NotifyVibratorStateArgs>(*out.begin());
    ASSERT_EQ(DEVICE_ID, cancelArgs.deviceId);
    ASSERT_FALSE(cancelArgs.isOn);
}

} // namespace android