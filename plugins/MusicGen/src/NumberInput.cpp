#include "NumberInput.hpp"

START_NAMESPACE_DISTRHO

using DGL_NAMESPACE::Color;

std::string floatToStringWithPrecision(float value, int precision) {
    std::ostringstream oss;
    oss.precision(precision);
    oss << std::fixed << value; // Convert float to string with specified precision
    return oss.str();
}

NumberInput::NumberInput(Widget *parent) noexcept
    : WAIVEWidget(parent),
      hasKeyFocus(false),
      hover(false),
      textValue(""),
      position(0),
      placeholder(""),
      align(Align::ALIGN_LEFT),
      callback(nullptr)
{
}

void NumberInput::setPlaceholder(const char *newText, bool sendCallback)
{
    placeholder = newText;
    setText(newText, sendCallback);
}

void NumberInput::setText(const char *newText, bool sendCallback)
{
    textValue.assign(newText);
    position = textValue.size();

    if (sendCallback && callback != nullptr)
        callback->textEntered(this, textValue);

    repaint();
}

std::string NumberInput::getText()
{
    return textValue;
}

void NumberInput::undo()
{
    // In case the text is invalid by reciever of callback,
    // can revert the text back to it's original
    textValue.assign(lastTextValue);
    if (callback != nullptr)
        callback->textInputChanged(this, textValue);
    repaint();
}

void NumberInput::onNanoDisplay()
{
    const float width = getWidth();
    const float height = getHeight();
    if(position > 0){
        textValue = floatToStringWithPrecision(std::stof(textValue), precision);
    }

    if (!hasKeyFocus) {
        if(position == 0){
            textValue = floatToStringWithPrecision(min, precision);
            position = textValue.size();
        }
    }

    beginPath();
    strokeColor(text_color);
    strokeWidth(3.0f);
    fillColor(background_color);
    rect(0, 0, width, height);
    fill();
    stroke();
    closePath();

    // beginPath();
    // strokeColor(text_color);
    // strokeWidth(3.0f);
    // moveTo(0, height);
    // lineTo(width, height);
    // stroke();
    // closePath();

    Rectangle<float> bounds;

    if (textValue.size() > 0)
    {
        beginPath();
        if (textValue != placeholder) {
            fillColor(text_color);
        } else {
            fillColor(text_color_greyed);
        }
        textAlign(ALIGN_MIDDLE | align);
        fontFaceId(font);
        fontSize(getFontSize());
        if (align == Align::ALIGN_CENTER)
            textBounds(width / 2, height / 2, textValue.c_str(), nullptr, bounds);
        else if (align == Align::ALIGN_LEFT)
            textBounds(2, height / 2, textValue.c_str(), nullptr, bounds);
        else
            textBounds(width - 2, height / 2, textValue.c_str(), nullptr, bounds);

        textAlign(ALIGN_LEFT | ALIGN_TOP);
        text(bounds.getX(), bounds.getY(), textValue.c_str(), nullptr);
        closePath();
    } else {
        if (align == Align::ALIGN_CENTER)
            bounds.setX(width / 2);
        else if (align == Align::ALIGN_LEFT)
            bounds.setX(2);
        else
            bounds.setX(width - 2);
    }

    if (!hasKeyFocus) {
        return;
    }
    float tv = textValue.size();
    float ps = position;
    if (tv == 0) {
        textValue.assign("1");
        position = textValue.size();
    }
 
    // draw cursor
    beginPath();
    textAlign(ALIGN_MIDDLE | ALIGN_LEFT);
    fontFaceId(font);
    float x;
    if (textValue.size() > 0 && position > 0)
    {
        textBounds(bounds.getX(), bounds.getY(), textValue.substr(0, position).c_str(), nullptr, bounds);
        x = bounds.getX() + bounds.getWidth();
    }
    else
        x = bounds.getX();
    strokeColor(text_color);
    moveTo(x, ((height - bounds.getHeight()) / 2));
    lineTo(x, height - ((height - bounds.getHeight()) / 2));
    strokeWidth(1.0f);
    stroke();
    closePath();

    if (tv == 0) {
        textValue.assign("");
        position = ps;
    }
}

