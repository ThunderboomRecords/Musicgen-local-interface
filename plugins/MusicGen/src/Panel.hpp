#ifndef PANEL_HPP_INCLUDED
#define PANEL_HPP_INCLUDED

#include "WidgetGroup.hpp"

START_NAMESPACE_DISTRHO

class Panel : public WidgetGroup
{
public:
    Panel(Widget *widget);

    bool roundCorner = false;
    float radius = 5;

protected:
    void onNanoDisplay() override;

    DISTRHO_LEAK_DETECTOR(Panel);
};

END_NAMESPACE_DISTRHO

#endif