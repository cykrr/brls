//
//  dropdown.cpp
//  borealis
//
//  Created by Даниил Виноградов on 14.05.2021.
//

#include "borealis/views/dropdown.hpp"

#include "borealis/core/application.hpp"
#include "borealis/views/cells/cell_radio.hpp"

namespace brls
{

const std::string dropdownFrameXML = R"xml(
    <brls:Box
        width="auto"
        height="auto"
        justifyContent="flexEnd"
        alignItems="stretch"
        axis="column"
        backgroundColor="@theme/brls/backdrop">
        <brls:AppletFrame
            style="dropdown"
            id="brls/dropdown/applet"
            width="auto"
            height="auto"
            maxHeight="80%"
            backgroundColor="@theme/brls/background">
            <brls:Box
                width="auto"
                height="auto"
                axis="row"
                justifyContent="center"
                alignItems="stretch">
                <brls:RecyclerFrame
                    id="brls/dropdown/recycler"
                    width="70%"
                    height="auto"
                    paddingTop="@style/brls/dropdown/listPadding"
                    paddingRight="@style/brls/dropdown/listPadding"
                    paddingBottom="@style/brls/dropdown/listPadding"
                    paddingLeft="@style/brls/dropdown/listPadding"/>
            </brls:Box>
        </brls:AppletFrame>
    </brls:Box>
)xml";

float min(float a, float b)
{
    if (a < b)
        return a;
    return b;
}

Dropdown::Dropdown(std::string title, std::vector<std::string> values, ValueSelectedEvent::Callback cb, int selected)
    : values(values)
    , cb(cb)
    , selected(selected)
{
    this->inflateFromXMLString(dropdownFrameXML);

    recycler->estimatedRowHeight = Application::getStyle()["brls/dropdown/listItemHeight"];
    recycler->registerCell("Cell", []() { return new RadioCell(); });
    recycler->setDefaultCellFocus(IndexPath(0, selected));
    recycler->setTitle(title);
    recycler->setDataSource(this);

    Style style = Application::getStyle();

    float height = numberOfRows(recycler, 0) * style["brls/dropdown/listItemHeight"]
        + applet->getHeader()->getHeight()
        + applet->getFooter()->getHeight()
        + style["brls/dropdown/listPadding"] // top
        + style["brls/dropdown/listPadding"] // bottom
        ;

    applet->setHeight(min(height, 576));
}

int Dropdown::numberOfRows(RecyclerFrame* recycler, int section)
{
    return (int)values.size();
}

RecyclerCell* Dropdown::cellForRow(RecyclerFrame* recycler, IndexPath index)
{
    RadioCell* cell = (RadioCell*)recycler->dequeueReusableCell("Cell");
    cell->title->setText(values[index.row]);
    cell->setSelected(index.row == selected);
    return cell;
}

void Dropdown::didSelectRowAt(RecyclerFrame* recycler, IndexPath index)
{
    this->cb(index.row);
    Application::popActivity();
}

AppletFrame* Dropdown::getAppletFrame()
{
    return applet;
}

float Dropdown::getShowAnimationDuration(TransitionAnimation animation)
{
    return View::getShowAnimationDuration(animation) / 2;
}

} // namespace brls
