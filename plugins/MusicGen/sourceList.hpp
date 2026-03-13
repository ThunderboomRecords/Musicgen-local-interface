#ifndef KNOB_HPP_INCLUDED
#define KNOB_HPP_INCLUDED

// #include "Window.hpp"
// #include "Widget.hpp"
#include "NanoVG.hpp"
#include <iostream>
#include "WAIVEWidget.hpp"

START_NAMESPACE_DISTRHO

class Knob : public WAIVEWidget
{
public:
    explicit Knob(Widget *widget) noexcept;

protected:
    void onNanoDisplay() override;
    bool onMouse(const MouseEvent &) override;
    bool onMotion(const MotionEvent &) override;
    bool onScroll(const ScrollEvent &) override;
    void drawIndicator();
    void drawLabel();

private:
    Callback *callback;
    bool dragging_, mousedown_;
    float value_, tmp_p;
    float dragStart;
    bool sensitive;

    DISTRHO_LEAK_DETECTOR(Knob);
};

END_NAMESPACE_DISTRHO

#endif