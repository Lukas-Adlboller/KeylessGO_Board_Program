#include "mbed.h"
#include "CryptoEngine.h"
#include "MT25Q.h"
#include "ILI9341.h"
#include "HR2046.h"
#include "EntryManager.h"
#include "KeylessComm_STM32F746.h"
#include "GUI\Window.h"
#include <cstdint>
#include <cstdio>

class BoardProgram
{
  public:
    BoardProgram(void);
    uint8_t initialize(void);
    uint8_t run(void);
    void reset(void);

    static Mutex threadMutex;
    static vector<Window> programWindows;
    static uint16_t scrollIndex;
    static uint8_t masterPassword[6];
    static bool pwdSet;
    static bool loginSuccessful;

  private:
    MT25Q flashMemory;
    ILI9341 displayDriver;
    HR2046 touchDriver;
    CryptoEngine* cryptoEngine;
    KeylessCom* serialCommunication;
    EntryManager* entryManager;

    uint8_t runFirstStartupRoutine(void);
    void updateEntryCount(uint16_t entryCount);

    // ------------ Window Event Functions ------------
    static vector<CredentialEntry> getEntriesToDraw(ILI9341* displayDrv)
    {
      uint16_t posY = 50;
      vector<CredentialEntry> credentialEntries;
      uint16_t entryEndIndex = (scrollIndex + 5) > EntryManager::credentialInfo.size() ? EntryManager::credentialInfo.size() : scrollIndex + 5;
      for(uint16_t i = scrollIndex; i < entryEndIndex; i++)
      {
        CredentialEntry tmp = CredentialEntry(displayDrv, Point(5, posY), BLACK, WHITE, "");
        tmp.SetCredentialInfo(get<0>(EntryManager::credentialInfo.at(i)), get<1>(EntryManager::credentialInfo.at(i)));
        credentialEntries.push_back(tmp);
        posY += 37;
        printf("Entry start: %d end: %d current: %d\n", scrollIndex, entryEndIndex, i);
      }

      return credentialEntries;
    }

    static void pinButtonPlus_onClick(GUIElement* sender)
    {
      ((Button*)sender)->elementIterator->strText[0]++;
      ((Button*)sender)->elementIterator->Draw();
    }

    static void pinButtonMinus_onClick(GUIElement* sender)
    {
      ((Button*)sender)->elementIterator->strText[0]--;
      ((Button*)sender)->elementIterator->Draw();
    }

    static void loginButton_onClick(GUIElement* sender)
    {
      vector<GUIElement>::iterator iter = ((Button*)sender)->elementIterator;
      printf(
        "[Info] Pin is: %c %c %c %c %c %c\n", 
        (iter + 2)->strText[0], 
        (iter + 3)->strText[0], 
        (iter + 4)->strText[0], 
        (iter + 5)->strText[0], 
        (iter + 6)->strText[0], 
        (iter + 7)->strText[0]);

      masterPassword[0] = (iter + 2)->strText[0];
      masterPassword[1] = (iter + 3)->strText[0];
      masterPassword[2] = (iter + 4)->strText[0];
      masterPassword[3] = (iter + 5)->strText[0];
      masterPassword[4] = (iter + 6)->strText[0];
      masterPassword[5] = (iter + 7)->strText[0];

      programWindows[0].loadForm = false;
    }

    static void createLoginButton_onClick(GUIElement* sender)
    {
      vector<GUIElement>::iterator iter = ((Button*)sender)->elementIterator;
      printf(
        "[Info] New Pin is: %c %c %c %c %c %c\n", 
        (iter + 2)->strText[0], 
        (iter + 3)->strText[0], 
        (iter + 4)->strText[0], 
        (iter + 5)->strText[0], 
        (iter + 6)->strText[0], 
        (iter + 7)->strText[0]);

      masterPassword[0] = (iter + 2)->strText[0];
      masterPassword[1] = (iter + 3)->strText[0];
      masterPassword[2] = (iter + 4)->strText[0];
      masterPassword[3] = (iter + 5)->strText[0];
      masterPassword[4] = (iter + 6)->strText[0];
      masterPassword[5] = (iter + 7)->strText[0];

      programWindows[1].loadForm = false;
    }

