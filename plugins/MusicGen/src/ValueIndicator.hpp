#ifndef VALUE_INDICATOR_HPP_INCLUDED
#define VALUE_INDICATOR_HPP_INCLUDED
#include <iostream>
#include <iomanip>
#include <sstream>

#include "WAIVEWidget.hpp"

START_NAMESPACE_DISTRHO

class ValueIndicator : public WAIVEWidget
{
public:
    explicit ValueIndicator(Widget *widget) noexcept;

    void setValue(float val);

protected:
    void onNanoDisplay() override;

private:
    float fValue;

    DISTRHO_LEAK_DETECTOR(ValueIndicator);
};

END_NAMESPACE_DISTRHO

#endif