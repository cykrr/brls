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

#pragma once

#include <borealis/core/event.hpp>
#include <borealis/core/bind.hpp>
#include <borealis/core/box.hpp>
#include <borealis/views/image.hpp>
#include <borealis/views/label.hpp>

namespace brls
{

// A Horizon settings-like frame, with header and footer (no sidebar)
class AppletFrame : public Box
{
  public:
    AppletFrame();
    AppletFrame(View* contentView);
    ~AppletFrame();

    void handleXMLElement(tinyxml2::XMLElement* element) override;

    void pushContentView(View* view);
    View* popContentView();

    void setTitle(std::string title);

    void setIconFromRes(std::string name);
    void setIconFromFile(std::string path);
    
    void setHeaderVisibility(Visibility visibility);
    void setFooterVisibility(Visibility visibility);

    static View* create();

  private:
    GenericEvent::Subscription hintSubscription;
    void refillHints(View* focusView);

    BRLS_BIND(Box, header, "brls/applet_frame/header");
    BRLS_BIND(Box, footer, "brls/applet_frame/footer");
    BRLS_BIND(Label, title, "brls/applet_frame/title_label");
    BRLS_BIND(Image, icon, "brls/applet_frame/title_icon");
    BRLS_BIND(Box, hints, "hints");

  protected:
    std::vector<View*> contentViewStack;
    View* contentView = nullptr;
    
    /**
     * Sets the content view for that AppletFrame.
     * Will be placed between header and footer and expanded with grow factor
     * and width / height to AUTO.
     */
    void setContentView(View* view);
};

} // namespace brls
