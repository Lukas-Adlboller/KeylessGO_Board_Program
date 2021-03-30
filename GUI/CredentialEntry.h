#include "GUIElement.h"
#include "Button.h"
#include "Label.h"
#include <cstdint>

#ifndef CREDENTIAL_ENTRY_H
#define CREDENTIAL_ENTRY_H

class CredentialEntry : public GUIElement
{
  public:
    CredentialEntry(ILI9341* displayDriver, Point pos, uint16_t bColor, uint16_t fColor, string text);

    void SetCredentialInfo(uint16_t id, string title);

    void Draw(void) override;
    bool Clicked(uint16_t x, uint16_t y) override;

  private:
    uint16_t credentialId;
    Label* infoLabel;
    Button* sendToPCButton; 
};

#endif