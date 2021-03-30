#include "ILI9341.h"
#include "HR2046.h"
#include "GUI\Label.h"
#include "GUI\Button.h"
#include "GUI\CredentialEntry.h"
#include <vector>

#ifndef WINDOW_H
#define WINDOW_H

class Window
{
  public:
    Window(ILI9341* displayDrv, HR2046* touchDrv);
    void Load(function<void(Window* sender, ILI9341* displayDrv, HR2046* touchDrv)> onLoadFunction);
    void Unload(void);

    ILI9341* displayDriver;
    HR2046* touchDriver;
    vector<Label> uiLabels;
    vector<Button> uiButtons;
    vector<CredentialEntry> uiEntries;
    bool loadForm;
};

#endif