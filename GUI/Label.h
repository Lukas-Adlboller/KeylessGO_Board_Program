#include "GUIElement.h"
#include <cstdint>

#ifndef LABEL_H
#define LABEL_H

class Label : public GUIElement
{
  public:
    Label(ILI9341* displayDriver, Point pos, Point size, Point txtMargin, uint8_t txtSize, uint16_t bColor, uint16_t fColor, string text);
    bool Clicked(uint16_t x, uint16_t y) override;
};

#endif