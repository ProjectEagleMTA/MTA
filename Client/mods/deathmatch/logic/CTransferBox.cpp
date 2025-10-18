/*****************************************************************************
* 
*   PROJECT:      Meta Loading Screen
*   LICENSE:      BY TOFFY | DS/TG - mta_lua
*   DS SERVER:    https://discord.gg/FHDJ8KCM2W
*   DATE:         25.02.2024 - 18:07:39
* 
******************************************************************************/

/*****************************************************************************
 *
 *  PROJECT:     Multi Theft Auto
 *  LICENSE:     See LICENSE in the top level directory
 *  FILE:        mods/deathmatch/logic/CTransferBox.cpp
 *  PURPOSE:     Transfer box GUI
 *
 *  Multi Theft Auto is available from http://www.multitheftauto.com/
 *
 *****************************************************************************/




#include <StdInc.h>
#include <sstream>
#include <cstdio> // для sprintf

#define TRANSFERBOX_HEIGHT         58
#define TRANSFERBOX_ICONSIZE       20
#define TRANSFERBOX_PROGRESSHEIGHT 28
#define TRANSFERBOX_YSTART         20
#define TRANSFERBOX_SPACER         11

#define PROGRESS_BG_HEIGHT 10
#define PROGRESS_BG_WIDTH  300

CTransferBox::CTransferBox(TransferBoxType transferType) : m_GUI(g_pCore->GetGUI())
{
    switch (transferType)
    {
        case TransferBoxType::MAP_DOWNLOAD:
            m_titleProgressPrefix = _("Map download progress:");
            break;
        default:
            m_titleProgressPrefix = _("Download Progress:");
    }

    m_visible.set(TB_VISIBILITY_CLIENT_SCRIPT);
    m_visible.set(TB_VISIBILITY_SERVER_SCRIPT);

    g_pCore->GetCVars()->Get("always_show_transferbox", m_alwaysVisible);

    CVector2D screenSize = m_GUI->GetResolution();

    // Óäàëåíèå ôîíà
    t_progress_bar_bg->SetSize(CVector2D(screenSize.fX, screenSize.fY));
    t_progress_bar_bg->SetPosition(CVector2D(0, 0));

    // Генеруємо рандомний фон
    char bg_num[500];

    // Генерація числа між 0 и 1 (включно)
    int randomNumber = rand() % 2;

    sprintf(bg_num, "cgui\\images\\meta_loading_menu\\main_%d.png", randomNumber);

    t_progress_bar_bg->LoadFromFile(bg_num);
    t_progress_bar_bg->SetZOrderingEnabled(false);
    t_progress_bar_bg->SetAlwaysOnTop(false);
    t_progress_bar_bg->SetVisible(false);

    // CreateTransferWindow();
    CreateProgressBarBg();
}

void CTransferBox::CreateProgressBarBg()
{
    CVector2D screenSize = m_GUI->GetResolution();


    // Progress Bar MAIN
    pProgressBG->SetPosition( CVector2D( (screenSize.fX - PROGRESS_BG_WIDTH) / 2, ( screenSize.fY - PROGRESS_BG_HEIGHT + 110.0f ) / 2 ) );
    pProgressBG->SetSize(CVector2D(PROGRESS_BG_WIDTH, PROGRESS_BG_HEIGHT));
    pProgressBG->LoadFromFile("cgui\\images\\meta_loading_menu\\progress_components\\progress_bar_main.png");
    pProgressBG->SetVisible(false);
    pProgressBG->SetAlwaysOnTop(true);

    // MASK
    pProgressMask->SetPosition(CVector2D(1.0f, 1.0f));
    pProgressMask->LoadFromFile("cgui\\images\\meta_loading_menu\\progress_components\\progress_bar_mask.png");
    pProgressMask->SetSize(CVector2D(0, PROGRESS_BG_HEIGHT));
    pProgressMask->SetVisible(false);
    pProgressMask->SetAlwaysOnTop(true);

    // DOWNLOAD PROGRESS LABEL
    pLabelDownloadProgress = reinterpret_cast<CGUILabel*>(m_GUI->CreateLabel("Завантаження гри..."));
    pLabelDownloadProgress->SetSize(CVector2D(pLabelDownloadProgress->GetTextExtent(), 40));
    pLabelDownloadProgress->SetPosition( CVector2D( (screenSize.fX - pLabelDownloadProgress->GetTextExtent() ) / 2, (screenSize.fY + 200.0f) / 2 ) );
    pLabelDownloadProgress->SetFont("sans");
    pLabelDownloadProgress->SetVisible(false);

    // Показ процентів
    pProgressLabel = reinterpret_cast<CGUILabel*>(m_GUI->CreateLabel(""));
    pProgressLabel->SetSize(CVector2D(pProgressLabel->GetTextExtent(), 20), false);
    pProgressLabel->SetPosition(CVector2D((screenSize.fX - pProgressLabel->GetTextExtent()) / 2, (screenSize.fY - 30.0f) / 2));
    pProgressLabel->SetFont("sans");
    pProgressLabel->SetVisible(false);

}

