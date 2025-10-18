/*****************************************************************************
 *
 *  PROJECT:     Multi Theft Auto v1.0
 *  LICENSE:     See LICENSE in the top level directory
 *  FILE:        core/CMainMenu.h
 *  PURPOSE:     Header file for main menu graphical user interface class
 *
 *  Multi Theft Auto is available from http://www.multitheftauto.com/
 *
 *****************************************************************************/

class CMainMenu;

#pragma once

#include "CCore.h"
#include "CQuestionBox.h"
#include <ServerBrowser/CServerBrowser.h>
#include <ServerBrowser/CServerInfo.h>
#include "CSettings.h"
#include <Graphics/CGraphics.h>
#include "CMainMenu.h"
class CLanguageSelector;

#define CORE_MTA_NEWS_ITEMS         3

struct sMenuItem
{
    unsigned char    menuType;
    int              drawPositionX;
    int              drawPositionY;
    int              nativeSizeX;
    int              nativeSizeY;
    CGUIStaticImage* image;
};

class CMainMenu
{
    friend class CServerBrowser;
    friend class CSettings;

public:
    CMainMenu(CGUI* pManager);
    ~CMainMenu();

    void Update();

    void Show(bool bOverlay);
    void Hide();

    void SetVisible(bool bVisible, bool bOverlay = true, bool bFrameDelay = true);
    bool IsVisible();
    bool IsFading() { return m_ucFade == FADE_IN || m_ucFade == FADE_OUT; }

    // Кнопки
    CGUIStaticImage* m_pCustomSettings;
    CGUIStaticImage* m_pCustomQuit;
    CGUIStaticImage* m_pCustomReconnect;

    // Анимации
    bool EnterSettings(CGUIElement* pElement);
    bool LeaveSettings(CGUIElement* pElement);
    bool ClickSettings(CGUIElement* pElement);
    bool EnterQuit(CGUIElement* pElement);
    bool LeaveQuit(CGUIElement* pElement);
    bool EnterReconnect(CGUIElement* pElement);
    bool LeaveReconnect(CGUIElement* pElement);
    bool ClickQuit(CGUIElement* pElement);
    bool ClickReconnect(CGUIElement* pElement);
    void SetVisibleCustomButtons(bool bVisible);

    CGUIStaticImage* m_pLogo;

    // Залупа всякая
    CGUILabel* m_pCanvasLabel;
    bool       m_bIsIngame;
    CSettings        m_Settings;
    CVector2D        m_ScreenSize;
    CGUI*            m_pManager;
    CGUIStaticImage* m_pBackground;

    void SetIsIngame(bool bIsIngame);
    bool GetIsIngame();

    CServerBrowser* GetServerBrowser() { return &m_ServerBrowser; };
    CSettings*      GetSettingsWindow() { return &m_Settings; };
    CQuestionBox*   GetQuestionWindow() { return &m_QuestionBox; };
    
    void SetMenuVerticalPosition(int iPosY);
    void SetMenuUnhovered();

    bool HasStarted() { return m_bStarted; };
private:
    bool OnQuickConnectButtonClick(CGUIElement* pElement);
    bool OnSettingsButtonClick(CGUIElement* pElement);
    bool OnQuitButtonClick(CGUIElement* pElement);

    void HideServerInfo();

    //CGUI* m_pManager;

    // Images
    //CGUIStaticImage* m_pBackground;
    CGUIStaticImage* m_pCanvas;

    std::set<sMenuItem*>   m_unhoveredItems;
    sMenuItem*             m_pHoveredItem;
    bool                   m_bMouseOverMenu;

    // Submenu classes
    CQuestionBox       m_QuestionBox;
    //CSettings          m_Settings;
    CServerBrowser     m_ServerBrowser;
    CServerInfo        m_ServerInfo;
    CLanguageSelector* m_pLanguageSelector;

    // Properties
    bool m_bIsVisible;
    bool m_bIsFullyVisible;
    bool m_bFrameDelay;
    bool m_bIsInSubWindow;
    bool m_bHideGame;

    int m_iXOff;
    int m_iYOff;
    int m_iMenuSizeX;
    int m_iMenuSizeY;
    int m_iFirstItemCentre;
    int m_iSecondItemCentre;
    // Define our bounding box for menu items
    int m_menuAX;
    int m_menuAY;
    int m_menuBX;
    int m_menuBY;

    CGraphics* m_pGraphics;
    bool       m_bStarted;
    //CVector2D  m_ScreenSize;

    // Fade variables
    unsigned char m_ucFade;
    float         m_fFader;

    // Animation variables
    unsigned long ulPreviousTick;
    int           m_iMoveStartPos;
    int           m_iMoveTargetPos;
    unsigned long m_ulMoveStartTick;

    CGUILabel* m_pNewsItemLabels[CORE_MTA_NEWS_ITEMS];
    CGUILabel* m_pNewsItemShadowLabels[CORE_MTA_NEWS_ITEMS];
    CGUILabel* m_pNewsItemDateLabels[CORE_MTA_NEWS_ITEMS];
    CGUILabel* m_pNewsItemNEWLabels[CORE_MTA_NEWS_ITEMS];

#ifdef CI_BUILD
    std::unique_ptr<CGUITexture>     m_pFeatureBranchAlertTexture;
    std::unique_ptr<CGUIStaticImage> m_pFeatureBranchAlertImage;
    std::unique_ptr<CGUILabel>       m_pFeatureBranchAlertLabel;
#endif

#if _WIN32_WINNT <= _WIN32_WINNT_WINXP
    std::unique_ptr<CGUITexture>     m_pAlertTexture;
    std::unique_ptr<CGUIStaticImage> m_pAlertImage;
    std::unique_ptr<CGUILabel>       m_pAlertLabel;
#endif

    // Fade states
    enum eFadeStates
    {
        FADE_VISIBLE,
        FADE_INVISIBLE,
        FADE_IN,
        FADE_OUT
    };
};