    static void repeatLoginButton_onClick(GUIElement* sender)
    {
      vector<GUIElement>::iterator iter = ((Button*)sender)->elementIterator;
      printf(
        "[Info] Repeated Pin is: %c %c %c %c %c %c\n",
        (iter + 2)->strText[0], 
        (iter + 3)->strText[0], 
        (iter + 4)->strText[0], 
        (iter + 5)->strText[0], 
        (iter + 6)->strText[0], 
        (iter + 7)->strText[0]);

      uint8_t pwd[6];
      pwd[0] = (iter + 2)->strText[0];
      pwd[1] = (iter + 3)->strText[0];
      pwd[2] = (iter + 4)->strText[0];
      pwd[3] = (iter + 5)->strText[0];
      pwd[4] = (iter + 6)->strText[0];
      pwd[5] = (iter + 7)->strText[0];

      bool samePwd = true;
      for(int i = 0; i < 6; i++)
      {
        if(pwd[i] != masterPassword[i])
        {
          samePwd = false;
          break;
        }
      }

      programWindows[2].loadForm = false;
      pwdSet = samePwd;
    }

    static void loginForm_onLoad(Window* sender, ILI9341* displayDrv, HR2046* touchDrv)
    {
      displayDrv->fillBackground(LIGHT_GRAY); // Fill Background

      Label titleLabel = Label(displayDrv, Point(5, 5), Point(310, 40), Point(10, 13), 2, BLACK, WHITE, "KEYLESS GO");
      Label infoLabel = Label(displayDrv, Point(5, 50), Point(310, 30), Point(10, 8), 2, DARK_GRAY, CYAN, "Enter Password");

      Label pinLabelA = Label(displayDrv, Point(20, 122), Point(30, 35), Point(8, 8), 3, BLACK, WHITE, "0");
      Label pinLabelB = Label(displayDrv, Point(70, 122), Point(30, 35), Point(8, 8), 3, BLACK, WHITE, "0");
      Label pinLabelC = Label(displayDrv, Point(120, 122), Point(30, 35), Point(8, 8), 3, BLACK, WHITE, "0");
      Label pinLabelD = Label(displayDrv, Point(170, 122), Point(30, 35), Point(8, 8), 3, BLACK, WHITE, "0");
      Label pinLabelE = Label(displayDrv, Point(220, 122), Point(30, 35), Point(8, 8), 3, BLACK, WHITE, "0");
      Label pinLabelF = Label(displayDrv, Point(270, 122), Point(30, 35), Point(8, 8), 3, BLACK, WHITE, "0");

      sender->uiLabels.push_back(titleLabel);
      sender->uiLabels.push_back(infoLabel);
      sender->uiLabels.push_back(pinLabelA);
      sender->uiLabels.push_back(pinLabelB);
      sender->uiLabels.push_back(pinLabelC);
      sender->uiLabels.push_back(pinLabelD);
      sender->uiLabels.push_back(pinLabelE);
      sender->uiLabels.push_back(pinLabelF);

      Button pinButtonAPlus = Button(displayDrv, Point(20, 100), Point(30, 20), Point(5, 3), 2, BLACK, WHITE, "/\\");
      pinButtonAPlus.attachElement(sender->uiLabels.begin() + 2);
      pinButtonAPlus.SetWhenClicked(pinButtonPlus_onClick);

      Button pinButtonAMinus = Button(displayDrv, Point(20, 160), Point(30, 20), Point(5, 3), 2, BLACK, WHITE, "\\/");
      pinButtonAMinus.attachElement(sender->uiLabels.begin() + 2);
      pinButtonAMinus.SetWhenClicked(pinButtonMinus_onClick);

      Button pinButtonBPlus = Button(displayDrv, Point(70, 100), Point(30, 20), Point(5, 3), 2, BLACK, WHITE, "/\\");
      pinButtonBPlus.attachElement(sender->uiLabels.begin() + 3);
      pinButtonBPlus.SetWhenClicked(pinButtonPlus_onClick);

      Button pinButtonBMinus = Button(displayDrv, Point(70, 160), Point(30, 20), Point(5, 3), 2, BLACK, WHITE, "\\/");
      pinButtonBMinus.attachElement(sender->uiLabels.begin() + 3);
      pinButtonBMinus.SetWhenClicked(pinButtonMinus_onClick);

      Button pinButtonCPlus = Button(displayDrv, Point(120, 100), Point(30, 20), Point(5, 3), 2, BLACK, WHITE, "/\\");
      pinButtonCPlus.attachElement(sender->uiLabels.begin() + 4);
      pinButtonCPlus.SetWhenClicked(pinButtonPlus_onClick);

      Button pinButtonCMinus = Button(displayDrv, Point(120, 160), Point(30, 20), Point(5, 3), 2, BLACK, WHITE, "\\/");
      pinButtonCMinus.attachElement(sender->uiLabels.begin() + 4);
      pinButtonCMinus.SetWhenClicked(pinButtonMinus_onClick);

      Button pinButtonDPlus = Button(displayDrv, Point(170, 100), Point(30, 20), Point(5, 3), 2, BLACK, WHITE, "/\\");
      pinButtonDPlus.attachElement(sender->uiLabels.begin() + 5);
      pinButtonDPlus.SetWhenClicked(pinButtonPlus_onClick);

      Button pinButtonDMinus = Button(displayDrv, Point(170, 160), Point(30, 20), Point(5, 3), 2, BLACK, WHITE, "\\/");
      pinButtonDMinus.attachElement(sender->uiLabels.begin() + 5);
      pinButtonDMinus.SetWhenClicked(pinButtonMinus_onClick);

      Button pinButtonEPlus = Button(displayDrv, Point(220, 100), Point(30, 20), Point(5, 3), 2, BLACK, WHITE, "/\\");
      pinButtonEPlus.attachElement(sender->uiLabels.begin() + 6);
      pinButtonEPlus.SetWhenClicked(pinButtonPlus_onClick);

      Button pinButtonEMinus = Button(displayDrv, Point(220, 160), Point(30, 20), Point(5, 3), 2, BLACK, WHITE, "\\/");
      pinButtonEMinus.attachElement(sender->uiLabels.begin() + 6);
      pinButtonEMinus.SetWhenClicked(pinButtonMinus_onClick);

      Button pinButtonFPlus = Button(displayDrv, Point(270, 100), Point(30, 20), Point(5, 3), 2, BLACK, WHITE, "/\\");
      pinButtonFPlus.attachElement(sender->uiLabels.begin() + 7);
      pinButtonFPlus.SetWhenClicked(pinButtonPlus_onClick);

      Button pinButtonFMinus = Button(displayDrv, Point(270, 160), Point(30, 20), Point(5, 3), 2, BLACK, WHITE, "\\/");
      pinButtonFMinus.attachElement(sender->uiLabels.begin() + 7);
      pinButtonFMinus.SetWhenClicked(pinButtonMinus_onClick);

      Button pinButtonLogin = Button(displayDrv, Point(190, 190), Point(120, 30), Point(10, 8), 2, BLACK, WHITE, "Login [ ]");
      pinButtonLogin.strText[7] = ARROW_LEFT;
      pinButtonLogin.attachElement(sender->uiLabels.begin());
      pinButtonLogin.SetWhenClicked(loginButton_onClick);

      sender->uiButtons.push_back(pinButtonAPlus);
      sender->uiButtons.push_back(pinButtonAMinus);
      sender->uiButtons.push_back(pinButtonBPlus);
      sender->uiButtons.push_back(pinButtonBMinus);
      sender->uiButtons.push_back(pinButtonCPlus);
      sender->uiButtons.push_back(pinButtonCMinus);
      sender->uiButtons.push_back(pinButtonDPlus);
      sender->uiButtons.push_back(pinButtonDMinus);
      sender->uiButtons.push_back(pinButtonEPlus);
      sender->uiButtons.push_back(pinButtonEMinus);
      sender->uiButtons.push_back(pinButtonFPlus);
      sender->uiButtons.push_back(pinButtonFMinus);
      sender->uiButtons.push_back(pinButtonLogin);
    }

