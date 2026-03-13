#include "Panel.hpp"

START_NAMESPACE_DISTRHO

Panel::Panel(Widget *widget)
    : WidgetGroup(widget)
{
    // font_size = 18.f;
}

void Panel::onNanoDisplay()
{
    const float width = getWidth();
    const float height = getHeight();

    beginPath();
    fillColor(background_color);
    if(roundCorner){
        roundedRect(0, 0, width, height, radius);
    } else {
        rect(0, 0, width, height);
    }
    fill();
    closePath();
}

END_NAMESPACE_DISTRHO