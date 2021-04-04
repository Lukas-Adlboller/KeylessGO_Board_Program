#include "HR2046.h"
#include "GUI\Label.h"
#include "GUI\Button.h"
#include "GUI\CredentialEntry.h"
#include <vector>
#include <chrono>
#include <cstdint>

#ifndef WINDOW_H
#define WINDOW_H

class Window
{
  public:
    Window(ILI9341* displayDrv, HR2046* touchDrv);
    void Load(function<void(Window* sender, ILI9341* displayDrv, HR2046* touchDrv)> onLoadFunction);
    
    ILI9341* displayDriver;
    HR2046* touchDriver;
    vector<Label> uiLabels;
    vector<Button> uiButtons;
    vector<CredentialEntry> uiEntries;
    bool loadForm;

  private:
    void Unload(void);
};

#endif