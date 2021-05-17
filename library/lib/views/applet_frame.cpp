/*
    Copyright 2019-2021 natinusala
    Copyright 2021 XITRIX
    Copyright 2019 p-sam

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
#include <borealis/core/i18n.hpp>
#include <borealis/core/logger.hpp>
#include <borealis/core/util.hpp>
#include <borealis/views/applet_frame.hpp>
#include <borealis/views/hint.hpp>

using namespace brls::literals;

namespace brls
{

const std::string appletFrameXML = R"xml(
    <brls:Box
        width="auto"
        height="auto"
        axis="column"
        justifyContent="spaceBetween">

        <!-- Header -->
        <brls:Box
            id="brls/applet_frame/header"
            width="auto"
            height="@style/brls/applet_frame/header_height"
            axis="row"
            paddingTop="@style/brls/applet_frame/header_padding_top_bottom"
            paddingBottom="@style/brls/applet_frame/header_padding_top_bottom"
            paddingLeft="@style/brls/applet_frame/header_padding_sides"
            paddingRight="@style/brls/applet_frame/header_padding_sides"
            marginLeft="@style/brls/applet_frame/padding_sides"
            marginRight="@style/brls/applet_frame/padding_sides"
            lineColor="@theme/brls/applet_frame/separator"
            lineBottom="1px">

            <brls:Image
                id="brls/applet_frame/title_icon"
                width="auto"
                height="auto"
                marginRight="@style/brls/applet_frame/header_image_title_spacing"
                visibility="gone" />

            <brls:Label
                id="brls/applet_frame/title_label"
                width="auto"
                height="auto"
                marginTop="@style/brls/applet_frame/header_title_top_offset"
                fontSize="@style/brls/applet_frame/header_title_font_size" />

        </brls:Box>

        <!-- Content will be injected here with grow="1.0" -->

        <!--
            Footer
            Direction inverted so that the bottom left text can be
            set to visibility="gone" without affecting the hint
        -->
        <brls:Box
            id="brls/applet_frame/footer"
            width="auto"
            height="@style/brls/applet_frame/footer_height"
            axis="row"
            direction="rightToLeft"
            paddingLeft="@style/brls/applet_frame/footer_padding_sides"
            paddingRight="@style/brls/applet_frame/footer_padding_sides"
            paddingTop="@style/brls/applet_frame/footer_padding_top_bottom"
            paddingBottom="@style/brls/applet_frame/footer_padding_top_bottom"
            marginLeft="@style/brls/applet_frame/padding_sides"
            marginRight="@style/brls/applet_frame/padding_sides"
            lineColor="@theme/brls/applet_frame/separator"
            lineTop="1px"
            justifyContent="spaceBetween" >

            <brls:Box
                id="hints"
                width="auto"
                height="auto"
                axis="row"
                direction="leftToRight" />

            <brls:Rectangle
                width="75px"
                height="auto"
                color="#FF00FF" />

        </brls:Box>

    </brls:Box>
)xml";

AppletFrame::AppletFrame()
{
    this->inflateFromXMLString(appletFrameXML);

    this->forwardXMLAttribute("iconInterpolation", this->icon, "interpolation");

    BRLS_REGISTER_ENUM_XML_ATTRIBUTE(
        "style", HeaderStyle, this->setHeaderStyle,
        {
            { "regular", HeaderStyle::REGULAR },
            { "dropdown", HeaderStyle::DROPDOWN },
            { "popup", HeaderStyle::POPUP },
        });

    this->registerBoolXMLAttribute("headerHidden", [this](bool value) {
        this->setHeaderVisibility(value ? Visibility::GONE : Visibility::VISIBLE);
    });

    this->registerBoolXMLAttribute("footerHidden", [this](bool value) {
        this->setFooterVisibility(value ? Visibility::GONE : Visibility::VISIBLE);
    });

    hintSubscription = Application::getGlobalFocusChangeEvent()->subscribe([this](View* view) {
        refillHints(view);
    });

    this->registerAction(
        "brls/hints/back"_i18n, BUTTON_B, [this](View* view) {
            this->contentViewStack.back()->dismiss();
            return true;
        },
        false, SOUND_BACK);
}

AppletFrame::AppletFrame(View* contentView)
    : AppletFrame::AppletFrame()
{
    contentViewStack.push_back(contentView);
    this->setContentView(contentView);
}

AppletFrame::~AppletFrame()
{
    Application::getGlobalFocusChangeEvent()->unsubscribe(hintSubscription);
}

bool actionsSortFunc(Action a, Action b)
{
    // From left to right:
    //  - first +
    //  - then all hints that are not B and A
    //  - finally B and A

    // + is before all others
    if (a.button == BUTTON_START)
        return true;

    // A is after all others
    if (b.button == BUTTON_A)
        return true;

    // B is after all others but A
    if (b.button == BUTTON_B && a.button != BUTTON_A)
        return true;

    // Keep original order for the rest
    return false;
}

void AppletFrame::refillHints(View* focusView)
{
    if (!focusView)
        return;

    hints->clearViews();

    std::set<ControllerButton> addedButtons; // we only ever want one action per key
    std::vector<Action> actions;

    while (focusView != nullptr)
    {
        for (auto& action : focusView->getActions())
        {
            if (action.hidden)
                continue;

            if (addedButtons.find(action.button) != addedButtons.end())
                continue;

            addedButtons.insert(action.button);
            actions.push_back(action);
        }

        focusView = focusView->getParent();
    }

    if (std::find(actions.begin(), actions.end(), BUTTON_A) == actions.end())
    {
        actions.push_back(Action { BUTTON_A, NULL, "brls/hints/ok"_i18n, false, false, Sound::SOUND_NONE, NULL });
    }

    // Sort the actions
    std::stable_sort(actions.begin(), actions.end(), actionsSortFunc);

    for (Action action : actions)
    {
        Hint* hint = new Hint(action);
        hints->addView(hint);
    }
}

void AppletFrame::setIconFromRes(std::string name)
{
    this->icon->setVisibility(Visibility::VISIBLE);
    this->icon->setImageFromRes(name);
}

void AppletFrame::setIconFromFile(std::string path)
{
    if (path.empty())
    {
        this->icon->setVisibility(Visibility::GONE);
    }
    else
    {
        this->icon->setVisibility(Visibility::VISIBLE);
        this->icon->setImageFromFile(path);
    }
}

void AppletFrame::setHeaderVisibility(Visibility visibility)
{
    header->setVisibility(visibility);
}

void AppletFrame::setFooterVisibility(Visibility visibility)
{
    footer->setVisibility(visibility);
}

void AppletFrame::setTitle(std::string title)
{
    this->title->setText(title);
}

void AppletFrame::pushContentView(View* view)
{
    contentViewStack.push_back(view);
    setContentView(view);
    Application::giveFocus(view);
}

void AppletFrame::popContentView()
{
    if (contentViewStack.size() <= 1)
    {
        if (!Application::popActivity())
            Application::quit();
        return;
    }

    View* lastView = contentViewStack.back();
    contentViewStack.pop_back();

    View* newView = contentViewStack.back();
    setContentView(newView);
    Application::giveFocus(newView);

    lastView->freeView();
}

void AppletFrame::setContentView(View* view)
{
    if (this->contentView)
    {
        // Remove the node
        this->removeView(this->contentView, false);
        this->contentView = nullptr;
    }

    if (!view)
        return;

    this->contentView = view;

    this->contentView->setDimensions(View::AUTO, View::AUTO);
    this->contentView->setGrow(1.0f);

    this->addView(this->contentView, 1);

    this->setTitle(view->getTitle());
    this->setIconFromFile(view->getIconFile());
}

void AppletFrame::handleXMLElement(tinyxml2::XMLElement* element)
{
    if (this->contentView)
        fatal("brls:AppletFrame can only have one child XML element");

    View* view = View::createFromXMLElement(element);
    contentViewStack.push_back(view);
    this->setContentView(view);
}

void AppletFrame::setHeaderStyle(HeaderStyle style)
{
    this->style = style;

    Style appStyle = Application::getStyle();
    switch (style)
    {
        case HeaderStyle::REGULAR:
            header->setHeight(appStyle["brls/applet_frame/header_height"]);
            title->setFontSize(appStyle["brls/applet_frame/header_title_font_size"]);
            break;
        case HeaderStyle::DROPDOWN:
            header->setHeight(appStyle["brls/applet_frame/dropdown_header_height"]);
            title->setFontSize(appStyle["brls/applet_frame/dropdown_header_title_font_size"]);
            break;
        default:
            break;
    }
}

View* AppletFrame::create()
{
    return new AppletFrame();
}

} // namespace brls
