#include "Button.h"

#include <cstdint>

Button::Button(ILI9341* displayDriver, Point pos, Point size, Point txtMargin, uint8_t txtSize, uint16_t bColor, uint16_t fColor, string text)
  : GUIElement(displayDriver, pos, size, txtMargin, txtSize, bColor, fColor, text)
{
  
}

void Button::attachElement(vector<GUIElement>::iterator elementIterator)
{
  this->elementIterator = elementIterator;
}

bool Button::Clicked(uint16_t x, uint16_t y)
{
  bool inXRange = x >= position.x && x <= (position.x + size.x);
  bool inYRange = y >= position.y && y <= (position.y + size.y);

  if(inXRange && inYRange)
  {
    if(clickAction != NULL)
    {
      clickAction(this);
    }

    Draw();
    return true;
  }
  return false;
}