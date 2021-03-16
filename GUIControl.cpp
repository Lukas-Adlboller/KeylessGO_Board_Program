#include "GUIControl.h"

GUIControl::GUIControl(ILI9341* displayController, bool clickable, void(*funcToCall)(void))
{
  this->displayController = displayController;
  this->clickable = clickable;
  functionToCall = funcToCall;
}

bool GUIControl::Touched(Point point)
{
  bool inXRange = point.x >= position.x && point.x <= (position.x + size.x);
  bool inYRange = point.y >= position.y && point.y <= (position.y + size.y);

  if(inXRange && inYRange)
  {
    if(clickable && functionToCall != NULL)
    {
      functionToCall();
    }
    return true;
  }

  return false;
}

void GUIControl::SetText(const char* text, int size)
{
  if(this->text == NULL)
  {
    this->text = (char*)malloc(sizeof(char) * size);
  }
  else
  {
    this->text = (char*)realloc(this->text, sizeof(char) * size);
  }

  if(this->text == NULL)
  {
    return;
  }

  for(int i = 0; i < size; i++)
  {
    this->text[i] = text[i];
  }

  textLength = size;

  Draw();
}

void GUIControl::SetPosition(Point point)
{
  position = point;
  Draw();
}

void GUIControl::SetSize(Point point)
{
  size = point;
  Draw();
}

void GUIControl::SetTextSize(uint8_t textSize)
{
  this->textSize = textSize;
  Draw();
}

void GUIControl::SetColor(uint16_t foreColor, uint16_t backColor)
{
  this->foreColor = foreColor;
  this->backColor = backColor;
  Draw();
}

void GUIControl::GetText(char* buffer)
{
  if(buffer == NULL)
  {
    return;
  }

  for(int i = 0; i < textLength; i++)
  {
    buffer[i] = text[i];
  }
}

Point GUIControl::GetPosition(void)
{
  return position;
}

Point GUIControl::GetSize(void)
{
  return size;
}

void GUIControl::Draw(void)
{
  displayController->drawRectangle(position.x, position.y, size.x, size.y, backColor);
  int textY = position.y + ((size.y - (textSize * 7)) / 2); // The size of a character is 5x7 (WidthxHeight) multiplied by the textSize
  // ToDo: Replace X value with non constant variant.
  displayController->drawString(10, textY, text, textSize, textSize, foreColor, backColor);
}