    static void createLoginForm_onLoad(Window* sender, ILI9341* displayDrv, HR2046* touchDrv)
    {
      loginForm_onLoad(sender, displayDrv, touchDrv);
      sender->uiLabels[1].strText = "Create New Password";
      sender->uiButtons[12].strText = "Next [ ]";
      sender->uiButtons[12].strText[6] = ARROW_LEFT;
      sender->uiButtons[12].SetWhenClicked(createLoginButton_onClick);
    }

    static void repeatLoginForm_onLoad(Window* sender, ILI9341* displayDrv, HR2046* touchDrv)
    {
      loginForm_onLoad(sender, displayDrv, touchDrv);
      sender->uiLabels[1].strText = "Repeat New Password";
      sender->uiButtons[12].strText = "Start [ ]";
      sender->uiButtons[12].strText[7] = ARROW_LEFT;
      sender->uiButtons[12].SetWhenClicked(repeatLoginButton_onClick);
    }

    static void scrollUpButton_onClick(GUIElement* sender)
    {
      scrollIndex = scrollIndex == 0 ? 0 : scrollIndex - 1;
      vector<CredentialEntry> entries = getEntriesToDraw(sender->displayDrv);

      programWindows[3].uiEntries.clear();

      sender->displayDrv->fillRectangle(5, 50, 285, 185, LIGHT_GRAY);

      for(auto i = 0; i < entries.size(); i++)
      {
        programWindows[3].uiEntries.push_back(entries[i]);
        programWindows[3].uiEntries[i].Draw();
      }
    }

