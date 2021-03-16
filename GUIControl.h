#include "ILI9341.h"
#include "HR2046.h"
#include <cstdint>

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

class GUIControl
{
  public:
    GUIControl(ILI9341* displayController, bool clickable, void(*funcToCall)(void));
    virtual void Draw(void);
    virtual bool Touched(Point point);
    void SetText(const char* text, int size);
    void SetPosition(Point point);
    void SetColor(uint16_t foreColor, uint16_t backColor);
    void SetSize(Point point);
    void SetTextSize(uint8_t textSize);
    void GetText(char* buffer);
    Point GetPosition(void);
    Point GetSize(void);

  private:
    ILI9341* displayController;
    Point position;
    Point size;
    uint8_t textSize;
    uint16_t foreColor;
    uint16_t backColor;
    char* text;
    int textLength;
    bool clickable;
    void (*functionToCall)(void);
};