void CTransferBox::UpdateProgressBarVisigle(bool state)
{
    pProgressBG->SetVisible(state);
}

void CTransferBox::SetProgressBarDownloadProgress(uint64_t downloadedSizeTotal)
{
    pProgressBG->SetVisible(true);
}

void CTransferBox::SetDownloadProgress(uint64_t downloadedSizeTotal)
{
    CVector2D screenSize = m_GUI->GetResolution();

    float MaskSize = (float)pProgressMask->GetSize().fX;
    // Êîäèðîâùèê
    SString current = GetDataUnit(downloadedSizeTotal);
    SString total = GetDataUnit(m_downloadTotalSize);

    // Âû÷èñëåíèå ïðîãðåññà çàãðóçêè
    float progress = static_cast<float>(downloadedSizeTotal) / m_downloadTotalSize;
    // float progressBarWidth = 750.0f;            // Øèðèíà âàøåãî ïðîãðåññ áàðà
    float progressBarWidth = PROGRESS_BG_WIDTH;
    float maskWidth = progressBarWidth * progress;

    // Çàïèñóºìî çíà÷åííÿ â Label
    SString MbProgress_text = SString(_("%s / %s"), current.c_str(), total.c_str());
    // Розширюємо маску ( прогрес прогрес бара )
    pProgressMask->SetSize(CVector2D(maskWidth, pProgressMask->GetSize().fY));

    // записуємо один раз в змінну текст, шоб не перерендирювати постійно його

    pLabelDownloadProgress->SetText(m_titleProgressPrefix.c_str());
    pLabelDownloadProgress->SetSize(CVector2D(pLabelDownloadProgress->GetTextExtent(), 20));
    pLabelDownloadProgress->SetPosition(CVector2D((screenSize.fX - pLabelDownloadProgress->GetTextExtent()) / 2, (screenSize.fY + 200.0f) / 2));

    // Вивід процентів
    char percantange[500];
    memset(percantange, 0, sizeof(percantange));
    strcat(percantange, std::to_string((downloadedSizeTotal * 100) / m_downloadTotalSize).c_str());
    strcat(percantange, "%");

    pProgressLabel->SetText(percantange);            // MbProgress_text.c_str()
    pProgressLabel->SetSize(CVector2D(pProgressLabel->GetTextExtent(), 20), false);
    pProgressLabel->SetPosition(CVector2D((screenSize.fX - pProgressLabel->GetTextExtent()) / 2, (screenSize.fY + 140.0f) / 2) );
}

