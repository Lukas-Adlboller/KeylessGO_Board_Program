#include "Window.h"
#include <chrono>
#include <cstdint>

Window::Window(ILI9341* displayDrv, HR2046* touchDrv)
{
  this->displayDriver = displayDrv;
  this->touchDriver = touchDrv;
  loadForm = false;
}

void Window::Load(function<void(Window* sender, ILI9341* displayDrv, HR2046* touchDrv)> onLoadFunction)
{
  onLoadFunction(this, displayDriver, touchDriver);

  for(Label label : uiLabels)
  {
    label.Draw();
  }

  for(Button button : uiButtons)
  {
    button.Draw();
  }

  for(CredentialEntry entry : uiEntries)
  {
    entry.Draw();
  }

  loadForm = true;
  
  
  while(loadForm)
  {
    uint16_t x, y, z;
    
    if(touchDriver->wasTouched())
    {
      touchDriver->readData(&x, &y, &z);

      for(Button button : uiButtons)
      {
        button.Clicked(x, y);
      }

      for(CredentialEntry entry : uiEntries)
      {
        entry.Clicked(x, y);
      }

      ThisThread::sleep_for(chrono::milliseconds(200));
    }
  }

  Unload();
}

void Window::Unload(void)
{
  uiLabels.clear();
  uiButtons.clear();
  uiEntries.clear();
}