bool NumberInput::onCharacterInput(const CharacterInputEvent &ev)
{
    // std::cout << "TextInput::onCharacterInput: ev.keycode = " << ev.keycode << std::endl;
    if (!hasKeyFocus || !isVisible())
        return false;

    if (std::isdigit(ev.string[0]) || ev.keycode == 47) { //  || ev.keycode == 43 for comma

    } else {
        return false;
    }

    switch (ev.keycode)
    {
    case 36:
        break;
    case 65:
        // space
        textValue.insert(textValue.begin() + position, ev.string[0]);
        position += 1;
        break;
    case 22:
        // backspace
    case 23:
        // tab
    case 119:
        // DEL
        break;
    default:
        // other characters
        textValue.insert(textValue.begin() + position, ev.string[0]);
        position += 1;
        break;
    }

    if (std::stof(textValue) < min) {
        textValue = floatToStringWithPrecision(min, precision);
        position = textValue.size();
    }

    if (std::stof(textValue) > max) {
        textValue = floatToStringWithPrecision(max, precision);
        position = textValue.size();
    }

    textValue = floatToStringWithPrecision(std::stof(textValue), precision);
    position = textValue.size();

    if (callback != nullptr)
        callback->textInputChanged(this, textValue);

    repaint();
    return true;
}

bool NumberInput::onKeyboard(const KeyboardEvent &ev)
{
    if (!hasKeyFocus || !ev.press)
        return false;

    float p = 0;
    if(precision == 0){
        p = 1;
    } else {
        p = precision * 10;
    }

    // std::cout << ev.key <<std::endl;

    switch (ev.key)
    {
    case kKeyUp:
        if(std::stof(textValue) + (1 / p) > max || std::stof(textValue) + (1 / p) < min) {
            break;
        }
        textValue.assign(floatToStringWithPrecision(std::stof(textValue) + (1 / p), precision));
        break;
    case kKeyDown:
        if(std::stof(textValue) - (1 / p) > max || std::stof(textValue) - (1 / p) < min) {
            break;
        }
        textValue.assign(floatToStringWithPrecision(std::stof(textValue) - (1 / p), precision));
        break;
    case kKeyLeft:
        if (position > 0)
            position -= 1;
        break;
    case kKeyRight:
        if (position < textValue.size())
            position += 1;
        break;
    case kKeyHome:
        position = 0;
        break;
    case kKeyEnd:
        position = textValue.size();
        break;
    case kKeyEscape:
        hasKeyFocus = false;
        textValue.assign(lastTextValue);
        if (callback != nullptr)
            callback->textInputChanged(this, textValue);
        break;
    case kKeyBackspace:
        if (position > 0)
        {
            textValue.erase(textValue.begin() + position - 1, textValue.begin() + position);
            position = position - 1;
        }
        break;
    case kKeyDelete:
        if (position < textValue.size())
            textValue.erase(textValue.begin() + position, textValue.begin() + position + 1);
        break;
    case kKeyEnter:
        if (callback != nullptr && textValue.compare(lastTextValue) != 0)
            callback->textEntered(this, textValue);
        hasKeyFocus = false;
        break;
    default:
        return false;
        break;
    }

    if (callback != nullptr)
        callback->textInputChanged(this, textValue);

    repaint();
    return true;
}

bool NumberInput::onMouse(const MouseEvent &ev)
{
    if (!isVisible())
        return false;

    if (ev.press)
    {
        bool inWidget = contains(ev.pos);
        if (inWidget && !hasKeyFocus)
        {
            hasKeyFocus = true;
            if(textValue == placeholder){ // When in focus remove placeholder
                textValue = "";
            }
            lastTextValue.assign(textValue);
            position = textValue.size();
            repaint();
            return true;
        }
        else if (!inWidget && hasKeyFocus)
        {
            if (callback != nullptr && textValue.size() > 0 && textValue.compare(lastTextValue) != 0){
                callback->textEntered(this, textValue);
            }
            else if (textValue.size() == 0) { 
                textValue.assign(placeholder);
            }
            hasKeyFocus = false;
            repaint();
            return false;
        }
    }

    return false;
}

bool NumberInput::onMotion(const MotionEvent &ev)
{
    if (!isVisible())
        return false;

    bool over = contains(ev.pos);
    if (!hover && over)
    {
        getWindow().setCursor(kMouseCursorCaret);
        hover = true;
        return true;
    }
    else if (hover && !over)
    {
        getWindow().setCursor(kMouseCursorArrow);
        hover = false;
        return false;
    }

    return false;
}

void NumberInput::setCallback(Callback *cb)
{
    callback = cb;
}

END_NAMESPACE_DISTRHO