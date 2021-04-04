#include "GUIElement.h"
#include "Label.h"
#include <vector>
#include <cstdint>

#ifndef BUTTON_H
#define BUTTON_H

class Button : public GUIElement
{
  public:
    Button(ILI9341* displayDriver, Point pos, Point size, Point txtMargin, uint8_t txtSize, uint16_t bColor, uint16_t fColor, string text);
    void attachElement(vector<GUIElement>::iterator elementIterator);

    vector<GUIElement>::iterator elementIterator;
};

#endif