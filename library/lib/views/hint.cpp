/*
    Copyright 2021 XITRIX

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include <borealis/core/application.hpp>
#include <borealis/core/logger.hpp>
#include <borealis/core/util.hpp>
#include <borealis/core/touch/tap_gesture.hpp>
#include <borealis/views/applet_frame.hpp>
#include <borealis/views/hint.hpp>

namespace brls
{

const std::string hintXML = R"xml(
    <brls:Box
        width="auto"
        height="auto"
        axis="row"
        marginLeft="32">
            <brls:Label
                id="icon"
                width="auto"
                height="auto"
                fontSize="25.5"/>

            <brls:Label
                id="hint"
                width="auto"
                height="auto"
                fontSize="21.5"
                marginLeft="8"/>

    </brls:Box>
)xml";

Hint::Hint(Action action)
    : Box(Axis::ROW), action(action)
{
    this->inflateFromXMLString(hintXML);
    this->setFocusable(false);

    icon->setText(getKeyIcon(action.button));
    hint->setText(action.hintText);

    if (!action.available)
    {
        icon->setTextColor(nvgRGB(80, 80, 80));
        hint->setTextColor(nvgRGB(80, 80, 80));
    }
}

std::string Hint::getKeyIcon(ControllerButton button)
{
    switch (button)
    {
        case BUTTON_A:
            return "\uE0E0";
        case BUTTON_B:
            return "\uE0E1";
        case BUTTON_X:
            return "\uE0E2";
        case BUTTON_Y:
            return "\uE0E3";
        case BUTTON_LSB:
            return "\uE104";
        case BUTTON_RSB:
            return "\uE105";
        case BUTTON_LB:
            return "\uE0E4";
        case BUTTON_RB:
            return "\uE0E5";
        case BUTTON_START:
            return "\uE0EF";
        case BUTTON_GUIDE:
            return "\uE0F0";
        case BUTTON_LEFT:
            return "\uE0ED";
        case BUTTON_UP:
            return "\uE0EB";
        case BUTTON_RIGHT:
            return "\uE0EF";
        case BUTTON_DOWN:
            return "\uE0EC";
        default:
            return "\uE152";
    }
}

} // namespace brls
