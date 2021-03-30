#include "GUIElement.h"
#include <cstdint>

GUIElement::GUIElement(ILI9341* displayDriver, Point pos, Point size, Point txtMargin, uint8_t txtSize, uint16_t bColor, uint16_t fColor, string text)
{
  displayDrv = displayDriver;

  this->position = pos;
  this->size = size;
  this->textMargin = txtMargin;
  this->backColor = bColor;
  this->foreColor = fColor;
  this->strText = text;
  this->textSize = txtSize;

  // Draw();
}


void GUIElement::Draw(void)
{
  displayDrv->fillRectangle(position.x, position.y, size.x, size.y, backColor);
  displayDrv->drawString(position.x + textMargin.x, position.y + textMargin.y, strText.c_str(), strText.length(), textSize, foreColor, backColor);
}

bool GUIElement::Clicked(uint16_t x, uint16_t y)
{
  bool inXRange = x >= position.x && x <= (position.x + size.x);
  bool inYRange = y >= position.y && y <= (position.y + size.y);

  if(inXRange && inYRange)
  {
    clickAction(this);
    Draw();
    return true;
  }
  return false;
}


void GUIElement::SetWhenClicked(std::function<void(GUIElement* sender)> action)
{
  clickAction = action;
}

void GUIElement::SetColor(uint16_t bColor, uint16_t fColor)
{
  backColor = bColor;
  foreColor = fColor;
}

void GUIElement::SetPosition(uint16_t x, uint16_t y)
{
  position.x = x;
  position.y = y;
}

void GUIElement::SetSize(uint16_t h, uint16_t w)
{
  size.x = w;
  size.y = h;
}

void GUIElement::SetTextSize(uint8_t size)
{
  textSize = size;
}