void CTransferBox::CreateTransferWindow()
{
    // Find our largest piece of text, so we can size accordingly
    std::string largeTextSample = m_titleProgressPrefix + " " + SString(_("%s of %s"), "999.99 kB", "999.99 kB");
    float       fTransferBoxWidth = m_GUI->GetTextExtent(largeTextSample.c_str(), "default-bold-small");
    fTransferBoxWidth = std::max<float>(fTransferBoxWidth, m_GUI->GetTextExtent(_("Disconnect to cancel download"), "default-normal"));

    // Add some padding to our text for the size of the window
    fTransferBoxWidth += 80;

    CVector2D screenSize = m_GUI->GetResolution();

    m_window.reset(m_GUI->CreateWnd());
    m_window->SetText("");
    m_window->SetAlpha(0.0f);
    m_window->SetVisible(false);
    m_window->SetAlwaysOnTop(true);
    m_window->SetCloseButtonEnabled(false);
    m_window->SetSizingEnabled(false);
    // m_window->SetPosition(CVector2D(screenSize.fX * 0.5f - fTransferBoxWidth * 0.5f, screenSize.fY * 0.85f - TRANSFERBOX_HEIGHT * 0.5f));
    m_window->SetPosition(CVector2D(screenSize.fX * 0.6f - fTransferBoxWidth * 0.5f, screenSize.fY * 0.85f - TRANSFERBOX_HEIGHT * 0.5f));
    m_window->SetSize(CVector2D(screenSize.fX * 0.5f - 0.2f, 58));            // relative 0.35, 0.225

    m_progressBar.reset(m_GUI->CreateProgressBar(m_window.get()));
    m_progressBar->SetPosition(CVector2D(0, TRANSFERBOX_YSTART));
    // m_progressBar->SetPosition(CVector2D(screenSize.fX * 0.6f - fTransferBoxWidth * 0.5f, screenSize.fY * 0.60f - TRANSFERBOX_HEIGHT * 0.5f));
    // m_progressBar->SetSize(CVector2D(fTransferBoxWidth, TRANSFERBOX_HEIGHT - TRANSFERBOX_YSTART - TRANSFERBOX_SPACER));
    m_progressBar->SetSize(CVector2D(screenSize.fX * 0.5f - 0.2f, screenSize.fY * 0.70f));

    m_infoLabel.reset(m_GUI->CreateLabel(m_progressBar.get(), _("Disconnect to cancel download")));
    float fTempX = (m_progressBar->GetSize().fX - m_GUI->GetTextExtent(m_infoLabel->GetText().c_str()) - TRANSFERBOX_ICONSIZE - 4) * 0.5f;
    m_infoLabel->SetPosition(CVector2D(fTempX + TRANSFERBOX_ICONSIZE + 4, 0));
    m_infoLabel->SetSize(CVector2D(fTransferBoxWidth, TRANSFERBOX_PROGRESSHEIGHT));
    m_infoLabel->SetTextColor(0, 0, 0);
    m_infoLabel->SetVerticalAlign(CGUI_ALIGN_VERTICALCENTER);

}

void CTransferBox::Show()
{
    m_visible.set(TB_VISIBILITY_MTA);
    UpdateWindowVisibility();
}

void CTransferBox::Hide()
{
    m_visible.reset(TB_VISIBILITY_MTA);
    UpdateWindowVisibility();

    m_downloadTotalSize = 0;
}

void CTransferBox::DoPulse()
{
    if (m_iconTimer.Get() >= 50)
    {
        m_iconTimer.Reset();
        // m_iconImages[m_iconIndex]->SetVisible(false);
        // m_iconIndex = (m_iconIndex + 1) % m_iconImages.size();
        // m_iconImages[m_iconIndex]->SetVisible(true);
    }
}

bool CTransferBox::SetClientVisibility(bool visible)
{
    if (m_visible[TB_VISIBILITY_CLIENT_SCRIPT] == visible)
        return false;

    m_visible.set(TB_VISIBILITY_CLIENT_SCRIPT, visible);
    UpdateWindowVisibility();
    return true;
}

bool CTransferBox::SetServerVisibility(bool visible)
{
    if (m_visible[TB_VISIBILITY_SERVER_SCRIPT] == visible)
        return false;

    m_visible.set(TB_VISIBILITY_SERVER_SCRIPT, visible);
    UpdateWindowVisibility();
    return true;
}

bool CTransferBox::SetAlwaysVisible(bool visible)
{
    if (m_alwaysVisible == visible)
        return false;

    m_alwaysVisible = visible;
    UpdateWindowVisibility();
    return true;
}

void CTransferBox::UpdateWindowVisibility() const
{
    bool visible = m_visible.all() || (m_visible[TB_VISIBILITY_MTA] && m_alwaysVisible);
    // m_window->SetVisible(visible);
    pProgressBG->SetVisible(visible);
    pProgressMask->SetVisible(visible);
    pLabelDownloadProgress->SetVisible(visible);
    pProgressLabel->SetVisible(visible);
    t_progress_bar_bg->SetVisible(visible);
}
