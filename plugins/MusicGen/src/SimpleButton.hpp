#ifndef SIMPLEBUTTON_HPP_INCLUDED
#define SIMPLEBUTTON_HPP_INCLUDED

#include "WAIVEWidget.hpp"

#include <iostream>
#include <string>

START_NAMESPACE_DGL

class Button : public WAIVEWidget
{
public:
    class Callback
    {
    public:
        virtual ~Callback() {};
        virtual void buttonClicked(Button *button) = 0;
        virtual void contextClicked(Button *button) = 0;
    };
    explicit Button(Widget *parent);

    void setCallback(Callback *cb);

    void setLabel(const std::string &label);
    std::string getLabel();
    void setEnabled(bool enabled);
    void setToggled(bool value, bool sendCallback = false);
    bool getToggled() const;
    void resizeToFit();

    bool drawBackground;
    bool isToggle;
    bool sqrt = false;
    bool hasContextFN = false;
    float radius = 0;
    std::string filename = "";
    int lableAlignment = Align::ALIGN_CENTER | Align::ALIGN_MIDDLE;
    bool fEnabled;

protected:
    void onNanoDisplay() override;
    bool onMouse(const MouseEvent &ev) override;
    bool onMotion(const MotionEvent &ev) override;

private:
    Callback *callback;

    std::string label;

    bool fHasFocus;
    bool fToggleValue;
    bool pressed = false;
    bool ghost = false;
    float ghostX = 0;
    float ghostY = 0;

    DISTRHO_LEAK_DETECTOR(Button)
};

END_NAMESPACE_DGL

#endif