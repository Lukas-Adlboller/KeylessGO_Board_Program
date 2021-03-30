#include "Label.h"
#include <cstdint>

Label::Label(ILI9341* displayDriver, Point pos, Point size, Point txtMargin, uint8_t txtSize, uint16_t bColor, uint16_t fColor, string text)
  : GUIElement(displayDriver, pos, size, txtMargin, txtSize, bColor, fColor, text)
{
  
}

bool Label::Clicked(uint16_t x, uint16_t y)
{
  return false;
};