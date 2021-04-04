#include "Button.h"

Button::Button(ILI9341* displayDriver, Point pos, Point size, Point txtMargin, uint8_t txtSize, uint16_t bColor, uint16_t fColor, string text)
  : GUIElement(displayDriver, pos, size, txtMargin, txtSize, bColor, fColor, text)
{
  
}

void Button::attachElement(vector<GUIElement>::iterator elementIterator)
{
  this->elementIterator = elementIterator;
}