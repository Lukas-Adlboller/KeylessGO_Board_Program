#include "ILI9341.h"
#include <cstdint>
#include <functional>
#include <string>

#define ARROW_LEFT  0x1A
#define ARROW_UP    0x1E
#define ARROW_DOWN  0x1F
#define PLAY_ICON   0x10
#define MENU_ICON   0xF0

#ifndef GUI_ELEMENT_H
#define GUI_ELEMENT_H

class Point
{
  public:
    int16_t x, y;

	  Point(void) : x(0), y(0) {}
	  Point(int16_t x, int16_t y) : x(x), y(y) {}

	  bool operator==(Point p)
    {
      return ((p.x == x) && (p.y == y));
    }

	  bool operator!=(Point p)
    {
      return ((p.x != x) || (p.y != y));
    }
};

class GUIElement
{
  public:
    GUIElement(ILI9341* displayDriver, Point pos, Point size, Point txtMargin, uint8_t txtSize, uint16_t bColor, uint16_t fColor, string text);
    virtual void Draw(void);
    virtual bool Clicked(uint16_t x, uint16_t y);
    void SetWhenClicked(std::function<void(GUIElement* sender)> action);
    void SetColor(uint16_t bColor, uint16_t fColor);
    void SetPosition(uint16_t x, uint16_t y);
    void SetSize(uint16_t h, uint16_t w);
    void SetTextSize(uint8_t size);

    string strText;
    ILI9341* displayDrv;

  protected:
    Point position;
    Point size;
    Point textMargin;
    uint8_t textSize;
    uint16_t backColor, foreColor;
    std::function<void(GUIElement* sender)> clickAction;
};

#endif