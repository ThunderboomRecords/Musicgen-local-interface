#ifndef NUMBER_INPUT_HPP
#define NUMBER_INPUT_HPP

#include "WAIVEWidget.hpp"
#include <iostream>
#include <sstream>

START_NAMESPACE_DISTRHO

class NumberInput : public WAIVEWidget
{
public:
    class Callback
    {
    public:
        virtual ~Callback(){};
        virtual void textEntered(NumberInput *NumberInput, std::string text) = 0;
        virtual void textInputChanged(NumberInput *NumberInput, std::string text) = 0;
    };

    explicit NumberInput(Widget *widget) noexcept;

    void setCallback(Callback *cb);
    void setText(const char *text, bool sendCallback = false);
    void setPlaceholder(const char *text, bool sendCallback = false);
    void finalizeText();
    std::string getText();
    void undo();

    std::string placeholder;
    Align align;

    bool hasKeyFocus, hover;
    float min = 1;
    float precision = 2;
    float max = 30;

protected:
    void onNanoDisplay() override;
    bool onMouse(const MouseEvent &) override;
    bool onMotion(const MotionEvent &) override;
    bool onCharacterInput(const CharacterInputEvent &) override;
    bool onKeyboard(const KeyboardEvent &) override;

private:
    Callback *callback;

    std::string textValue, lastTextValue;
    int position;
};

END_NAMESPACE_DISTRHO

#endif