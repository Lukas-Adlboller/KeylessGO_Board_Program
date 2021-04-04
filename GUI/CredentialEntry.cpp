#include "CredentialEntry.h"

CredentialEntry::CredentialEntry(ILI9341* displayDriver, Point pos, uint16_t bColor, uint16_t fColor)
  : GUIElement(displayDriver, pos, Point(285, 37), Point(0, 0), 0, bColor, fColor, "")
{
  infoLabel = new Label(displayDriver, pos, Point(285, 37), Point(10, 10), 2, BLACK, WHITE, strText);
  sendToPCButton = new Button(displayDriver, Point(pos.x + 252, pos.y + 8), Point(22, 22), Point(6, 4), 2, DARK_GRAY, WHITE, " ");
  sendToPCButton->strText[0] = PLAY_ICON;
}

void CredentialEntry::Draw(void)
{
  infoLabel->Draw();
  sendToPCButton->Draw();
}

bool CredentialEntry::Clicked(uint16_t x, uint16_t y)
{
  if(sendToPCButton->Clicked(x, y))
  {
    clickAction(this);
    return true;
  }
  else
  {
    return false;
  }
}

void CredentialEntry::SetCredentialInfo(uint16_t id, string title)
{
  infoLabel->strText = title;
  credentialId = id;
}

uint16_t CredentialEntry::getEntryId(void)
{
  return credentialId;
}