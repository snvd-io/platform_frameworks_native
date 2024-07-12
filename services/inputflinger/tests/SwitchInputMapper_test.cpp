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

#include "SwitchInputMapper.h"

#include <list>
#include <variant>

#include <NotifyArgs.h>
#include <gtest/gtest.h>
#include <input/Input.h>
#include <linux/input-event-codes.h>

#include "InputMapperTest.h"

namespace android {

class SwitchInputMapperTest : public InputMapperTest {
protected:
};

TEST_F(SwitchInputMapperTest, GetSources) {
    SwitchInputMapper& mapper = constructAndAddMapper<SwitchInputMapper>();

    ASSERT_EQ(uint32_t(AINPUT_SOURCE_SWITCH), mapper.getSources());
}

TEST_F(SwitchInputMapperTest, GetSwitchState) {
    SwitchInputMapper& mapper = constructAndAddMapper<SwitchInputMapper>();

    mFakeEventHub->setSwitchState(EVENTHUB_ID, SW_LID, 1);
    ASSERT_EQ(1, mapper.getSwitchState(AINPUT_SOURCE_ANY, SW_LID));

    mFakeEventHub->setSwitchState(EVENTHUB_ID, SW_LID, 0);
    ASSERT_EQ(0, mapper.getSwitchState(AINPUT_SOURCE_ANY, SW_LID));
}

TEST_F(SwitchInputMapperTest, Process) {
    SwitchInputMapper& mapper = constructAndAddMapper<SwitchInputMapper>();
    std::list<NotifyArgs> out;
    out = process(mapper, ARBITRARY_TIME, READ_TIME, EV_SW, SW_LID, 1);
    ASSERT_TRUE(out.empty());
    out = process(mapper, ARBITRARY_TIME, READ_TIME, EV_SW, SW_JACK_PHYSICAL_INSERT, 1);
    ASSERT_TRUE(out.empty());
    out = process(mapper, ARBITRARY_TIME, READ_TIME, EV_SW, SW_HEADPHONE_INSERT, 0);
    ASSERT_TRUE(out.empty());
    out = process(mapper, ARBITRARY_TIME, READ_TIME, EV_SYN, SYN_REPORT, 0);

    ASSERT_EQ(1u, out.size());
    const NotifySwitchArgs& args = std::get<NotifySwitchArgs>(*out.begin());
    ASSERT_EQ(ARBITRARY_TIME, args.eventTime);
    ASSERT_EQ((1U << SW_LID) | (1U << SW_JACK_PHYSICAL_INSERT), args.switchValues);
    ASSERT_EQ((1U << SW_LID) | (1U << SW_JACK_PHYSICAL_INSERT) | (1 << SW_HEADPHONE_INSERT),
              args.switchMask);
    ASSERT_EQ(uint32_t(0), args.policyFlags);
}

} // namespace android