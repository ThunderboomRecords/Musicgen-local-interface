#include "SimpleButton.hpp"

START_NAMESPACE_DGL

Button::Button(Widget *parent)
    : WAIVEWidget(parent),
      label("button"),
      fHasFocus(false),
      callback(nullptr),
      fEnabled(true),
      drawBackground(true),
      isToggle(false)
{
    background_color = WaiveColors::grey2;
}

void Button::setLabel(const std::string &label_)
{
    label = label_;
    repaint();
}

std::string Button::getLabel()
{
    return label;
}

void Button::resizeToFit()
{
    if (label.length() == 0)
        return;

    fontSize(getFontSize());
    fontFaceId(font);

    Rectangle<float> bounds;
    textBounds(0, 0, label.c_str(), NULL, bounds);

    // setHeight(bounds.getHeight() * 2.f);
    setHeight(getFontSize() * 2.f);
    setWidth(bounds.getWidth() + getHeight());
}

void Button::setEnabled(bool enabled)
{
    fEnabled = enabled;
    repaint();
}

void Button::onNanoDisplay()
{
    const uint width = getWidth();
    const uint height = getHeight();
    const float margin = 1.0f;

    if (renderDebug)
    {
        beginPath();
        strokeColor(accent_color);
        rect(0, 0, width, height);
        stroke();
        closePath();
    }

    // Background
    if (drawBackground || isToggle)
    {
        beginPath();
        if (fToggleValue)
            fillColor(accent_color);
        else
            fillColor(fHasFocus ? highlight_color : background_color);
        if(!sqrt){
            if(radius == 0){
                roundedRect(margin, margin, width - 2 * margin, height - 2 * margin, height * 0.5f);
            } else {
                roundedRect(margin, margin, width - 2 * margin, height - 2 * margin, radius);
            }
        } else {
            rect(margin, margin, width - 2 * margin, height - 2 * margin);
        }
        fill();
        closePath();
    }

    // Label
    beginPath();
    fontSize(getFontSize());
    fontFaceId(font);
    fillColor(fToggleValue ? background_color : text_color);
    textAlign(lableAlignment);
    text(width / 2, height / 2, label.c_str(), nullptr);
    closePath();

    if (!fEnabled && drawBackground)
    {
        beginPath();
        fillColor(0.f, 0.f, 0.f, 0.5f);
        if(!sqrt){
            if(radius == 0){
                roundedRect(margin, margin, width - 2 * margin, height - 2 * margin, height * 0.5f);
            } else {
                roundedRect(margin, margin, width - 2 * margin, height - 2 * margin, radius);
            }
        } else {
            rect(margin, margin, width - 2 * margin, height - 2 * margin);
        }
        fill();
        closePath();
    }
}

bool Button::onMouse(const MouseEvent &ev)
{
    // std::cout << "Button::onMouse " << label << ": " << fEnabled << " " << (callback != nullptr) << " " << ev.press << " " << (ev.button == kMouseButtonLeft) << " " << contains(ev.pos) << std::endl;
    // std::cout << ev.pos.getX() << ev.pos.getY() << std::endl;
    // std::cout << this->getAbsoluteX() << this->getAbsoluteY() << std::endl;
    if (
        fEnabled &&
        callback != nullptr &&
        ev.press &&
        ev.button == kMouseButtonLeft &&
        contains(ev.pos)
        // ev.pos.getX() > this->getAbsoluteX() && ev.pos.getX() < this->getAbsoluteX() + this->getWidth() &&
        // ev.pos.getY() > this->getAbsoluteY() && ev.pos.getY() < this->getAbsoluteY() + this->getHeight())
    )
    {
        if (isToggle)
            fToggleValue = !fToggleValue;

        callback->buttonClicked(this);
        repaint();
        return true;
    } else if(fEnabled && callback != nullptr && ev.press && ev.button == kMouseButtonRight && hasContextFN && contains(ev.pos)) {
        callback->contextClicked(this);
        repaint();
        return true;
    }

    return false;
}

bool Button::onMotion(const MotionEvent &ev)
{
    if (contains(ev.pos))
    {
        if (!fHasFocus && fEnabled)
        {
            fHasFocus = true;
            getWindow().setCursor(kMouseCursorHand);
            repaint();
        }
        return true;
    }
    else
    {
        if (fHasFocus)
        {
            fHasFocus = false;
            getWindow().setCursor(kMouseCursorArrow);
            repaint();
        }
    }
    return false;
}

void Button::setToggled(bool value, bool sendCallback)
{
    fToggleValue = value;
    repaint();
    if (callback != nullptr && sendCallback)
        callback->buttonClicked(this);
}

bool Button::getToggled() const
{
    return fToggleValue;
}

void Button::setCallback(Callback *cb)
{
    callback = cb;
}

END_NAMESPACE_DGL