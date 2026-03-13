#include "ValueIndicator.hpp"

START_NAMESPACE_DISTRHO

using DGL_NAMESPACE::Color;

std::string floatToString(float value) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(2) << value;
    return out.str();
}


ValueIndicator::ValueIndicator(Widget *parent) noexcept
    : WAIVEWidget(parent),
      fValue(0.0f)
{
}

void ValueIndicator::setValue(float val)
{
    fValue = val;
    repaint();
}

void ValueIndicator::onNanoDisplay()
{
    std::string textString = floatToString(fValue);

    const float width = getWidth();
    const float height = getHeight();

    beginPath();
    fillColor(background_color);
    roundedRect(0.0f, 0.0f, width, height, 4.0f);
    fill();
    closePath();

    beginPath();
    fillColor(text_color);
    textAlign(Align::ALIGN_CENTER | Align::ALIGN_MIDDLE);
    fontSize(getFontSize());
    fontFaceId(font);
    text(width / 2, height / 2, textString.c_str(), NULL);
    closePath();
}

END_NAMESPACE_DISTRHO