    static void scrollDownButton_onClick(GUIElement* sender)
    {
      scrollIndex = scrollIndex == EntryManager::credentialInfo.size() - 1 ? 0 : scrollIndex + 1;
      vector<CredentialEntry> entries = getEntriesToDraw(sender->displayDrv);

      programWindows[3].uiEntries.clear();

      sender->displayDrv->fillRectangle(5, 50, 285, 185, LIGHT_GRAY);

      for(auto i = 0; i < entries.size(); i++)
      {
        programWindows[3].uiEntries.push_back(entries[i]);
        programWindows[3].uiEntries[i].Draw();
      }
    }

    static void refreshButton_onClick(GUIElement* sender)
    {
      scrollIndex = 0;
      vector<CredentialEntry> entries = getEntriesToDraw(sender->displayDrv);

      programWindows[3].uiEntries.clear();

      sender->displayDrv->fillRectangle(5, 50, 285, 185, LIGHT_GRAY);

      for(auto i = 0; i < entries.size(); i++)
      {
        programWindows[3].uiEntries.push_back(entries[i]);
        programWindows[3].uiEntries[i].Draw();
      }
    }

    static void mainWindow_onLoad(Window* sender, ILI9341* displayDrv, HR2046* touchDrv)
    {
      displayDrv->fillBackground(LIGHT_GRAY); // Fill Background

      Label titleLabel = Label(displayDrv, Point(5, 5), Point(310, 40), Point(10, 13), 2, BLACK, WHITE, "KEYLESS GO");
      Label barLabel = Label(displayDrv, Point(290, 70), Point(25, 145), Point(10, 13), 2, GRAY, GRAY, "");

      sender->uiLabels.push_back(titleLabel);
      sender->uiLabels.push_back(barLabel);

      Button menuButton = Button(displayDrv, Point(225, 10), Point(30, 30), Point(10, 8), 2, DARK_GRAY, WHITE, " ");
      menuButton.strText[0] = MENU_ICON;
      menuButton.SetWhenClicked(refreshButton_onClick);

      Button logOffButton = Button(displayDrv, Point(260, 10), Point(50, 30), Point(10, 8), 2, DARK_GRAY, WHITE, "[ ]");
      logOffButton.strText[1] = ARROW_LEFT;

      Button scrollUpButton = Button(displayDrv, Point(290, 50), Point(25, 20), Point(7, 4), 2, DARK_GRAY, WHITE, " ");
      scrollUpButton.strText[0] = ARROW_UP;
      scrollUpButton.SetWhenClicked(scrollUpButton_onClick);

      Button scrollDownButton = Button(displayDrv, Point(290, 215), Point(25, 20), Point(7, 4), 2, DARK_GRAY, WHITE, " ");
      scrollDownButton.strText[0] = ARROW_DOWN;
      scrollDownButton.SetWhenClicked(scrollDownButton_onClick);

      sender->uiButtons.push_back(menuButton);
      sender->uiButtons.push_back(logOffButton);
      sender->uiButtons.push_back(scrollUpButton);
      sender->uiButtons.push_back(scrollDownButton);

      vector<CredentialEntry> entries = getEntriesToDraw(displayDrv);

      sender->uiEntries.clear();

      for(auto i = 0; i < entries.size(); i++)
      {
        programWindows[3].uiEntries.push_back(entries[i]);
      }
    }
};