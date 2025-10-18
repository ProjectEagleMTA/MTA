/*****************************************************************************
 *
 *  PROJECT:     Multi Theft Auto
 *  LICENSE:     See LICENSE in the top level directory
 *  FILE:        core/CSettings.cpp
 *  PURPOSE:     In-game settings window
 *
 *  Multi Theft Auto is available from http://www.multitheftauto.com/
 *
 *****************************************************************************/

#include "StdInc.h"
#include <core/CClientCommands.h>
#include <game/CGame.h>
#include <game/CSettings.h>

using namespace std;

#define CORE_SETTINGS_UPDATE_INTERVAL 30            // Settings update interval in frames
#define CORE_SETTINGS_HEADERS 3
#define CORE_SETTINGS_HEADER_SPACER " "
#define CORE_SETTINGS_NO_KEY " "

extern CCore*              g_pCore;
extern SBindableGTAControl g_bcControls[];
extern SBindableKey        g_bkKeys[];

CSettings::CSettings()
{
    CGameSettings* gameSettings = CCore::GetSingleton().GetGame()->GetSettings();
    m_fRadioVolume = (float)gameSettings->GetRadioVolume() / 64.0f;
    m_fSFXVolume = (float)gameSettings->GetSFXVolume() / 64.0f;

    m_iMaxAnisotropic = g_pDeviceState->AdapterState.MaxAnisotropicSetting;
    m_pWindow = NULL;
    CreateGUI();

    // Disable progress animation if required
    if (GetApplicationSettingInt(GENERAL_PROGRESS_ANIMATION_DISABLE))
    {
        SetApplicationSettingInt(GENERAL_PROGRESS_ANIMATION_DISABLE, 0);
        CVARS_SET("progress_animation", 0);
    }
}

CSettings::~CSettings()
{
    DestroyGUI();
}

void CSettings::CreateGUI()
{
    if (m_pWindow)
        DestroyGUI();

    CGUITab *pTabVideo, *pTabAudio, *pTabBinds, *pTabControls;
    CGUI*    pManager = g_pCore->GetGUI();

    // Init
    m_bIsModLoaded = false;
    m_bCaptureKey = false;
    m_dwFrameCount = 0;
    m_bShownVolumetricShadowsWarning = false;
    m_bShownAllowScreenUploadMessage = false;
    CVector2D vecTemp;
    CVector2D vecSize;

    CVector2D resolution = CCore::GetSingleton().GetGUI()->GetResolution();

    CVector2D contentSize(640, 480);
    float     fBottomButtonAreaHeight = 38;
    CVector2D tabPanelPosition;
    CVector2D tabPanelSize = contentSize - CVector2D(0, fBottomButtonAreaHeight);

    // Window size is content size plus window frame edge dims
    CVector2D windowSize = contentSize + CVector2D(9 + 9, 20 + 2);

    if (windowSize.fX <= resolution.fX && windowSize.fY <= resolution.fY)
    {
        // Create window (with frame) if it will fit inside the screen resolution
        CGUIWindow* pWindow = reinterpret_cast<CGUIWindow*>(pManager->CreateWnd(NULL, _("SETTINGS")));
        pWindow->SetCloseButtonEnabled(true);
        pWindow->SetMovable(true);
        pWindow->SetPosition((resolution - windowSize) / 2);
        pWindow->SetSize(windowSize);
        pWindow->SetSizingEnabled(false);
        pWindow->SetAlwaysOnTop(true);
        pWindow->BringToFront();
        m_pWindow = pWindow;
        tabPanelPosition = CVector2D(0, 20);
    }

    // Create the tab panel and necessary tabs
    m_pTabs = reinterpret_cast<CGUITabPanel*>(pManager->CreateTabPanel(m_pWindow));
    m_pTabs->SetPosition(tabPanelPosition);
    m_pTabs->SetSize(tabPanelSize);
    m_pTabs->SetSelectionHandler(GUI_CALLBACK(&CSettings::OnTabChanged, this));

    pTabVideo = m_pTabs->CreateTab(_("Video"));
    pTabAudio = m_pTabs->CreateTab(_("Audio"));
    pTabBinds = m_pTabs->CreateTab(_("Binds"));
    pTabControls = m_pTabs->CreateTab(_("Controls"));

    // Create buttons
    //  OK button
    m_pButtonOK = reinterpret_cast<CGUIButton*>(pManager->CreateButton(m_pWindow, _("OK")));
    m_pButtonOK->SetPosition(CVector2D(contentSize.fX * 0.5f, tabPanelSize.fY + tabPanelPosition.fY + 8));
    m_pButtonOK->SetZOrderingEnabled(false);

    //  Cancel button
    m_pButtonCancel = reinterpret_cast<CGUIButton*>(pManager->CreateButton(m_pWindow, _("Cancel")));
    m_pButtonCancel->SetPosition(CVector2D(contentSize.fX * 0.78f, tabPanelSize.fY + tabPanelPosition.fY + 8));
    m_pButtonCancel->SetZOrderingEnabled(false);

    /**
     *  Binds tab
     **/
    m_pBindsList = reinterpret_cast<CGUIGridList*>(pManager->CreateGridList(pTabBinds, false));
    m_pBindsList->SetPosition(CVector2D(10, 15));
    m_pBindsList->SetSize(CVector2D(620, 357));
    m_pBindsList->SetSortingEnabled(false);
    m_pBindsList->SetSelectionMode(SelectionModes::CellSingle);
    m_pBindsList->SetDoubleClickHandler(GUI_CALLBACK(&CSettings::OnBindsListClick, this));

    m_pTabs->GetSize(vecTemp);
    m_pBindsDefButton = reinterpret_cast<CGUIButton*>(pManager->CreateButton(pTabBinds, _("Load defaults")));
    m_pBindsDefButton->SetClickHandler(GUI_CALLBACK(&CSettings::OnBindsDefaultClick, this));
    m_pBindsDefButton->AutoSize(NULL, 20.0f, 8.0f);
    m_pBindsDefButton->GetSize(vecSize);
    m_pBindsDefButton->SetPosition(CVector2D(vecTemp.fX - vecSize.fX - 12.0f, 387));
    m_pBindsDefButton->SetZOrderingEnabled(false);

    /**
     *  Controls tab
     **/
    float fIndentX = pManager->CGUI_GetMaxTextExtent("default-normal", _("Mouse sensitivity:"), _("Vertical aim sensitivity:"));
    vecTemp = CVector2D(0, 13);
    // Mouse Options
    m_pControlsMouseLabel = reinterpret_cast<CGUILabel*>(pManager->CreateLabel(pTabControls, _("Mouse options")));
    m_pControlsMouseLabel->SetPosition(CVector2D(vecTemp.fX + 11, vecTemp.fY));
    m_pControlsMouseLabel->AutoSize(NULL, 20.0f);
    m_pControlsMouseLabel->SetFont("default-bold-small");
    vecTemp.fY += 18;

    vecTemp.fX = 16;
    m_pInvertMouse = reinterpret_cast<CGUICheckBox*>(pManager->CreateCheckBox(pTabControls, _("Invert mouse vertically"), true));
    m_pInvertMouse->SetPosition(CVector2D(vecTemp.fX, vecTemp.fY));
    m_pInvertMouse->AutoSize(NULL, 20.0f);

    m_pSteerWithMouse = reinterpret_cast<CGUICheckBox*>(pManager->CreateCheckBox(pTabControls, _("Steer with mouse"), true));
    m_pSteerWithMouse->SetPosition(CVector2D(vecTemp.fX, vecTemp.fY + 16.0f));
    m_pSteerWithMouse->AutoSize(NULL, 20.0f);

    m_pFlyWithMouse = reinterpret_cast<CGUICheckBox*>(pManager->CreateCheckBox(pTabControls, _("Fly with mouse"), true));
    m_pFlyWithMouse->SetPosition(CVector2D(vecTemp.fX, vecTemp.fY + 32.0f));
    m_pFlyWithMouse->AutoSize(NULL, 20.0f);

    // MouseSensitivity
    vecTemp.fY += 54.0f;
    m_pLabelMouseSensitivity = reinterpret_cast<CGUILabel*>(pManager->CreateLabel(pTabControls, _("Mouse sensitivity:")));
    m_pLabelMouseSensitivity->SetPosition(CVector2D(vecTemp.fX, vecTemp.fY));
    m_pLabelMouseSensitivity->AutoSize();

    m_pMouseSensitivity = reinterpret_cast<CGUIScrollBar*>(pManager->CreateScrollBar(true, pTabControls));
    m_pMouseSensitivity->SetPosition(CVector2D(vecTemp.fX + fIndentX + 5.0f, vecTemp.fY));
    m_pMouseSensitivity->GetPosition(vecTemp);
    m_pMouseSensitivity->SetSize(CVector2D(160.0f, 20.0f));
    m_pMouseSensitivity->GetSize(vecSize);
    m_pMouseSensitivity->SetProperty("StepSize", "0.01");

    m_pLabelMouseSensitivityValue = reinterpret_cast<CGUILabel*>(pManager->CreateLabel(pTabControls, "0%"));
    m_pLabelMouseSensitivityValue->SetPosition(CVector2D(vecTemp.fX + vecSize.fX + 5.0f, vecTemp.fY));
    m_pLabelMouseSensitivityValue->AutoSize("100%");
    vecTemp.fX = 16;
    vecTemp.fY += 24.f;

    // VerticalAimSensitivity
    m_pLabelVerticalAimSensitivity = reinterpret_cast<CGUILabel*>(pManager->CreateLabel(pTabControls, _("Vertical aim sensitivity:")));
    m_pLabelVerticalAimSensitivity->SetPosition(CVector2D(vecTemp.fX, vecTemp.fY));
    m_pLabelVerticalAimSensitivity->AutoSize(m_pLabelVerticalAimSensitivity->GetText().c_str());

    m_pVerticalAimSensitivity = reinterpret_cast<CGUIScrollBar*>(pManager->CreateScrollBar(true, pTabControls));
    m_pVerticalAimSensitivity->SetPosition(CVector2D(vecTemp.fX + fIndentX + 5.0f, vecTemp.fY));
    m_pVerticalAimSensitivity->GetPosition(vecTemp);
    m_pVerticalAimSensitivity->SetSize(CVector2D(160.0f, 20.0f));
    m_pVerticalAimSensitivity->GetSize(vecSize);
    m_pVerticalAimSensitivity->SetProperty("StepSize", "0.01");

    m_pLabelVerticalAimSensitivityValue = reinterpret_cast<CGUILabel*>(pManager->CreateLabel(pTabControls, "0%"));
    m_pLabelVerticalAimSensitivityValue->SetPosition(CVector2D(vecTemp.fX + vecSize.fX + 5.0f, vecTemp.fY));
    m_pLabelVerticalAimSensitivityValue->AutoSize("100%");
    vecTemp.fY += 30.f;

    // Create a mini-scrollpane for the radio buttons (only way to group them together)
    m_pControlsInputTypePane = reinterpret_cast<CGUIScrollPane*>(pManager->CreateScrollPane(pTabControls));
    m_pControlsInputTypePane->SetProperty("ContentPaneAutoSized", "False");
    m_pControlsInputTypePane->SetPosition(CVector2D(0, vecTemp.fY));
    m_pControlsInputTypePane->SetSize(CVector2D(1.0f, 0.27f), true);
    m_pControlsInputTypePane->SetZOrderingEnabled(false);

    m_pControlsInputTypePane->GetPosition(vecTemp);
    vecTemp.fY += 24;

    m_pTabs->GetSize(vecTemp);
    CGUIButton* pControlsDefButton = reinterpret_cast<CGUIButton*>(pManager->CreateButton(pTabControls, _("Load defaults")));
    pControlsDefButton->SetClickHandler(GUI_CALLBACK(&CSettings::OnControlsDefaultClick, this));
    pControlsDefButton->AutoSize(NULL, 20.0f, 8.0f);
    pControlsDefButton->GetSize(vecSize);
    pControlsDefButton->SetPosition(CVector2D(vecTemp.fX - vecSize.fX - 12.0f, 387));
    pControlsDefButton->SetZOrderingEnabled(false);

    m_hBind = m_pBindsList->AddColumn(_("DESCRIPTION"), 0.35f);
    m_hPriKey = m_pBindsList->AddColumn(_("KEY"), 0.24f);
    for (int k = 0; k < SecKeyNum; k++)
        m_hSecKeys[k] = m_pBindsList->AddColumn(_("ALT. KEY"), 0.24f);

    /**
     *  Audio tab
     **/
    fIndentX = pManager->CGUI_GetMaxTextExtent("default-normal", _("Master volume:"), _("Radio volume:"), _("SFX volume:"), _("MTA volume:"),
                                               _("Voice volume:"), _("Play mode:"));

    m_pAudioGeneralLabel = reinterpret_cast<CGUILabel*>(pManager->CreateLabel(pTabAudio, _("General")));
    m_pAudioGeneralLabel->SetPosition(CVector2D(11, 13));
    m_pAudioGeneralLabel->GetPosition(vecTemp, false);
    m_pAudioGeneralLabel->AutoSize(NULL, 5.0f);
    m_pAudioGeneralLabel->SetFont("default-bold-small");

    m_pLabelMasterVolume = reinterpret_cast<CGUILabel*>(pManager->CreateLabel(pTabAudio, _("Master volume:")));
    m_pLabelMasterVolume->SetPosition(CVector2D(vecTemp.fX, vecTemp.fY + 29.0f), false);
    m_pLabelMasterVolume->GetPosition(vecTemp, false);
    m_pLabelMasterVolume->AutoSize();

    m_pAudioMasterVolume = reinterpret_cast<CGUIScrollBar*>(pManager->CreateScrollBar(true, pTabAudio));
    m_pAudioMasterVolume->SetPosition(CVector2D(vecTemp.fX + fIndentX + 5.0f, vecTemp.fY));
    m_pAudioMasterVolume->GetPosition(vecTemp, false);
    m_pAudioMasterVolume->SetSize(CVector2D(160.0f, 20.0f));
    m_pAudioMasterVolume->GetSize(vecSize, false);
    m_pAudioMasterVolume->SetProperty("StepSize", "0.01");

    m_pLabelMasterVolumeValue = reinterpret_cast<CGUILabel*>(pManager->CreateLabel(pTabAudio, "0%"));
    m_pLabelMasterVolumeValue->SetPosition(CVector2D(vecTemp.fX + vecSize.fX + 5.0f, vecTemp.fY));
    m_pLabelMasterVolumeValue->GetPosition(vecTemp, false);
    m_pLabelMasterVolumeValue->AutoSize("100%");
    m_pLabelMasterVolumeValue->GetSize(vecSize, false);

    vecTemp.fX = 11;
    m_pLabelRadioVolume = reinterpret_cast<CGUILabel*>(pManager->CreateLabel(pTabAudio, _("Radio volume:")));
    m_pLabelRadioVolume->SetPosition(CVector2D(vecTemp.fX, vecTemp.fY + 29.0f), false);
    m_pLabelRadioVolume->GetPosition(vecTemp, false);
    m_pLabelRadioVolume->AutoSize();

    m_pAudioRadioVolume = reinterpret_cast<CGUIScrollBar*>(pManager->CreateScrollBar(true, pTabAudio));
    m_pAudioRadioVolume->SetPosition(CVector2D(vecTemp.fX + fIndentX + 5.0f, vecTemp.fY));
    m_pAudioRadioVolume->GetPosition(vecTemp, false);
    m_pAudioRadioVolume->SetSize(CVector2D(160.0f, 20.0f));
    m_pAudioRadioVolume->GetSize(vecSize, false);
    m_pAudioRadioVolume->SetProperty("StepSize", "0.01");

    m_pLabelRadioVolumeValue = reinterpret_cast<CGUILabel*>(pManager->CreateLabel(pTabAudio, "0%"));
    m_pLabelRadioVolumeValue->SetPosition(CVector2D(vecTemp.fX + vecSize.fX + 5.0f, vecTemp.fY));
    m_pLabelRadioVolumeValue->GetPosition(vecTemp, false);
    m_pLabelRadioVolumeValue->AutoSize("100%");
    m_pLabelRadioVolumeValue->GetSize(vecSize, false);

    vecTemp.fX = 11;
    m_pLabelSFXVolume = reinterpret_cast<CGUILabel*>(pManager->CreateLabel(pTabAudio, _("SFX volume:")));
    m_pLabelSFXVolume->SetPosition(CVector2D(vecTemp.fX, vecTemp.fY + 29.0f));
    m_pLabelSFXVolume->GetPosition(vecTemp, false);
    m_pLabelSFXVolume->AutoSize();

    m_pAudioSFXVolume = reinterpret_cast<CGUIScrollBar*>(pManager->CreateScrollBar(true, pTabAudio));
    m_pAudioSFXVolume->SetPosition(CVector2D(vecTemp.fX + fIndentX + 5.0f, vecTemp.fY));
    m_pAudioSFXVolume->GetPosition(vecTemp, false);
    m_pAudioSFXVolume->SetSize(CVector2D(160.0f, 20.0f));
    m_pAudioSFXVolume->GetSize(vecSize, false);
    m_pAudioSFXVolume->SetProperty("StepSize", "0.01");

    m_pLabelSFXVolumeValue = reinterpret_cast<CGUILabel*>(pManager->CreateLabel(pTabAudio, "0%"));
    m_pLabelSFXVolumeValue->SetPosition(CVector2D(vecTemp.fX + vecSize.fX + 5.0f, vecTemp.fY));
    m_pLabelSFXVolumeValue->GetPosition(vecTemp, false);
    m_pLabelSFXVolumeValue->AutoSize("100%");
    m_pLabelSFXVolumeValue->GetSize(vecSize, false);

    vecTemp.fX = 11;
    m_pLabelMTAVolume = reinterpret_cast<CGUILabel*>(pManager->CreateLabel(pTabAudio, _("MTA volume:")));
    m_pLabelMTAVolume->SetPosition(CVector2D(vecTemp.fX, vecTemp.fY + 29.0f));
    m_pLabelMTAVolume->GetPosition(vecTemp, false);
    m_pLabelMTAVolume->AutoSize();

    m_pAudioMTAVolume = reinterpret_cast<CGUIScrollBar*>(pManager->CreateScrollBar(true, pTabAudio));
    m_pAudioMTAVolume->SetPosition(CVector2D(vecTemp.fX + fIndentX + 5.0f, vecTemp.fY));
    m_pAudioMTAVolume->GetPosition(vecTemp, false);
    m_pAudioMTAVolume->SetSize(CVector2D(160.0f, 20.0f));
    m_pAudioMTAVolume->GetSize(vecSize, false);
    m_pAudioMTAVolume->SetProperty("StepSize", "0.01");

    m_pLabelMTAVolumeValue = reinterpret_cast<CGUILabel*>(pManager->CreateLabel(pTabAudio, "0%"));
    m_pLabelMTAVolumeValue->SetPosition(CVector2D(vecTemp.fX + vecSize.fX + 5.0f, vecTemp.fY));
    m_pLabelMTAVolumeValue->GetPosition(vecTemp, false);
    m_pLabelMTAVolumeValue->AutoSize("100%");
    m_pLabelMTAVolumeValue->GetSize(vecSize, false);

    vecTemp.fX = 11;
    m_pLabelVoiceVolume = reinterpret_cast<CGUILabel*>(pManager->CreateLabel(pTabAudio, _("Voice volume:")));
    m_pLabelVoiceVolume->SetPosition(CVector2D(vecTemp.fX, vecTemp.fY + 29.0f));
    m_pLabelVoiceVolume->GetPosition(vecTemp, false);
    m_pLabelVoiceVolume->AutoSize();

    m_pAudioVoiceVolume = reinterpret_cast<CGUIScrollBar*>(pManager->CreateScrollBar(true, pTabAudio));
    m_pAudioVoiceVolume->SetPosition(CVector2D(vecTemp.fX + fIndentX + 5.0f, vecTemp.fY));
    m_pAudioVoiceVolume->GetPosition(vecTemp, false);
    m_pAudioVoiceVolume->SetSize(CVector2D(160.0f, 20.0f));
    m_pAudioVoiceVolume->GetSize(vecSize, false);
    m_pAudioVoiceVolume->SetProperty("StepSize", "0.01");

    m_pLabelVoiceVolumeValue = reinterpret_cast<CGUILabel*>(pManager->CreateLabel(pTabAudio, "0%"));
    m_pLabelVoiceVolumeValue->SetPosition(CVector2D(vecTemp.fX + vecSize.fX + 5.0f, vecTemp.fY));
    m_pLabelVoiceVolumeValue->GetPosition(vecTemp, false);
    m_pLabelVoiceVolumeValue->AutoSize("100%");
    m_pLabelVoiceVolumeValue->GetSize(vecSize, false);

    vecTemp.fX = 11;
    m_pAudioRadioLabel = reinterpret_cast<CGUILabel*>(pManager->CreateLabel(pTabAudio, _("Radio options")));
    m_pAudioRadioLabel->SetPosition(CVector2D(vecTemp.fX, vecTemp.fY + 30.0f), false);
    m_pAudioRadioLabel->GetPosition(vecTemp, false);
    m_pAudioRadioLabel->AutoSize(NULL, 20.0f);
    m_pAudioRadioLabel->SetFont("default-bold-small");

    m_pCheckBoxAudioEqualizer = reinterpret_cast<CGUICheckBox*>(pManager->CreateCheckBox(pTabAudio, _("Radio Equalizer"), true));
    m_pCheckBoxAudioEqualizer->SetPosition(CVector2D(vecTemp.fX, vecTemp.fY + 29.0f));
    m_pCheckBoxAudioEqualizer->AutoSize(NULL, 20.0f);
    m_pCheckBoxAudioEqualizer->GetPosition(vecTemp);

    m_pCheckBoxAudioAutotune = reinterpret_cast<CGUICheckBox*>(pManager->CreateCheckBox(pTabAudio, _("Radio Auto-tune"), true));
    m_pCheckBoxAudioAutotune->SetPosition(CVector2D(vecTemp.fX, vecTemp.fY + 20.0f));
    m_pCheckBoxAudioAutotune->AutoSize(NULL, 20.0f);
    m_pCheckBoxAudioAutotune->GetPosition(vecTemp);

    m_pAudioUsertrackLabel = reinterpret_cast<CGUILabel*>(pManager->CreateLabel(pTabAudio, _("Usertrack options")));
    m_pAudioUsertrackLabel->SetPosition(CVector2D(vecTemp.fX, vecTemp.fY + 30.0f), false);
    m_pAudioUsertrackLabel->GetPosition(vecTemp, false);
    m_pAudioUsertrackLabel->AutoSize(NULL, 20.0f);
    m_pAudioUsertrackLabel->SetFont("default-bold-small");

    m_pLabelUserTrackMode = reinterpret_cast<CGUILabel*>(pManager->CreateLabel(pTabAudio, _("Play mode:")));
    m_pLabelUserTrackMode->SetPosition(CVector2D(vecTemp.fX, vecTemp.fY + 30.0f));
    m_pLabelUserTrackMode->GetPosition(vecTemp, false);
    m_pLabelUserTrackMode->AutoSize();

    m_pComboUsertrackMode = reinterpret_cast<CGUIComboBox*>(pManager->CreateComboBox(pTabAudio, ""));
    m_pComboUsertrackMode->SetPosition(CVector2D(vecTemp.fX, vecTemp.fY + 20.0f));
    m_pComboUsertrackMode->SetSize(CVector2D(160.0f, 80.0f));
    m_pComboUsertrackMode->AddItem(_("Radio"))->SetData((void*)0);
    m_pComboUsertrackMode->AddItem(_("Random"))->SetData((void*)1);
    m_pComboUsertrackMode->AddItem(_("Sequential"))->SetData((void*)2);
    m_pComboUsertrackMode->SetReadOnly(true);

    m_pCheckBoxUserAutoscan = reinterpret_cast<CGUICheckBox*>(pManager->CreateCheckBox(pTabAudio, _("Automatic Media Scan"), true));
    m_pCheckBoxUserAutoscan->SetPosition(CVector2D(vecTemp.fX, vecTemp.fY + 52.0f));
    m_pCheckBoxUserAutoscan->AutoSize(NULL, 20.0f);
    m_pCheckBoxUserAutoscan->GetPosition(vecTemp, false);

    m_pAudioRadioLabel->GetPosition(vecTemp, false);
    vecTemp.fX = fIndentX + 173;
    m_pAudioMuteLabel = reinterpret_cast<CGUILabel*>(pManager->CreateLabel(pTabAudio, _("Mute options")));
    m_pAudioMuteLabel->SetPosition(CVector2D(vecTemp.fX, vecTemp.fY + 52.0f));
    m_pAudioMuteLabel->GetPosition(vecTemp, false);
    m_pAudioMuteLabel->AutoSize(NULL, 20.0f);
    m_pAudioMuteLabel->SetFont("default-bold-small");

    m_pCheckBoxMuteMaster = reinterpret_cast<CGUICheckBox*>(pManager->CreateCheckBox(pTabAudio, _("Mute All sounds when minimized"), true));
    m_pCheckBoxMuteMaster->SetPosition(CVector2D(vecTemp.fX, vecTemp.fY + 29.0f));
    m_pCheckBoxMuteMaster->GetPosition(vecTemp, false);
    m_pCheckBoxMuteMaster->AutoSize(NULL, 20.0f);

    m_pCheckBoxMuteRadio = reinterpret_cast<CGUICheckBox*>(pManager->CreateCheckBox(pTabAudio, _("Mute Radio sounds when minimized"), true));
    m_pCheckBoxMuteRadio->SetPosition(CVector2D(vecTemp.fX, vecTemp.fY + 20.0f));
    m_pCheckBoxMuteRadio->GetPosition(vecTemp, false);
    m_pCheckBoxMuteRadio->AutoSize(NULL, 20.0f);

    m_pCheckBoxMuteSFX = reinterpret_cast<CGUICheckBox*>(pManager->CreateCheckBox(pTabAudio, _("Mute SFX sounds when minimized"), true));
    m_pCheckBoxMuteSFX->SetPosition(CVector2D(vecTemp.fX, vecTemp.fY + 20.0f));
    m_pCheckBoxMuteSFX->GetPosition(vecTemp, false);
    m_pCheckBoxMuteSFX->AutoSize(NULL, 20.0f);

    m_pCheckBoxMuteMTA = reinterpret_cast<CGUICheckBox*>(pManager->CreateCheckBox(pTabAudio, _("Mute MTA sounds when minimized"), true));
    m_pCheckBoxMuteMTA->SetPosition(CVector2D(vecTemp.fX, vecTemp.fY + 20.0f));
    m_pCheckBoxMuteMTA->GetPosition(vecTemp, false);
    m_pCheckBoxMuteMTA->AutoSize(NULL, 20.0f);

    m_pCheckBoxMuteVoice = reinterpret_cast<CGUICheckBox*>(pManager->CreateCheckBox(pTabAudio, _("Mute Voice sounds when minimized"), true));
    m_pCheckBoxMuteVoice->SetPosition(CVector2D(vecTemp.fX, vecTemp.fY + 20.0f));
    m_pCheckBoxMuteVoice->GetPosition(vecTemp, false);
    m_pCheckBoxMuteVoice->AutoSize(NULL, 20.0f);

    m_pTabs->GetSize(vecTemp);
    m_pAudioDefButton = reinterpret_cast<CGUIButton*>(pManager->CreateButton(pTabAudio, _("Load defaults")));
    m_pAudioDefButton->SetClickHandler(GUI_CALLBACK(&CSettings::OnAudioDefaultClick, this));
    m_pAudioDefButton->AutoSize(NULL, 20.0f, 8.0f);
    m_pAudioDefButton->GetSize(vecSize);
    m_pAudioDefButton->SetPosition(CVector2D(vecTemp.fX - vecSize.fX - 12.0f, 387));
    m_pAudioDefButton->SetZOrderingEnabled(false);

    /**
     *  Video tab
     **/
    fIndentX = pManager->CGUI_GetMaxTextExtent("default-normal", _("Resolution:"), _("FOV:"), _("Draw Distance:"), _("Brightness:"), _("FX Quality:"),
                                               _("Anisotropic filtering:"), _("Anti-aliasing:"), _("Aspect Ratio:"), _("Opacity:"));

    m_pVideoGeneralLabel = reinterpret_cast<CGUILabel*>(pManager->CreateLabel(pTabVideo, _("General")));
    m_pVideoGeneralLabel->SetPosition(CVector2D(11, 13));
    m_pVideoGeneralLabel->GetPosition(vecTemp, false);
    m_pVideoGeneralLabel->AutoSize(NULL, 3.0f);
    m_pVideoGeneralLabel->SetFont("default-bold-small");

    m_pVideoResolutionLabel = reinterpret_cast<CGUILabel*>(pManager->CreateLabel(pTabVideo, _("Resolution:")));
    m_pVideoResolutionLabel->SetPosition(CVector2D(vecTemp.fX, vecTemp.fY + 26.0f));
    m_pVideoResolutionLabel->GetPosition(vecTemp, false);
    m_pVideoResolutionLabel->AutoSize();

    m_pComboResolution = reinterpret_cast<CGUIComboBox*>(pManager->CreateComboBox(pTabVideo, ""));
    m_pComboResolution->SetPosition(CVector2D(vecTemp.fX + fIndentX + 5.0f, vecTemp.fY - 3.0f));
    m_pComboResolution->GetPosition(vecTemp, false);
    m_pComboResolution->SetSize(CVector2D(200.0f, 160.0f));
    m_pComboResolution->GetSize(vecSize);
    m_pComboResolution->SetReadOnly(true);

    m_pCheckBoxWindowed = reinterpret_cast<CGUICheckBox*>(pManager->CreateCheckBox(pTabVideo, _("Windowed"), true));
    m_pCheckBoxWindowed->SetPosition(CVector2D(vecTemp.fX + vecSize.fX + 10.0f, vecTemp.fY + 3.0f));
    m_pCheckBoxWindowed->AutoSize(NULL, 20.0f);
    m_pCheckBoxWindowed->GetPosition(vecTemp, false);
    m_pCheckBoxWindowed->GetSize(vecSize);

    m_pCheckBoxDPIAware = reinterpret_cast<CGUICheckBox*>(pManager->CreateCheckBox(pTabVideo, _("DPI aware"), false));
    m_pCheckBoxDPIAware->SetPosition(CVector2D(vecTemp.fX + vecSize.fX + 10.0f, vecTemp.fY));
    m_pCheckBoxDPIAware->AutoSize(NULL, 20.0f);

    m_pVideoResolutionLabel->GetPosition(vecTemp, false);            // Restore our label position

    // Fullscreen mode
    vecTemp.fY += 26;
    m_pFullscreenStyleLabel = reinterpret_cast<CGUILabel*>(pManager->CreateLabel(pTabVideo, _("Fullscreen mode:")));
    m_pFullscreenStyleLabel->SetPosition(CVector2D(vecTemp.fX, vecTemp.fY));
    m_pFullscreenStyleLabel->AutoSize();

    m_pFullscreenStyleCombo = reinterpret_cast<CGUIComboBox*>(pManager->CreateComboBox(pTabVideo, ""));
    m_pFullscreenStyleCombo->SetPosition(CVector2D(vecTemp.fX + fIndentX + 5.0f, vecTemp.fY - 1.0f));
    m_pFullscreenStyleCombo->SetSize(CVector2D(200, 95.0f));
    m_pFullscreenStyleCombo->AddItem(_("Standard"))->SetData((void*)FULLSCREEN_STANDARD);
    m_pFullscreenStyleCombo->AddItem(_("Borderless window"))->SetData((void*)FULLSCREEN_BORDERLESS);
    m_pFullscreenStyleCombo->AddItem(_("Borderless keep res"))->SetData((void*)FULLSCREEN_BORDERLESS_KEEP_RES);
    m_pFullscreenStyleCombo->SetReadOnly(true);
    vecTemp.fY += 4;

    m_pCheckBoxMipMapping = reinterpret_cast<CGUICheckBox*>(pManager->CreateCheckBox(pTabVideo, _("Mip Mapping"), true));
#ifndef MIP_MAPPING_SETTING_APPEARS_TO_DO_SOMETHING
    m_pCheckBoxMipMapping->SetPosition(CVector2D(vecTemp.fX + 340.0f, vecTemp.fY + 45.0f));
    m_pCheckBoxMipMapping->SetSize(CVector2D(224.0f, 16.0f));
    m_pCheckBoxMipMapping->SetVisible(false);
#endif

    vecTemp.fY -= 5;
    m_pFieldOfViewLabel = reinterpret_cast<CGUILabel*>(pManager->CreateLabel(pTabVideo, _("FOV:")));
    m_pFieldOfViewLabel->SetPosition(CVector2D(vecTemp.fX, vecTemp.fY + 30.0f));
    m_pFieldOfViewLabel->GetPosition(vecTemp, false);
    m_pFieldOfViewLabel->AutoSize();

    m_pFieldOfView = reinterpret_cast<CGUIScrollBar*>(pManager->CreateScrollBar(true, pTabVideo));
    m_pFieldOfView->SetPosition(CVector2D(vecTemp.fX + fIndentX + 5.0f, vecTemp.fY));
    m_pFieldOfView->GetPosition(vecTemp, false);
    m_pFieldOfView->SetSize(CVector2D(160.0f, 20.0f));
    m_pFieldOfView->GetSize(vecSize);

    m_pFieldOfViewValueLabel = reinterpret_cast<CGUILabel*>(pManager->CreateLabel(pTabVideo, "70"));
    m_pFieldOfViewValueLabel->SetPosition(CVector2D(vecTemp.fX + vecSize.fX + 5.0f, vecTemp.fY));
    m_pFieldOfViewValueLabel->AutoSize("70  ");

    vecTemp.fX = 11;
    m_pDrawDistanceLabel = reinterpret_cast<CGUILabel*>(pManager->CreateLabel(pTabVideo, _("Draw Distance:")));
    m_pDrawDistanceLabel->SetPosition(CVector2D(vecTemp.fX, vecTemp.fY + 26.0f));
    m_pDrawDistanceLabel->GetPosition(vecTemp, false);
    m_pDrawDistanceLabel->AutoSize();

    m_pDrawDistance = reinterpret_cast<CGUIScrollBar*>(pManager->CreateScrollBar(true, pTabVideo));
    m_pDrawDistance->SetPosition(CVector2D(vecTemp.fX + fIndentX + 5.0f, vecTemp.fY));
    m_pDrawDistance->GetPosition(vecTemp, false);
    m_pDrawDistance->SetSize(CVector2D(160.0f, 20.0f));
    m_pDrawDistance->GetSize(vecSize);
    m_pDrawDistance->SetProperty("StepSize", "0.01");

    m_pDrawDistanceValueLabel = reinterpret_cast<CGUILabel*>(pManager->CreateLabel(pTabVideo, "0%"));
    m_pDrawDistanceValueLabel->SetPosition(CVector2D(vecTemp.fX + vecSize.fX + 5.0f, vecTemp.fY));
    m_pDrawDistanceValueLabel->AutoSize("100%");

    vecTemp.fX = 11;

    m_pBrightnessLabel = reinterpret_cast<CGUILabel*>(pManager->CreateLabel(pTabVideo, _("Brightness:")));
    m_pBrightnessLabel->SetPosition(CVector2D(vecTemp.fX, vecTemp.fY + 26.0f));
    m_pBrightnessLabel->GetPosition(vecTemp, false);
    m_pBrightnessLabel->AutoSize();

    m_pBrightness = reinterpret_cast<CGUIScrollBar*>(pManager->CreateScrollBar(true, pTabVideo));
    m_pBrightness->SetPosition(CVector2D(vecTemp.fX + fIndentX + 5.0f, vecTemp.fY));
    m_pBrightness->GetPosition(vecTemp, false);
    m_pBrightness->SetSize(CVector2D(160.0f, 20.0f));
    m_pBrightness->GetSize(vecSize);
    m_pBrightness->SetProperty("StepSize", "0.01");

    m_pBrightnessValueLabel = reinterpret_cast<CGUILabel*>(pManager->CreateLabel(pTabVideo, "0%"));
    m_pBrightnessValueLabel->SetPosition(CVector2D(vecTemp.fX + vecSize.fX + 5.0f, vecTemp.fY));
    m_pBrightnessValueLabel->AutoSize("100%");

    vecTemp.fX = 11;

    m_pFXQualityLabel = reinterpret_cast<CGUILabel*>(pManager->CreateLabel(pTabVideo, _("FX Quality:")));
    m_pFXQualityLabel->SetPosition(CVector2D(vecTemp.fX, vecTemp.fY + 30.0f));
    m_pFXQualityLabel->GetPosition(vecTemp, false);
    m_pFXQualityLabel->AutoSize();

    m_pComboFxQuality = reinterpret_cast<CGUIComboBox*>(pManager->CreateComboBox(pTabVideo, ""));
    m_pComboFxQuality->SetPosition(CVector2D(vecTemp.fX + fIndentX + 5.0f, vecTemp.fY - 1.0f));
    m_pComboFxQuality->SetSize(CVector2D(200.0f, 95.0f));
    m_pComboFxQuality->AddItem(_("Low"))->SetData((void*)0);
    m_pComboFxQuality->AddItem(_("Medium"))->SetData((void*)1);
    m_pComboFxQuality->AddItem(_("High"))->SetData((void*)2);
    m_pComboFxQuality->AddItem(_("Very high"))->SetData((void*)3);
    m_pComboFxQuality->SetReadOnly(true);

    m_pAnisotropicLabel = reinterpret_cast<CGUILabel*>(pManager->CreateLabel(pTabVideo, _("Anisotropic filtering:")));
    m_pAnisotropicLabel->SetPosition(CVector2D(vecTemp.fX, vecTemp.fY + 30.0f));
    m_pAnisotropicLabel->GetPosition(vecTemp, false);
    m_pAnisotropicLabel->AutoSize();

    m_pAnisotropic = reinterpret_cast<CGUIScrollBar*>(pManager->CreateScrollBar(true, pTabVideo));
    m_pAnisotropic->SetPosition(CVector2D(vecTemp.fX + fIndentX + 5.0f, vecTemp.fY));
    m_pAnisotropic->GetPosition(vecTemp, false);
    m_pAnisotropic->SetSize(CVector2D(160.0f, 20.0f));
    m_pAnisotropic->GetSize(vecSize);
    m_pAnisotropic->SetProperty("StepSize", SString("%1.2f", 1 / (float)m_iMaxAnisotropic));

    m_pAnisotropicValueLabel = reinterpret_cast<CGUILabel*>(pManager->CreateLabel(pTabVideo, _("Off")));
    m_pAnisotropicValueLabel->SetPosition(CVector2D(vecTemp.fX + vecSize.fX + 5.0f, vecTemp.fY));
    m_pAnisotropicValueLabel->SetSize(CVector2D(100.0f, 20.0f));

    if (m_iMaxAnisotropic < 1)
    {
        // Hide if system can't do anisotropic filtering
        m_pFXQualityLabel->GetPosition(vecTemp, false);
        m_pAnisotropicLabel->SetVisible(false);
        m_pAnisotropic->SetVisible(false);
        m_pAnisotropicValueLabel->SetVisible(false);
    }

    vecTemp.fX = 11;

    m_pAntiAliasingLabel = reinterpret_cast<CGUILabel*>(pManager->CreateLabel(pTabVideo, _("Anti-aliasing:")));
    m_pAntiAliasingLabel->SetPosition(CVector2D(vecTemp.fX, vecTemp.fY + 30.0f));
    m_pAntiAliasingLabel->GetPosition(vecTemp, false);
    m_pAntiAliasingLabel->AutoSize();

    m_pComboAntiAliasing = reinterpret_cast<CGUIComboBox*>(pManager->CreateComboBox(pTabVideo, ""));
    m_pComboAntiAliasing->SetPosition(CVector2D(vecTemp.fX + fIndentX + 5.0f, vecTemp.fY - 1.0f));
    m_pComboAntiAliasing->SetSize(CVector2D(200.0f, 95.0f));
    m_pComboAntiAliasing->AddItem(_("Off"))->SetData((void*)1);
    m_pComboAntiAliasing->AddItem(_("1x"))->SetData((void*)2);
    m_pComboAntiAliasing->AddItem(_("2x"))->SetData((void*)3);
    m_pComboAntiAliasing->AddItem(_("3x"))->SetData((void*)4);
    m_pComboAntiAliasing->SetReadOnly(true);

    m_pAspectRatioLabel = reinterpret_cast<CGUILabel*>(pManager->CreateLabel(pTabVideo, _("Aspect Ratio:")));
    m_pAspectRatioLabel->SetPosition(CVector2D(vecTemp.fX, vecTemp.fY + 26.0f));
    m_pAspectRatioLabel->GetPosition(vecTemp, false);
    m_pAspectRatioLabel->AutoSize();

    m_pComboAspectRatio = reinterpret_cast<CGUIComboBox*>(pManager->CreateComboBox(pTabVideo, ""));
    m_pComboAspectRatio->SetPosition(CVector2D(vecTemp.fX + fIndentX + 5.0f, vecTemp.fY - 1.0f));
    m_pComboAspectRatio->GetPosition(vecTemp, false);
    m_pComboAspectRatio->SetSize(CVector2D(200.0f, 95.0f));
    m_pComboAspectRatio->GetSize(vecSize);
    m_pComboAspectRatio->AddItem(_("Auto"))->SetData((void*)ASPECT_RATIO_AUTO);
    m_pComboAspectRatio->AddItem(_("4:3"))->SetData((void*)ASPECT_RATIO_4_3);
    m_pComboAspectRatio->AddItem(_("16:10"))->SetData((void*)ASPECT_RATIO_16_10);
    m_pComboAspectRatio->AddItem(_("16:9"))->SetData((void*)ASPECT_RATIO_16_9);
    m_pComboAspectRatio->SetReadOnly(true);

    m_pCheckBoxHudMatchAspectRatio = reinterpret_cast<CGUICheckBox*>(pManager->CreateCheckBox(pTabVideo, _("HUD Match Aspect Ratio"), true));
    m_pCheckBoxHudMatchAspectRatio->SetPosition(CVector2D(vecTemp.fX + vecSize.fX + 10.0f, vecTemp.fY + 4.0f));
    m_pCheckBoxHudMatchAspectRatio->AutoSize(NULL, 20.0f);

    vecTemp.fX = 11;

    m_pCheckBoxVolumetricShadows = reinterpret_cast<CGUICheckBox*>(pManager->CreateCheckBox(pTabVideo, _("Volumetric Shadows"), true));
    m_pCheckBoxVolumetricShadows->SetPosition(CVector2D(vecTemp.fX, vecTemp.fY + 30.0f));
    m_pCheckBoxVolumetricShadows->AutoSize(NULL, 20.0f);

    m_pCheckBoxGrass = reinterpret_cast<CGUICheckBox*>(pManager->CreateCheckBox(pTabVideo, _("Grass effect"), true));
    m_pCheckBoxGrass->SetPosition(CVector2D(vecTemp.fX, vecTemp.fY + 50.0f));
    m_pCheckBoxGrass->AutoSize(NULL, 20.0f);

    m_pCheckBoxHeatHaze = reinterpret_cast<CGUICheckBox*>(pManager->CreateCheckBox(pTabVideo, _("Heat haze"), true));
    m_pCheckBoxHeatHaze->SetPosition(CVector2D(vecTemp.fX, vecTemp.fY + 70.0f));
    m_pCheckBoxHeatHaze->AutoSize(NULL, 20.0f);

    m_pCheckBoxTyreSmokeParticles = reinterpret_cast<CGUICheckBox*>(pManager->CreateCheckBox(pTabVideo, _("Tyre Smoke etc"), true));
    m_pCheckBoxTyreSmokeParticles->SetPosition(CVector2D(vecTemp.fX, vecTemp.fY + 90.0f));
    m_pCheckBoxTyreSmokeParticles->AutoSize(NULL, 20.0f);

    m_pCheckBoxDynamicPedShadows = reinterpret_cast<CGUICheckBox*>(pManager->CreateCheckBox(pTabVideo, _("Dynamic ped shadows"), true));
    m_pCheckBoxDynamicPedShadows->SetPosition(CVector2D(vecTemp.fX, vecTemp.fY + 110.0f));
    m_pCheckBoxDynamicPedShadows->AutoSize(NULL, 20.0f);

    m_pCheckBoxBlur = reinterpret_cast<CGUICheckBox*>(pManager->CreateCheckBox(pTabVideo, _("Motion blur"), true));
    m_pCheckBoxBlur->SetPosition(CVector2D(vecTemp.fX, vecTemp.fY + 130.0f));
    m_pCheckBoxBlur->AutoSize(NULL, 20.0f);

    float fPosY = vecTemp.fY;
    m_pCheckBoxMinimize = reinterpret_cast<CGUICheckBox*>(pManager->CreateCheckBox(pTabVideo, _("Full Screen Minimize"), true));
    m_pCheckBoxMinimize->SetPosition(CVector2D(vecTemp.fX + 245.0f, fPosY + 30.0f));
    m_pCheckBoxMinimize->AutoSize(NULL, 20.0f);

#ifndef SHOWALLSETTINGS
    if (!GetVideoModeManager()->IsMultiMonitor())
    {
        m_pCheckBoxMinimize->SetVisible(false);
        fPosY -= 20.0f;
    }
#endif

    m_pCheckBoxDeviceSelectionDialog = reinterpret_cast<CGUICheckBox*>(pManager->CreateCheckBox(pTabVideo, _("Enable Device Selection Dialog"), true));
    m_pCheckBoxDeviceSelectionDialog->SetPosition(CVector2D(vecTemp.fX + 245.0f, fPosY + 50.0f));
    m_pCheckBoxDeviceSelectionDialog->AutoSize(NULL, 20.0f);

#ifndef SHOWALLSETTINGS
    if (!GetVideoModeManager()->IsMultiMonitor())
    {
        m_pCheckBoxDeviceSelectionDialog->SetVisible(false);
        fPosY -= 20.0f;
    }
#endif

    m_pCheckBoxShowUnsafeResolutions = reinterpret_cast<CGUICheckBox*>(pManager->CreateCheckBox(pTabVideo, _("Show unsafe resolutions"), true));
    m_pCheckBoxShowUnsafeResolutions->SetPosition(CVector2D(vecTemp.fX + 245.0f, fPosY + 70.0f));
    m_pCheckBoxShowUnsafeResolutions->AutoSize(NULL, 20.0f);

#ifndef SHOWALLSETTINGS
    if (!CCore::GetSingleton().GetGame()->GetSettings()->HasUnsafeResolutions())
    {
        m_pCheckBoxShowUnsafeResolutions->SetVisible(false);
        fPosY -= 20.0f;
    }
#endif

    m_pCheckBoxHighDetailVehicles = reinterpret_cast<CGUICheckBox*>(pManager->CreateCheckBox(pTabVideo, _("Render vehicles always in high detail"), true));
    m_pCheckBoxHighDetailVehicles->SetPosition(CVector2D(vecTemp.fX + 245.0f, fPosY + 90.0f));
    m_pCheckBoxHighDetailVehicles->AutoSize(NULL, 20.0f);

    m_pCheckBoxHighDetailPeds = reinterpret_cast<CGUICheckBox*>(pManager->CreateCheckBox(pTabVideo, _("Render peds always in high detail"), true));
    m_pCheckBoxHighDetailPeds->SetPosition(CVector2D(vecTemp.fX + 245.0f, fPosY + 110.0f));
    m_pCheckBoxHighDetailPeds->AutoSize(NULL, 20.0f);

    m_pCheckBoxCoronaReflections = reinterpret_cast<CGUICheckBox*>(pManager->CreateCheckBox(pTabVideo, _("Corona rain reflections"), true));
    m_pCheckBoxCoronaReflections->SetPosition(CVector2D(vecTemp.fX + 245.0f, fPosY + 130.0f));
    m_pCheckBoxCoronaReflections->AutoSize(NULL, 20.0f);

    vecTemp.fY += 10;

    m_pTabs->GetSize(vecTemp);

    m_pVideoDefButton = reinterpret_cast<CGUIButton*>(pManager->CreateButton(pTabVideo, _("Load defaults")));
    m_pVideoDefButton->SetClickHandler(GUI_CALLBACK(&CSettings::OnVideoDefaultClick, this));
    m_pVideoDefButton->AutoSize(NULL, 20.0f, 8.0f);
    m_pVideoDefButton->GetSize(vecSize);
    m_pVideoDefButton->SetPosition(CVector2D(vecTemp.fX - vecSize.fX - 12.0f, 387));
    m_pVideoDefButton->SetZOrderingEnabled(false);

    // Set up the events
    m_pWindow->SetEnterKeyHandler(GUI_CALLBACK(&CSettings::OnOKButtonClick, this));
    m_pButtonOK->SetClickHandler(GUI_CALLBACK(&CSettings::OnOKButtonClick, this));
    m_pButtonCancel->SetClickHandler(GUI_CALLBACK(&CSettings::OnCancelButtonClick, this));
    m_pAudioMasterVolume->SetOnScrollHandler(GUI_CALLBACK(&CSettings::OnMasterVolumeChanged, this));
    m_pAudioRadioVolume->SetOnScrollHandler(GUI_CALLBACK(&CSettings::OnRadioVolumeChanged, this));
    m_pAudioSFXVolume->SetOnScrollHandler(GUI_CALLBACK(&CSettings::OnSFXVolumeChanged, this));
    m_pAudioMTAVolume->SetOnScrollHandler(GUI_CALLBACK(&CSettings::OnMTAVolumeChanged, this));
    m_pAudioVoiceVolume->SetOnScrollHandler(GUI_CALLBACK(&CSettings::OnVoiceVolumeChanged, this));
    m_pCheckBoxMuteMaster->SetClickHandler(GUI_CALLBACK(&CSettings::OnMasterMuteMinimizedChanged, this));
    m_pCheckBoxMuteRadio->SetClickHandler(GUI_CALLBACK(&CSettings::OnRadioMuteMinimizedChanged, this));
    m_pCheckBoxMuteSFX->SetClickHandler(GUI_CALLBACK(&CSettings::OnSFXMuteMinimizedChanged, this));
    m_pCheckBoxMuteMTA->SetClickHandler(GUI_CALLBACK(&CSettings::OnMTAMuteMinimizedChanged, this));
    m_pCheckBoxMuteVoice->SetClickHandler(GUI_CALLBACK(&CSettings::OnVoiceMuteMinimizedChanged, this));
    m_pFieldOfView->SetOnScrollHandler(GUI_CALLBACK(&CSettings::OnFieldOfViewChanged, this));
    m_pDrawDistance->SetOnScrollHandler(GUI_CALLBACK(&CSettings::OnDrawDistanceChanged, this));
    m_pBrightness->SetOnScrollHandler(GUI_CALLBACK(&CSettings::OnBrightnessChanged, this));
    m_pAnisotropic->SetOnScrollHandler(GUI_CALLBACK(&CSettings::OnAnisotropicChanged, this));
    m_pMouseSensitivity->SetOnScrollHandler(GUI_CALLBACK(&CSettings::OnMouseSensitivityChanged, this));
    m_pVerticalAimSensitivity->SetOnScrollHandler(GUI_CALLBACK(&CSettings::OnVerticalAimSensitivityChanged, this));
    m_pComboFxQuality->SetSelectionHandler(GUI_CALLBACK(&CSettings::OnFxQualityChanged, this));
    m_pCheckBoxVolumetricShadows->SetClickHandler(GUI_CALLBACK(&CSettings::OnVolumetricShadowsClick, this));
    m_pCheckBoxWindowed->SetClickHandler(GUI_CALLBACK(&CSettings::OnWindowedClick, this));
    m_pCheckBoxDPIAware->SetClickHandler(GUI_CALLBACK(&CSettings::OnDPIAwareClick, this));
    m_pCheckBoxShowUnsafeResolutions->SetClickHandler(GUI_CALLBACK(&CSettings::ShowUnsafeResolutionsClick, this));
}

void CSettings::DestroyGUI()
{
    // Destroy
    delete m_pButtonCancel;
    delete m_pButtonOK;
    delete m_pWindow;
    m_pWindow = NULL;
}

void RestartCallBack(void* ptr, unsigned int uiButton)
{
    CCore::GetSingleton().GetLocalGUI()->GetMainMenu()->GetQuestionWindow()->Reset();

    if (uiButton == 1)
    {
        SetOnQuitCommand("restart");
        CCore::GetSingleton().Quit();
    }
}

void CSettings::ShowRestartQuestion()
{
    SString strMessage = _("Some settings will be changed when you next start MTA");
    strMessage += _("\n\nDo you want to restart now?");
    CQuestionBox* pQuestionBox = CCore::GetSingleton().GetLocalGUI()->GetMainMenu()->GetQuestionWindow();
    pQuestionBox->Reset();
    pQuestionBox->SetTitle(_("RESTART REQUIRED"));
    pQuestionBox->SetMessage(strMessage);
    pQuestionBox->SetButton(0, _("No"));
    pQuestionBox->SetButton(1, _("Yes"));
    pQuestionBox->SetCallback(RestartCallBack);
    pQuestionBox->Show();
}

bool CSettings::OnMouseDoubleClick(CGUIMouseEventArgs Args)
{
    if (Args.pWindow == m_pBindsList)
    {
        OnBindsListClick(m_pBindsList);
        return true;
    }

    return false;
}

void CSettings::Update()
{
    UpdateCaptureAxis();
}

void CSettings::UpdateAudioTab()
{
    float fMasterVolume = 0, fMTAVolume = 0, fVoiceVolume = 0;

    CVARS_GET("mastervolume", fMasterVolume);
    CVARS_GET("mtavolume", fMTAVolume);
    CVARS_GET("voicevolume", fVoiceVolume);

    CGameSettings* gameSettings = CCore::GetSingleton().GetGame()->GetSettings();

    m_pAudioRadioVolume->SetScrollPosition(m_fRadioVolume);
    m_pAudioSFXVolume->SetScrollPosition(m_fSFXVolume);
    m_pAudioMTAVolume->SetScrollPosition(fMTAVolume);
    m_pAudioVoiceVolume->SetScrollPosition(fVoiceVolume);
    // Lastly, set our master volume scroll position
    m_pAudioMasterVolume->SetScrollPosition(fMasterVolume);

    m_pCheckBoxAudioEqualizer->SetSelected(gameSettings->IsRadioEqualizerEnabled());
    m_pCheckBoxAudioAutotune->SetSelected(gameSettings->IsRadioAutotuneEnabled());
    m_pCheckBoxUserAutoscan->SetSelected(gameSettings->IsUsertrackAutoScan());

    CVARS_GET("mute_master_when_minimized", m_bMuteMaster);
    CVARS_GET("mute_radio_when_minimized", m_bMuteRadio);
    CVARS_GET("mute_sfx_when_minimized", m_bMuteSFX);
    CVARS_GET("mute_mta_when_minimized", m_bMuteMTA);
    CVARS_GET("mute_voice_when_minimized", m_bMuteVoice);

    m_pCheckBoxMuteMaster->SetSelected(m_bMuteMaster);
    m_pCheckBoxMuteRadio->SetSelected(m_bMuteRadio);
    m_pCheckBoxMuteSFX->SetSelected(m_bMuteSFX);
    m_pCheckBoxMuteMTA->SetSelected(m_bMuteMTA);
    m_pCheckBoxMuteVoice->SetSelected(m_bMuteVoice);

    m_pCheckBoxMuteRadio->SetEnabled(!m_bMuteMaster);
    m_pCheckBoxMuteSFX->SetEnabled(!m_bMuteMaster);
    m_pCheckBoxMuteMTA->SetEnabled(!m_bMuteMaster);
    m_pCheckBoxMuteVoice->SetEnabled(!m_bMuteMaster);

    m_pComboUsertrackMode->SetSelectedItemByIndex(gameSettings->GetUsertrackMode());
}

void CSettings::UpdateVideoTab()
{
    CGameSettings* gameSettings = CCore::GetSingleton().GetGame()->GetSettings();

    bool bNextWindowed;
    bool bNextFSMinimize;
    int  iNextVidMode;
    int  iNextFullscreenStyle;
    GetVideoModeManager()->GetNextVideoMode(iNextVidMode, bNextWindowed, bNextFSMinimize, iNextFullscreenStyle);

    m_pCheckBoxMipMapping->SetSelected(gameSettings->IsMipMappingEnabled());
    m_pCheckBoxWindowed->SetSelected(bNextWindowed);
    m_pCheckBoxMinimize->SetSelected(bNextFSMinimize);
    m_pDrawDistance->SetScrollPosition((gameSettings->GetDrawDistance() - 0.925f) / 0.8749f);
    m_pBrightness->SetScrollPosition((float)gameSettings->GetBrightness() / 384);

    // DPI aware
    bool processDPIAware = false;
    CVARS_GET("process_dpi_aware", processDPIAware);
    m_pCheckBoxDPIAware->SetSelected(processDPIAware);

    // FieldOfView
    int iFieldOfView;
    CVARS_GET("fov", iFieldOfView);
    m_pFieldOfView->SetScrollPosition((iFieldOfView - 70) / 30.f);

    // Anisotropic filtering
    int iAnisotropic;
    CVARS_GET("anisotropic", iAnisotropic);
    m_pAnisotropic->SetScrollPosition(iAnisotropic / (float)m_iMaxAnisotropic);

    int FxQuality = gameSettings->GetFXQuality();
    if (FxQuality == 0)
        m_pComboFxQuality->SetText(_("Low"));
    else if (FxQuality == 1)
        m_pComboFxQuality->SetText(_("Medium"));
    else if (FxQuality == 2)
        m_pComboFxQuality->SetText(_("High"));
    else if (FxQuality == 3)
        m_pComboFxQuality->SetText(_("Very high"));

    char AntiAliasing = gameSettings->GetAntiAliasing();
    if (AntiAliasing == 1)
        m_pComboAntiAliasing->SetText(_("Off"));
    else if (AntiAliasing == 2)
        m_pComboAntiAliasing->SetText(_("1x"));
    else if (AntiAliasing == 3)
        m_pComboAntiAliasing->SetText(_("2x"));
    else if (AntiAliasing == 4)
        m_pComboAntiAliasing->SetText(_("3x"));

    // Aspect ratio
    int aspectRatio;
    CVARS_GET("aspect_ratio", aspectRatio);
    if (aspectRatio == ASPECT_RATIO_AUTO)
        m_pComboAspectRatio->SetText(_("Auto"));
    else if (aspectRatio == ASPECT_RATIO_4_3)
        m_pComboAspectRatio->SetText(_("4:3"));
    else if (aspectRatio == ASPECT_RATIO_16_10)
        m_pComboAspectRatio->SetText(_("16:10"));
    else if (aspectRatio == ASPECT_RATIO_16_9)
        m_pComboAspectRatio->SetText(_("16:9"));

    // HUD match aspect ratio
    m_pCheckBoxHudMatchAspectRatio->SetSelected(CVARS_GET_VALUE<bool>("hud_match_aspect_ratio"));

    // Volumetric shadows
    bool bVolumetricShadowsEnabled;
    CVARS_GET("volumetric_shadows", bVolumetricShadowsEnabled);
    m_pCheckBoxVolumetricShadows->SetSelected(bVolumetricShadowsEnabled);
    m_pCheckBoxVolumetricShadows->SetEnabled(FxQuality != 0);

    // Device selection dialog
    bool bDeviceSelectionDialogEnabled = GetApplicationSettingInt("device-selection-disabled") ? false : true;
    m_pCheckBoxDeviceSelectionDialog->SetSelected(bDeviceSelectionDialogEnabled);

    // Show unsafe resolutions
    bool bShowUnsafeResolutions;
    CVARS_GET("show_unsafe_resolutions", bShowUnsafeResolutions);
    m_pCheckBoxShowUnsafeResolutions->SetSelected(bShowUnsafeResolutions);

    // Grass
    bool bGrassEnabled;
    CVARS_GET("grass", bGrassEnabled);
    m_pCheckBoxGrass->SetSelected(bGrassEnabled);
    m_pCheckBoxGrass->SetEnabled(FxQuality != 0);

    // Heat haze
    bool bHeatHazeEnabled;
    CVARS_GET("heat_haze", bHeatHazeEnabled);
    m_pCheckBoxHeatHaze->SetSelected(bHeatHazeEnabled);

    // Tyre smoke
    bool bTyreSmokeEnabled;
    CVARS_GET("tyre_smoke_enabled", bTyreSmokeEnabled);
    m_pCheckBoxTyreSmokeParticles->SetSelected(bTyreSmokeEnabled);

    // High detail vehicles
    bool bHighDetailVehicles;
    CVARS_GET("high_detail_vehicles", bHighDetailVehicles);
    m_pCheckBoxHighDetailVehicles->SetSelected(bHighDetailVehicles);

    // High detail peds
    bool bHighDetailPeds;
    CVARS_GET("high_detail_peds", bHighDetailPeds);
    m_pCheckBoxHighDetailPeds->SetSelected(bHighDetailPeds);

    // Corona rain reflections
    bool bCoronaReflections;
    CVARS_GET("corona_reflections", bCoronaReflections);
    m_pCheckBoxCoronaReflections->SetSelected(bCoronaReflections);

    // Dynamic ped shadows
    bool bDynamicPedShadows;
    CVARS_GET("dynamic_ped_shadows", bDynamicPedShadows);
    m_pCheckBoxDynamicPedShadows->SetSelected(bDynamicPedShadows);

    // Enable dynamic ped shadows checkbox if visual quality option is set to high or very high
    m_pCheckBoxDynamicPedShadows->SetEnabled(FxQuality >= 2);

    PopulateResolutionComboBox();

    // Fullscreen style
    if (iNextFullscreenStyle == FULLSCREEN_STANDARD)
        m_pFullscreenStyleCombo->SetText(_("Standard"));
    else if (iNextFullscreenStyle == FULLSCREEN_BORDERLESS)
        m_pFullscreenStyleCombo->SetText(_("Borderless window"));
    else if (iNextFullscreenStyle == FULLSCREEN_BORDERLESS_KEEP_RES)
        m_pFullscreenStyleCombo->SetText(_("Borderless keep res"));

    UpdateFullScreenComboBoxEnabled();
}

//
// PopulateResolutionComboBox
//
void CSettings::PopulateResolutionComboBox()
{
    bool bNextWindowed;
    bool bNextFSMinimize;
    int  iNextVidMode;
    int  iNextFullscreenStyle;
    GetVideoModeManager()->GetNextVideoMode(iNextVidMode, bNextWindowed, bNextFSMinimize, iNextFullscreenStyle);
    bool bShowUnsafeResolutions = m_pCheckBoxShowUnsafeResolutions->GetSelected();

    CGameSettings* gameSettings = CCore::GetSingleton().GetGame()->GetSettings();

    VideoMode vidModemInfo;
    int       vidMode, numVidModes;

    m_pComboResolution->Clear();
    numVidModes = gameSettings->GetNumVideoModes();

    for (vidMode = 0; vidMode < numVidModes; vidMode++)
    {
        gameSettings->GetVideoModeInfo(&vidModemInfo, vidMode);

        // Remove resolutions that will make the gui unusable
        if (vidModemInfo.width < 800 || vidModemInfo.height < 600)
            continue;

        // Check resolution hasn't already been added
        bool bDuplicate = false;
        for (int i = 1; i < vidMode; i++)
        {
            VideoMode info;
            gameSettings->GetVideoModeInfo(&info, i);
            if (info.width == vidModemInfo.width && info.height == vidModemInfo.height && info.depth == vidModemInfo.depth)
                bDuplicate = true;
        }
        if (bDuplicate)
            continue;

        // Check resolution is below desktop res unless that is allowed
        if (gameSettings->IsUnsafeResolution(vidModemInfo.width, vidModemInfo.height) && !bShowUnsafeResolutions)
            continue;

        SString strMode("%lu x %lu x %lu", vidModemInfo.width, vidModemInfo.height, vidModemInfo.depth);

        if (vidModemInfo.flags & rwVIDEOMODEEXCLUSIVE)
            m_pComboResolution->AddItem(strMode)->SetData((void*)vidMode);

        VideoMode currentInfo;
        gameSettings->GetVideoModeInfo(&currentInfo, iNextVidMode);

        if (currentInfo.width == vidModemInfo.width && currentInfo.height == vidModemInfo.height && currentInfo.depth == vidModemInfo.depth)
            m_pComboResolution->SetText(strMode);
    }
}

//
// Disable/enable the full screen mode options
//
void CSettings::UpdateFullScreenComboBoxEnabled()
{
    if (m_pCheckBoxWindowed->GetSelected())
    {
        m_pFullscreenStyleLabel->SetAlpha(0.42f);
        m_pFullscreenStyleCombo->SetAlpha(0.42f);
        m_pFullscreenStyleCombo->SetEnabled(false);
    }
    else
    {
        m_pFullscreenStyleLabel->SetAlpha(1);
        m_pFullscreenStyleCombo->SetAlpha(1);
        m_pFullscreenStyleCombo->SetEnabled(true);
    }
}

//
// Called every frame. If capturing an axis, see if its all done yet.
//
void CSettings::UpdateCaptureAxis()
{
    if (m_bCaptureAxis)
    {
        // Are we done?
        if (GetJoystickManager()->IsAxisBindComplete())
        {
            // Remove the messagebox we created earlier
            CCore::GetSingleton().RemoveMessageBox();

            m_bCaptureAxis = false;
        }
    }
}

//
// Called when the user clicks on the video 'Load Defaults' button.
//
bool CSettings::OnVideoDefaultClick(CGUIElement* pElement)
{
    CGameSettings* gameSettings = CCore::GetSingleton().GetGame()->GetSettings();

    CVARS_SET("aspect_ratio", ASPECT_RATIO_AUTO);
    CVARS_SET("fov", 100);
    CVARS_SET("anisotropic", 0);
    CVARS_SET("volumetric_shadows", false);
    CVARS_SET("grass", true);
    CVARS_SET("heat_haze", true);
    CVARS_SET("tyre_smoke_enabled", true);
    CVARS_SET("high_detail_vehicles", false);
    CVARS_SET("high_detail_peds", false);
    CVARS_SET("blur", true);
    CVARS_SET("corona_reflections", false);
    CVARS_SET("dynamic_ped_shadows", false);
    gameSettings->UpdateFieldOfViewFromSettings();
    gameSettings->SetDrawDistance(1.19625f);            // All values taken from a default SA install, no gta_sa.set or coreconfig.xml modifications.
    gameSettings->SetBrightness(253);
    gameSettings->SetFXQuality(2);
    gameSettings->SetAntiAliasing(1, true);
    gameSettings->ResetVehiclesLODDistance(false);
    gameSettings->ResetPedsLODDistance(false);
    gameSettings->SetDynamicPedShadowsEnabled(false);

    // change
    bool bIsVideoModeChanged = GetVideoModeManager()->SetVideoMode(0, false, false, FULLSCREEN_STANDARD);

    // Display restart required message if required
    bool bIsAntiAliasingChanged = gameSettings->GetAntiAliasing() != m_pComboAntiAliasing->GetSelectedItemIndex();
    if (bIsVideoModeChanged || bIsAntiAliasingChanged)
        ShowRestartQuestion();

    // Update the GUI
    UpdateVideoTab();

    return true;
}

void CSettings::ResetGTAVolume()
{
    // This will set the GTA volume to the GTA volume value in the settings,
    // and is not affected by the master volume setting.
    CGameSettings* gameSettings = CCore::GetSingleton().GetGame()->GetSettings();
    gameSettings->SetRadioVolume(m_fRadioVolume * 64.0f);
    gameSettings->SetSFXVolume(m_fSFXVolume * 64.0f);
    gameSettings->Save();
}

void CSettings::SetRadioVolume(float fVolume)
{
    fVolume = std::max(0.0f, std::min(fVolume, 1.0f));

    m_fRadioVolume = fVolume;

    CCore::GetSingleton().GetGame()->GetSettings()->SetRadioVolume(m_fRadioVolume * CVARS_GET_VALUE<float>("mastervolume") * 64.0f);
}

void CSettings::SetSFXVolume(float fVolume)
{
    fVolume = std::max(0.0f, std::min(fVolume, 1.0f));

    m_fSFXVolume = fVolume;

    CCore::GetSingleton().GetGame()->GetSettings()->SetSFXVolume(m_fSFXVolume * CVARS_GET_VALUE<float>("mastervolume") * 64.0f);
}

//
// Called when the user clicks on the audio 'Load Defaults' button.
//
bool CSettings::OnAudioDefaultClick(CGUIElement* pElement)
{
    CVARS_SET("mastervolume", 1.0f);
    SetRadioVolume(1.0f);
    SetSFXVolume(1.0f);
    CVARS_SET("mtavolume", 1.0f);
    CVARS_SET("voicevolume", 1.0f);

    CVARS_SET("mute_master_when_minimized", false);
    CVARS_SET("mute_sfx_when_minimized", false);
    CVARS_SET("mute_radio_when_minimized", false);
    CVARS_SET("mute_mta_when_minimized", false);
    CVARS_SET("mute_voice_when_minimized", false);

    CGameSettings* gameSettings = CCore::GetSingleton().GetGame()->GetSettings();

    gameSettings->SetRadioAutotuneEnabled(true);
    gameSettings->SetRadioEqualizerEnabled(true);

    gameSettings->SetUsertrackAutoScan(false);
    gameSettings->SetUsertrackMode(0);

    // Update the GUI
    UpdateAudioTab();

    return true;
}

//
// Called when the user clicks on an map axis button. Starts the capture axis process.
//
bool CSettings::OnAxisSelectClick(CGUIElement* pElement)
{
    int index = reinterpret_cast<int>(pElement->GetUserData());

    if (GetJoystickManager()->BindNextUsedAxisToOutput(index))
    {
        m_bCaptureAxis = true;
        CCore::GetSingleton().ShowMessageBox(_("Binding axis"), _("Move an axis to bind, or escape to clear"), MB_ICON_QUESTION);
    }

    return true;
}

//
// Called when the user clicks on the controls tab 'Load Defaults' button.
//
bool CSettings::OnControlsDefaultClick(CGUIElement* pElement)
{
    CGameSettings*            gameSettings = CCore::GetSingleton().GetGame()->GetSettings();
    CControllerConfigManager* pController = g_pCore->GetGame()->GetControllerConfigManager();

    // Load the default settings
    GetJoystickManager()->SetDefaults();
    CVARS_SET("invert_mouse", false);
    CVARS_SET("fly_with_mouse", false);
    CVARS_SET("steer_with_mouse", false);
    pController->SetVerticalAimSensitivity(0.5f);
    CVARS_SET("vertical_aim_sensitivity", pController->GetVerticalAimSensitivityRawValue());
    gameSettings->SetMouseSensitivity(0.5f);

    // Set game vars
    pController->SetMouseInverted(CVARS_GET_VALUE<bool>("invert_mouse"));
    pController->SetFlyWithMouse(CVARS_GET_VALUE<bool>("fly_with_mouse"));
    pController->SetSteerWithMouse(CVARS_GET_VALUE<bool>("steer_with_mouse"));
    pController->SetVerticalAimSensitivityRawValue(CVARS_GET_VALUE<float>("vertical_aim_sensitivity"));

    // Update the GUI
    m_pInvertMouse->SetSelected(CVARS_GET_VALUE<bool>("invert_mouse"));
    m_pFlyWithMouse->SetSelected(CVARS_GET_VALUE<bool>("fly_with_mouse"));
    m_pSteerWithMouse->SetSelected(CVARS_GET_VALUE<bool>("steer_with_mouse"));
    m_pMouseSensitivity->SetScrollPosition(gameSettings->GetMouseSensitivity());
    m_pVerticalAimSensitivity->SetScrollPosition(pController->GetVerticalAimSensitivity());

    return true;
}

//
// Called when the user clicks on the bind 'Load Defaults' button.
//
bool CSettings::OnBindsDefaultClick(CGUIElement* pElement)
{
    // Load the default binds
    CCore::GetSingleton().GetKeyBinds()->LoadDefaultBinds();

    // Clear the binds list
    m_pBindsList->Clear();

    // Re-initialize the binds list
    Initialize();

    return true;
}

// Saves the keybinds
void CSettings::ProcessKeyBinds()
{
    CKeyBindsInterface* pKeyBinds = CCore::GetSingleton().GetKeyBinds();

    SString strResource;

    // Loop through every row in the binds list
    for (int i = 0; i < m_pBindsList->GetRowCount(); i++)
    {
        // Get the type and keys
        auto                bindType = static_cast<KeyBindType>(reinterpret_cast<intptr_t>(m_pBindsList->GetItemData(i, m_hBind)));
        const char*         szPri = m_pBindsList->GetItemText(i, m_hPriKey);
        const SBindableKey* pPriKey = pKeyBinds->GetBindableFromKey(szPri);
        const SBindableKey* pSecKeys[SecKeyNum];
        for (int k = 0; k < SecKeyNum; k++)
        {
            const char* szSec = m_pBindsList->GetItemText(i, m_hSecKeys[k]);
            pSecKeys[k] = pKeyBinds->GetBindableFromKey(szSec);
        }
        // If it is a resource name
        if (static_cast<intptr_t>(bindType) == 255)
        {
            strResource = m_pBindsList->GetItemText(i, m_hBind);
        }
        // If the type is control
        else if (bindType == KeyBindType::GTA_CONTROL)
        {
            // Get the previous bind
            CGTAControlBind* pBind = reinterpret_cast<CGTAControlBind*>(m_pBindsList->GetItemData(i, m_hPriKey));
            if (pBind)
            {
                // If theres a primary key, change it
                if (pPriKey)
                    pBind->boundKey = pPriKey;
                else
                    // If not remove the bind
                    pKeyBinds->Remove(pBind);
            }

            for (int k = 0; k < SecKeyNum; k++)
            {
                CGTAControlBind* pSecBind = reinterpret_cast<CGTAControlBind*>(m_pBindsList->GetItemData(i, m_hSecKeys[k]));
                if (pSecBind)
                {
                    // If theres a secondary key, change it
                    if (pSecKeys[k])
                    {
                        if (pSecBind->boundKey != pSecKeys[k])
                        {
                            if (!pKeyBinds->GTAControlExists(pSecKeys[k], pSecBind->control))
                            {
                                pSecBind->boundKey = pSecKeys[k];
                            }
                            else
                            {
                                // If not remove the bind
                                pKeyBinds->Remove(pSecBind);
                            }
                        }
                    }
                    else
                    {
                        // If not remove the bind
                        pKeyBinds->Remove(pSecBind);
                    }
                }
                else if (pSecKeys[k] && pBind)
                {
                    SBindableGTAControl* pControl = pBind->control;
                    if (!pKeyBinds->GTAControlExists(pSecKeys[k], pControl))
                        pKeyBinds->AddGTAControl(pSecKeys[k], pControl);
                }
            }
        }
        // If the type is an empty control (wasn't bound before)
        else if (bindType == KeyBindType::UNDEFINED)
        {
            // Grab the stored control
            SBindableGTAControl* pControl = reinterpret_cast<SBindableGTAControl*>(m_pBindsList->GetItemData(i, m_hPriKey));
            if (pPriKey)
                // If theres a new key for primary, add a new bind
                pKeyBinds->AddGTAControl(pPriKey, pControl);
            for (int k = 0; k < SecKeyNum; k++)
                if (pSecKeys[k])
                    // If theres a new key for secondary, add a new bind
                    pKeyBinds->AddGTAControl(pSecKeys[k], pControl);
        }
        // If the type is a command
        else if (bindType == KeyBindType::COMMAND)
        {
            SString strCmdArgs = m_pBindsList->GetItemText(i, m_hBind);

            SString strCommand, strArguments;
            strCmdArgs.Split(": ", &strCommand, &strArguments);

            const char* szCommand = strCommand;
            const char* szArguments = strArguments.empty() ? NULL : strArguments.c_str();

            /** Primary keybinds **/
            CCommandBind* pBind = reinterpret_cast<CCommandBind*>(m_pBindsList->GetItemData(i, m_hPriKey));
            // If a keybind for this command already exists
            if (pBind)
            {
                // If the user specified a valid primary key
                if (pPriKey)
                {
                    // If the primary key is different than the original one
                    if (pPriKey != pBind->boundKey)
                    {
                        // Did we have any keys with the same "up" state?
                        CCommandBind* pUpBind = pKeyBinds->FindMatchingUpBind(pBind);
                        if (pUpBind)
                        {
                            pKeyBinds->UserChangeCommandBoundKey(pUpBind, pPriKey);
                        }

                        pKeyBinds->UserChangeCommandBoundKey(pBind, pPriKey);
                    }
                }
                // If the primary key field was empty, we can remove the keybind
                else
                {
                    // Remove any matching "up" state binds we may have
                    CCommandBind* pUpBind = pKeyBinds->FindMatchingUpBind(pBind);
                    if (pUpBind)
                    {
                        pKeyBinds->UserRemoveCommandBoundKey(pUpBind);
                    }
                    pKeyBinds->UserRemoveCommandBoundKey(pBind);
                }
            }
            // If there was no keybind for this command, create it
            else if (pPriKey)
            {
                if (strResource.empty())
                    pKeyBinds->AddCommand(pPriKey, szCommand, szArguments);
                else
                    pKeyBinds->AddCommand(pPriKey->szKey, szCommand, szArguments, true, strResource.c_str());
            }

            /** Secondary keybinds **/
            for (int k = 0; k < SecKeyNum; k++)
            {
                pBind = reinterpret_cast<CCommandBind*>(m_pBindsList->GetItemData(i, m_hSecKeys[k]));
                // If this is a valid bind in the keybinds list
                if (pBind)
                {
                    // And our secondary key field was not empty
                    if (pSecKeys[k])
                    {
                        if (pSecKeys[k] != pBind->boundKey)
                        {
                            // Did we have any keys with the same "up" state?
                            CCommandBind* pUpBind = pKeyBinds->FindMatchingUpBind(pBind);
                            if (pUpBind)
                            {
                                pKeyBinds->UserChangeCommandBoundKey(pUpBind, pSecKeys[k]);
                            }
                            pKeyBinds->UserChangeCommandBoundKey(pBind, pSecKeys[k]);
                        }
                    }
                    // If the secondary key field was empty, we should remove the keybind
                    else
                    {
                        // Remove any matching "up" state binds we may have
                        CCommandBind* pUpBind = pKeyBinds->FindMatchingUpBind(pBind);
                        if (pUpBind)
                        {
                            pKeyBinds->UserRemoveCommandBoundKey(pUpBind);
                        }
                        pKeyBinds->UserRemoveCommandBoundKey(pBind);
                    }
                }
                // If this key bind didn't exist, create it
                else if (pSecKeys[k])
                {
                    if (strResource.empty())
                        pKeyBinds->AddCommand(pSecKeys[k], szCommand, szArguments);
                    else
                        pKeyBinds->AddCommand(pSecKeys[k]->szKey, szCommand, szArguments, true, strResource.c_str());

                    // Also add a matching "up" state if applicable
                    CCommandBind* pUpBind = pKeyBinds->GetBindFromCommand(szCommand, NULL, true, pPriKey->szKey, true, false);
                    if (pUpBind)
                    {
                        const char* szResource = pUpBind->resource.empty() ? nullptr : pUpBind->resource.c_str();
                        pKeyBinds->AddCommand(pSecKeys[k]->szKey, szCommand, pUpBind->arguments.c_str(), false, szResource);
                    }
                }
            }
        }
        else
        {
            assert(0);
        }
    }
}

bool CSettings::OnBindsListClick(CGUIElement* pElement)
{
    CGUIListItem* pItem = m_pBindsList->GetSelectedItem();
    if (pItem)
    {
        CGUIListItem* pItemBind = m_pBindsList->GetItem(m_pBindsList->GetItemRowIndex(pItem), m_hBind);

        // Proceed if the user didn't select the "Bind"-column
        if (pItem != pItemBind)
        {
            m_uiCaptureKey = 0;
            m_pSelectedBind = pItem;
            m_bCaptureKey = true;

            // Determine if the primary or secondary column was selected
            if (m_pBindsList->GetItemColumnIndex(pItem) == 1 /*m_hPriKey  Note: handle is not the same as index */)
            {
                // Create a messagebox to notify the user
                // SString strText = SString::Printf ( "Press a key to bind to '%s'", pItemBind->GetText ().c_str () );
                SString strText = _("Press a key to bind, or escape to clear");
                CCore::GetSingleton().ShowMessageBox(_("Binding a primary key"), strText, MB_ICON_QUESTION);
            }
            else
            {
                // Create a messagebox to notify the user
                // sSString strText = SString::Printf ( "Press a key to bind to '%s'", pItemBind->GetText ().c_str () );
                SString strText = _("Press a key to bind, or escape to clear");
                CCore::GetSingleton().ShowMessageBox(_("Binding a secondary key"), strText, MB_ICON_QUESTION);
            }
        }
    }

    return true;
}

#ifndef WM_XBUTTONDOWN
#define WM_XBUTTONDOWN 0x020B
#endif
#ifndef WM_XBUTTONUP
#define WM_XBUTTONUP 0x020C
#endif

bool CSettings::ProcessMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (m_bCaptureKey)
    {
        CKeyBindsInterface* pKeyBinds = CCore::GetSingleton().GetKeyBinds();

        if (uMsg != WM_KEYDOWN && uMsg != WM_KEYUP && uMsg != WM_SYSKEYDOWN && uMsg != WM_SYSKEYUP && uMsg != WM_LBUTTONDOWN && uMsg != WM_LBUTTONUP &&
            uMsg != WM_RBUTTONDOWN && uMsg != WM_RBUTTONUP && uMsg != WM_MBUTTONDOWN && uMsg != WM_MBUTTONUP && uMsg != WM_XBUTTONDOWN &&
            uMsg != WM_XBUTTONUP && uMsg != WM_MOUSEWHEEL)
        {
            return false;
        }

        // Ignore the first key, which is the one that initiated the capture process
        if (m_uiCaptureKey == 0)
        {
            m_uiCaptureKey++;
            return false;
        }

        bool                bState;
        const SBindableKey* pKey = pKeyBinds->GetBindableFromMessage(uMsg, wParam, lParam, bState);

        // If escape was pressed, don't create a bind
        if (uMsg == WM_KEYDOWN && wParam == VK_ESCAPE)
        {
            m_pSelectedBind->SetText(CORE_SETTINGS_NO_KEY);
        }
        else if (pKey)
        {
            m_pSelectedBind->SetText(pKey->szKey);
        }
        else
        {
            return false;
        }

        // Remove the messagebox we created earlier
        CCore::GetSingleton().RemoveMessageBox();

        // Make sure the list gets redrawed/updated
        m_pBindsList->Activate();

        m_bCaptureKey = false;
        return true;
    }

    return false;
}

void CSettings::Initialize()
{
    using KeyBindPtr = CKeyBindsInterface::KeyBindPtr;

    // Add binds and sections
    bool bPrimaryKey = true;
    int  iBind = 0, iRowGame;

    // Add the rows
    CKeyBinds* pKeyBinds = reinterpret_cast<CKeyBinds*>(CCore::GetSingleton().GetKeyBinds());
    iRowGame = m_pBindsList->AddRow();
    m_pBindsList->SetItemText(iRowGame, m_hBind, _("GTA GAME CONTROLS"), false, true);
    m_pBindsList->SetItemText(m_pBindsList->AddRow(), m_hBind, CORE_SETTINGS_HEADER_SPACER, false, true);
    m_pBindsList->SetItemText(m_pBindsList->AddRow(), m_hBind, _("MULTIPLAYER CONTROLS"), false, true);
    // iRows = CORE_SETTINGS_HEADERS;            // (game keys), (multiplayer keys)
    int iGameRowCount = 1;
    int iMultiplayerRowCount = 2;

    list<SKeyBindSection*>::const_iterator iters = m_pKeyBindSections.begin();
    for (; iters != m_pKeyBindSections.end(); iters++)
    {
        (*iters)->currentIndex = -1;
        (*iters)->rowCount = 0;
    }

    pKeyBinds->SortCommandBinds();

    // Loop through all the available controls
    int i = 0;

    for (; *g_bcControls[i].szControl != NULL; i++)
    {
    }

    for (i--; i >= 0; i--)
    {
        SBindableGTAControl* pControl = &g_bcControls[i];

        // Loop through the binds for a matching control
        size_t numMatches = 0;

        for (KeyBindPtr& bind : *pKeyBinds)
        {
            if (bind->isBeingDeleted || bind->type != KeyBindType::GTA_CONTROL)
                continue;

            auto controlBind = reinterpret_cast<CGTAControlBind*>(bind.get());

            if (controlBind->control != pControl)
                continue;

            if (!numMatches)            // Primary key
            {
                // Add bind to the list
                iBind = m_pBindsList->InsertRowAfter(iRowGame);
                m_pBindsList->SetItemText(iBind, m_hBind, _(pControl->szDescription));
                m_pBindsList->SetItemText(iBind, m_hPriKey, controlBind->boundKey->szKey);
                for (int k = 0; k < SecKeyNum; k++)
                    m_pBindsList->SetItemText(iBind, m_hSecKeys[k], CORE_SETTINGS_NO_KEY);
                m_pBindsList->SetItemData(iBind, m_hBind, reinterpret_cast<void*>(KeyBindType::GTA_CONTROL));
                m_pBindsList->SetItemData(iBind, m_hPriKey, controlBind);
                iGameRowCount++;
            }
            else            // Secondary key
            {
                for (size_t k = 0; k < SecKeyNum; k++)
                {
                    if (numMatches == k + 1)
                    {
                        m_pBindsList->SetItemText(iBind, m_hSecKeys[k], controlBind->boundKey->szKey);
                        m_pBindsList->SetItemData(iBind, m_hSecKeys[k], controlBind);
                    }
                }
            }

            ++numMatches;
        }

        // If we didnt find any matches
        if (!numMatches)
        {
            iBind = m_pBindsList->InsertRowAfter(iRowGame);
            m_pBindsList->SetItemText(iBind, m_hBind, _(pControl->szDescription));
            m_pBindsList->SetItemText(iBind, m_hPriKey, CORE_SETTINGS_NO_KEY);
            for (int k = 0; k < SecKeyNum; k++)
                m_pBindsList->SetItemText(iBind, m_hSecKeys[k], CORE_SETTINGS_NO_KEY);
            m_pBindsList->SetItemData(iBind, m_hBind, reinterpret_cast<void*>(KeyBindType::UNDEFINED));
            m_pBindsList->SetItemData(iBind, m_hPriKey, pControl);
            iGameRowCount++;
        }
    }

    struct SListedCommand
    {
        int           iIndex;
        CCommandBind* pBind;
        unsigned int  uiMatchCount;
    };

    auto         listedCommands = std::make_unique<SListedCommand[]>(pKeyBinds->Count(KeyBindType::COMMAND) + pKeyBinds->Count(KeyBindType::FUNCTION));
    unsigned int uiNumListedCommands = 0;

    std::map<std::string, int> iResourceItems;

    for (KeyBindPtr& bind : *pKeyBinds)
    {
        // keys bound to a console command or a function (we don't show keys bound
        // from gta controls by scripts as these are clearly not user editable)
        if (bind->isBeingDeleted || bind->type != KeyBindType::COMMAND)
            continue;

        auto commandBind = reinterpret_cast<CCommandBind*>(bind.get());

        if (!commandBind->triggerState)
            continue;

        bool foundMatches = false;

        // Loop through the already listed array of commands for matches
        for (unsigned int i = 0; i < uiNumListedCommands; i++)
        {
            SListedCommand* pListedCommand = &listedCommands[i];
            CCommandBind*   pListedBind = pListedCommand->pBind;

            if (pListedBind->command == commandBind->command)
            {
                if (pListedBind->arguments.empty() || pListedBind->arguments == commandBind->arguments)
                {
                    // If we found a 1st match, add it to the secondary section
                    foundMatches = true;

                    for (int k = 0; k < SecKeyNum; k++)
                    {
                        if (pListedCommand->uiMatchCount == k)
                        {
                            m_pBindsList->SetItemText(pListedCommand->iIndex, m_hSecKeys[k], commandBind->boundKey->szKey);
                            m_pBindsList->SetItemData(pListedCommand->iIndex, m_hSecKeys[k], commandBind);
                        }
                    }

                    pListedCommand->uiMatchCount++;
                }
            }
        }

        // If there weren't any matches
        if (!foundMatches)
        {
            unsigned int row = iGameRowCount + 1;

            // Combine command and arguments
            SString strDescription;
            bool    bSkip = false;

            if (!commandBind->resource.empty())
            {
                if (commandBind->isActive)
                {
                    const std::string& resource = commandBind->resource;

                    if (iResourceItems.count(resource) == 0)
                    {
                        iBind = m_pBindsList->AddRow(true);
                        m_pBindsList->SetItemText(iBind, m_hBind, CORE_SETTINGS_HEADER_SPACER, false, true);

                        iBind = m_pBindsList->AddRow(true);
                        m_pBindsList->SetItemText(iBind, m_hBind, resource.c_str(), false, true);
                        m_pBindsList->SetItemData(iBind, m_hBind, reinterpret_cast<void*>(255));
                        iResourceItems.insert(make_pair(resource, iBind));
                    }

                    row = iResourceItems[resource];
                    iMultiplayerRowCount++;
                }
                else
                    continue;
            }

            if (!commandBind->arguments.empty())
            {
                strDescription.Format("%s: %s", commandBind->command.c_str(), commandBind->arguments.c_str());
                iMultiplayerRowCount++;
            }
            else
            {
                strDescription = commandBind->command;
                iMultiplayerRowCount++;
            }

            if (!bSkip)
            {
                // Add the bind to the list
                iBind = m_pBindsList->AddRow(true);
                m_pBindsList->SetItemText(iBind, m_hBind, strDescription);
                m_pBindsList->SetItemText(iBind, m_hPriKey, commandBind->boundKey->szKey);
                for (int k = 0; k < SecKeyNum; k++)
                    m_pBindsList->SetItemText(iBind, m_hSecKeys[k], CORE_SETTINGS_NO_KEY);
                m_pBindsList->SetItemData(iBind, m_hBind, reinterpret_cast<void*>(KeyBindType::COMMAND));
                m_pBindsList->SetItemData(iBind, m_hPriKey, commandBind);

                // Add it to the already-listed array
                SListedCommand* pListedCommand = &listedCommands[uiNumListedCommands];
                pListedCommand->iIndex = iBind;
                pListedCommand->pBind = commandBind;
                pListedCommand->uiMatchCount = 0;
                uiNumListedCommands++;
            }
        }
    }
}

void CSettings::SetVisible(bool bVisible)
{
    // Hide / show the form
    m_pWindow->SetVisible(bVisible);

    // Load the config file if the dialog is shown
    if (bVisible)
    {
#ifdef MTA_DEBUG
        if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0)
            CreateGUI();            // Recreate GUI (for adjusting layout with edit and continue)
#endif
        m_pWindow->BringToFront();
        m_pWindow->Activate();
        LoadData();

        // Clear the binds list
        m_pBindsList->Clear();

        // Re-initialize the binds list
        Initialize();
    }

    m_pWindow->SetZOrderingEnabled(!bVisible);            // Message boxes dont appear on top otherwise
}

bool CSettings::IsVisible()
{
    return m_pWindow->IsVisible();
}

void CSettings::SetIsModLoaded(bool bLoaded)
{
    m_bIsModLoaded = bLoaded;
}

bool CSettings::OnOKButtonClick(CGUIElement* pElement)
{
    CMainMenu* pMainMenu = CLocalGUI::GetSingleton().GetMainMenu();

    // Process keybinds
    ProcessKeyBinds();

    // Save the config
    SaveData();

    // Close the window
    m_pWindow->SetVisible(false);
    pMainMenu->m_bIsInSubWindow = false;

    return true;
}

bool CSettings::OnCancelButtonClick(CGUIElement* pElement)
{
    CMainMenu* pMainMenu = CLocalGUI::GetSingleton().GetMainMenu();

    m_pWindow->SetVisible(false);
    pMainMenu->m_bIsInSubWindow = false;

    // restore old audio volume settings
    CVARS_SET("mastervolume", m_fOldMasterVolume);
    SetRadioVolume(m_fOldRadioVolume);
    SetSFXVolume(m_fOldSFXVolume);
    CVARS_SET("mtavolume", m_fOldMTAVolume);
    CVARS_SET("voicevolume", m_fOldVoiceVolume);

    // restore old audio mute settings
    CVARS_SET("mute_master_when_minimized", m_bOldMuteMaster);
    CVARS_SET("mute_radio_when_minimized", m_bOldMuteRadio);
    CVARS_SET("mute_sfx_when_minimized", m_bOldMuteSFX);
    CVARS_SET("mute_mta_when_minimized", m_bOldMuteMTA);
    CVARS_SET("mute_voice_when_minimized", m_bOldMuteVoice);

    m_pCheckBoxMuteMaster->SetSelected(m_bOldMuteMaster);
    m_pCheckBoxMuteRadio->SetSelected(m_bOldMuteRadio);
    m_pCheckBoxMuteSFX->SetSelected(m_bOldMuteSFX);
    m_pCheckBoxMuteMTA->SetSelected(m_bOldMuteMTA);
    m_pCheckBoxMuteVoice->SetSelected(m_bOldMuteVoice);

    m_pCheckBoxMuteRadio->SetEnabled(!m_bOldMuteMaster);
    m_pCheckBoxMuteSFX->SetEnabled(!m_bOldMuteMaster);
    m_pCheckBoxMuteMTA->SetEnabled(!m_bOldMuteMaster);
    m_pCheckBoxMuteVoice->SetEnabled(!m_bOldMuteMaster);

    return true;
}

void CSettings::LoadData()
{
    // Ensure CVARS ranges ok
    CClientVariables::GetSingleton().ValidateValues();

    std::string strVar;
    bool        bVar;

    // Controls
    CVARS_GET("invert_mouse", bVar);
    m_pInvertMouse->SetSelected(bVar);
    CVARS_GET("steer_with_mouse", bVar);
    m_pSteerWithMouse->SetSelected(bVar);
    CVARS_GET("fly_with_mouse", bVar);
    m_pFlyWithMouse->SetSelected(bVar);

    CGameSettings*            gameSettings = CCore::GetSingleton().GetGame()->GetSettings();
    CControllerConfigManager* pController = g_pCore->GetGame()->GetControllerConfigManager();

    m_pMouseSensitivity->SetScrollPosition(gameSettings->GetMouseSensitivity());
    pController->SetVerticalAimSensitivityRawValue(CVARS_GET_VALUE<float>("vertical_aim_sensitivity"));
    m_pVerticalAimSensitivity->SetScrollPosition(pController->GetVerticalAimSensitivity());

    // Audio
    m_pCheckBoxAudioEqualizer->SetSelected(gameSettings->IsRadioEqualizerEnabled());
    m_pCheckBoxAudioAutotune->SetSelected(gameSettings->IsRadioAutotuneEnabled());
    m_pCheckBoxUserAutoscan->SetSelected(gameSettings->IsUsertrackAutoScan());

    CVARS_GET("mute_master_when_minimized", m_bOldMuteMaster);
    CVARS_GET("mute_radio_when_minimized", m_bOldMuteRadio);
    CVARS_GET("mute_sfx_when_minimized", m_bOldMuteSFX);
    CVARS_GET("mute_mta_when_minimized", m_bOldMuteMTA);
    CVARS_GET("mute_voice_when_minimized", m_bOldMuteVoice);

    m_pCheckBoxMuteMaster->SetSelected(m_bOldMuteMaster);
    m_pCheckBoxMuteRadio->SetSelected(m_bOldMuteRadio);
    m_pCheckBoxMuteSFX->SetSelected(m_bOldMuteSFX);
    m_pCheckBoxMuteMTA->SetSelected(m_bOldMuteMTA);
    m_pCheckBoxMuteVoice->SetSelected(m_bOldMuteVoice);

    m_pCheckBoxMuteRadio->SetEnabled(!m_bOldMuteMaster);
    m_pCheckBoxMuteSFX->SetEnabled(!m_bOldMuteMaster);
    m_pCheckBoxMuteMTA->SetEnabled(!m_bOldMuteMaster);
    m_pCheckBoxMuteVoice->SetEnabled(!m_bOldMuteMaster);

    unsigned int uiUsertrackMode = gameSettings->GetUsertrackMode();
    if (uiUsertrackMode == 0)
        m_pComboUsertrackMode->SetText(_("Radio"));
    else if (uiUsertrackMode == 1)
        m_pComboUsertrackMode->SetText(_("Random"));
    else if (uiUsertrackMode == 2)
        m_pComboUsertrackMode->SetText(_("Sequential"));

    CVARS_GET("mastervolume", m_fOldMasterVolume);
    CVARS_GET("mtavolume", m_fOldMTAVolume);
    CVARS_GET("voicevolume", m_fOldVoiceVolume);
    m_fOldMasterVolume = max(0.0f, min(1.0f, m_fOldMasterVolume));
    m_fOldRadioVolume = max(0.0f, min(1.0f, m_fRadioVolume));
    m_fOldSFXVolume = max(0.0f, min(1.0f, m_fSFXVolume));
    m_fOldMTAVolume = max(0.0f, min(1.0f, m_fOldMTAVolume));
    m_fOldVoiceVolume = max(0.0f, min(1.0f, m_fOldVoiceVolume));
    m_pAudioMasterVolume->SetScrollPosition(m_fOldMasterVolume);
    m_pAudioRadioVolume->SetScrollPosition(m_fOldRadioVolume);
    m_pAudioSFXVolume->SetScrollPosition(m_fOldSFXVolume);
    m_pAudioMTAVolume->SetScrollPosition(m_fOldMTAVolume);
    m_pAudioVoiceVolume->SetScrollPosition(m_fOldVoiceVolume);

    UpdateVideoTab();
}

bool CSettings::OnTabChanged(CGUIElement* pElement)
{
    return true;
}

void CSettings::SaveData()
{
    std::string    strVar;
    CGameSettings* gameSettings = CCore::GetSingleton().GetGame()->GetSettings();

    // Very hacky
    CControllerConfigManager* pController = g_pCore->GetGame()->GetControllerConfigManager();
    CVARS_SET("invert_mouse", m_pInvertMouse->GetSelected());
    pController->SetMouseInverted(m_pInvertMouse->GetSelected());
    CVARS_SET("steer_with_mouse", m_pSteerWithMouse->GetSelected());
    pController->SetSteerWithMouse(m_pSteerWithMouse->GetSelected());
    CVARS_SET("fly_with_mouse", m_pFlyWithMouse->GetSelected());
    pController->SetFlyWithMouse(m_pFlyWithMouse->GetSelected());
    pController->SetVerticalAimSensitivity(m_pVerticalAimSensitivity->GetScrollPosition());
    CVARS_SET("vertical_aim_sensitivity", pController->GetVerticalAimSensitivityRawValue());

    // Video
    // get current
    int  iNextVidMode;
    bool bNextWindowed;
    bool bNextFSMinimize;
    int  iNextFullscreenStyle;
    GetVideoModeManager()->GetNextVideoMode(iNextVidMode, bNextWindowed, bNextFSMinimize, iNextFullscreenStyle);
    int iAntiAliasing = gameSettings->GetAntiAliasing();

    // update from gui
    bNextWindowed = m_pCheckBoxWindowed->GetSelected();
    if (CGUIListItem* pSelected = m_pComboResolution->GetSelectedItem())
        iNextVidMode = (int)pSelected->GetData();
    bNextFSMinimize = m_pCheckBoxMinimize->GetSelected();
    if (CGUIListItem* pSelected = m_pFullscreenStyleCombo->GetSelectedItem())
        iNextFullscreenStyle = (int)pSelected->GetData();
    if (CGUIListItem* pSelected = m_pComboAntiAliasing->GetSelectedItem())
        iAntiAliasing = (int)pSelected->GetData();

    // change
    bool bIsVideoModeChanged = GetVideoModeManager()->SetVideoMode(iNextVidMode, bNextWindowed, bNextFSMinimize, iNextFullscreenStyle);
    bool bIsAntiAliasingChanged = gameSettings->GetAntiAliasing() != iAntiAliasing;

    gameSettings->SetAntiAliasing(iAntiAliasing, true);
    gameSettings->SetDrawDistance((m_pDrawDistance->GetScrollPosition() * 0.875f) + 0.925f);
    gameSettings->SetBrightness(m_pBrightness->GetScrollPosition() * 384);
    gameSettings->SetMouseSensitivity(m_pMouseSensitivity->GetScrollPosition());
    gameSettings->SetMipMappingEnabled(m_pCheckBoxMipMapping->GetSelected());

    // Process DPI awareness
    bool previousProcessDPIAware = false;
    CVARS_GET("process_dpi_aware", previousProcessDPIAware);

    const bool processsDPIAwareChanged = (m_pCheckBoxDPIAware->GetSelected() != previousProcessDPIAware);

    if (processsDPIAwareChanged)
        CVARS_SET("process_dpi_aware", !previousProcessDPIAware);

    // iFieldOfView
    int iFieldOfView = std::min<int>(4, (m_pFieldOfView->GetScrollPosition()) * (3)) * 10 + 70;
    CVARS_SET("fov", iFieldOfView);
    gameSettings->UpdateFieldOfViewFromSettings();

    // Anisotropic filtering
    int iAnisotropic = std::min<int>(m_iMaxAnisotropic, (m_pAnisotropic->GetScrollPosition()) * (m_iMaxAnisotropic + 1));
    CVARS_SET("anisotropic", iAnisotropic);

    // Visual FX Quality
    if (CGUIListItem* pQualitySelected = m_pComboFxQuality->GetSelectedItem())
    {
        gameSettings->SetFXQuality((int)pQualitySelected->GetData());
    }

    // Aspect ratio
    if (CGUIListItem* pRatioSelected = m_pComboAspectRatio->GetSelectedItem())
    {
        eAspectRatio aspectRatio = (eAspectRatio)(int)pRatioSelected->GetData();
        CVARS_SET("aspect_ratio", aspectRatio);
    }

    // HUD match aspect ratio
    bool bHudMatchAspectRatio = m_pCheckBoxHudMatchAspectRatio->GetSelected();
    CVARS_SET("hud_match_aspect_ratio", bHudMatchAspectRatio);
    gameSettings->SetAspectRatio((eAspectRatio)CVARS_GET_VALUE<int>("aspect_ratio"), bHudMatchAspectRatio);

    // Volumetric shadows
    bool bVolumetricShadowsEnabled = m_pCheckBoxVolumetricShadows->GetSelected();
    CVARS_SET("volumetric_shadows", bVolumetricShadowsEnabled);
    gameSettings->SetVolumetricShadowsEnabled(bVolumetricShadowsEnabled);

    // Device selection dialog
    bool bDeviceSelectionDialogEnabled = m_pCheckBoxDeviceSelectionDialog->GetSelected();
    SetApplicationSettingInt("device-selection-disabled", bDeviceSelectionDialogEnabled ? 0 : 1);

    // Show unsafe resolutions
    bool bShowUnsafeResolutions = m_pCheckBoxShowUnsafeResolutions->GetSelected();
    CVARS_SET("show_unsafe_resolutions", bShowUnsafeResolutions);

    // Grass
    bool bGrassEnabled = m_pCheckBoxGrass->GetSelected();
    CVARS_SET("grass", bGrassEnabled);
    gameSettings->SetGrassEnabled(bGrassEnabled);

    // Heat haze
    bool bHeatHazeEnabled = m_pCheckBoxHeatHaze->GetSelected();
    CVARS_SET("heat_haze", bHeatHazeEnabled);
    g_pCore->GetMultiplayer()->SetHeatHazeEnabled(bHeatHazeEnabled);

    // Tyre smoke particles
    bool bTyreSmokeEnabled = m_pCheckBoxTyreSmokeParticles->GetSelected();
    CVARS_SET("tyre_smoke_enabled", bTyreSmokeEnabled);
    g_pCore->GetMultiplayer()->SetTyreSmokeEnabled(bTyreSmokeEnabled);

    // High detail vehicles
    bool bHighDetailVehicles = m_pCheckBoxHighDetailVehicles->GetSelected();
    CVARS_SET("high_detail_vehicles", bHighDetailVehicles);
    gameSettings->ResetVehiclesLODDistance(false);

    // High detail peds
    bool bHighDetailPeds = m_pCheckBoxHighDetailPeds->GetSelected();
    CVARS_SET("high_detail_peds", bHighDetailPeds);
    gameSettings->ResetPedsLODDistance(false);

    // Blur
    bool bBlur = m_pCheckBoxBlur->GetSelected();
    CVARS_SET("blur", bBlur);
    gameSettings->ResetBlurEnabled();

    // Corona rain reflections
    bool bCoronaReflections = m_pCheckBoxCoronaReflections->GetSelected();
    CVARS_SET("corona_reflections", bCoronaReflections);
    gameSettings->ResetCoronaReflectionsEnabled();

    // Dynamic ped shadows
    bool bDynamicPedShadows = m_pCheckBoxDynamicPedShadows->GetSelected();
    CVARS_SET("dynamic_ped_shadows", bDynamicPedShadows);
    gameSettings->SetDynamicPedShadowsEnabled(bDynamicPedShadows);

    // Audio
    gameSettings->SetRadioEqualizerEnabled(m_pCheckBoxAudioEqualizer->GetSelected());
    gameSettings->SetRadioAutotuneEnabled(m_pCheckBoxAudioAutotune->GetSelected());
    gameSettings->SetUsertrackAutoScan(m_pCheckBoxUserAutoscan->GetSelected());

    if (CGUIListItem* pSelected = m_pComboUsertrackMode->GetSelectedItem())
    {
        gameSettings->SetUsertrackMode((int)pSelected->GetData());
    }

    // Ensure CVARS ranges ok
    CClientVariables::GetSingleton().ValidateValues();

    // Save the config here
    CCore::GetSingleton().SaveConfig();
    // Save the single player settings (e.g. video mode, volume)
    gameSettings->Save();

    // Ask to restart?
    if (bIsVideoModeChanged || bIsAntiAliasingChanged || processsDPIAwareChanged)
    ShowRestartQuestion();
}

void CSettings::RemoveKeyBindSection(char* szSectionName)
{
    list<SKeyBindSection*>::iterator iter = m_pKeyBindSections.begin();
    for (; iter != m_pKeyBindSections.end(); iter++)
    {
        if (strcmp((*iter)->szOriginalTitle, szSectionName) == 0)
        {
            delete *iter;
            m_pKeyBindSections.erase(iter);
            break;
        }
    }
}

void CSettings::RemoveAllKeyBindSections()
{
    list<SKeyBindSection*>::const_iterator iter = m_pKeyBindSections.begin();
    for (; iter != m_pKeyBindSections.end(); iter++)
    {
        SKeyBindSection* section = (*iter);
        delete section;
    }
    m_pKeyBindSections.clear();
}

void CSettings::AddKeyBindSection(char* szSectionName)
{
    m_pKeyBindSections.push_back(new SKeyBindSection(szSectionName));
}

int CSettings::GetMilliseconds(CGUIEdit* pEdit)
{
    // Note to anyone listening: stringstream does not handle out of range numbers well
    double dValue = strtol(pEdit->GetText().c_str(), NULL, 0);
    dValue *= 1000;
    dValue = Clamp<double>(INT_MIN, dValue, INT_MAX);
    return static_cast<int>(dValue);
}

void CSettings::SetMilliseconds(CGUIEdit* pEdit, int iValue)
{
    stringstream ss;
    ss << (float)iValue / 1000;
    pEdit->SetText(ss.str().c_str());
}

bool CSettings::OnFieldOfViewChanged(CGUIElement* pElement)
{
    int iFieldOfView = std::min<int>(4, (m_pFieldOfView->GetScrollPosition()) * (3)) * 10 + 70;

    m_pFieldOfViewValueLabel->SetText(SString("%i", iFieldOfView).c_str());
    return true;
}

bool CSettings::OnDrawDistanceChanged(CGUIElement* pElement)
{
    int iDrawDistance = (m_pDrawDistance->GetScrollPosition()) * 100;

    m_pDrawDistanceValueLabel->SetText(SString("%i%%", iDrawDistance).c_str());
    return true;
}

bool CSettings::OnBrightnessChanged(CGUIElement* pElement)
{
    int iBrightness = (m_pBrightness->GetScrollPosition()) * 100;

    m_pBrightnessValueLabel->SetText(SString("%i%%", iBrightness).c_str());
    return true;
}

bool CSettings::OnAnisotropicChanged(CGUIElement* pElement)
{
    int iAnisotropic = std::min<int>(m_iMaxAnisotropic, (m_pAnisotropic->GetScrollPosition()) * (m_iMaxAnisotropic + 1));

    SString strLabel;
    if (iAnisotropic > 0)
        strLabel = SString("%ix", 1 << iAnisotropic);
    else
        strLabel = _("Off");

    m_pAnisotropicValueLabel->SetText(strLabel);
    return true;
}

bool CSettings::OnMouseSensitivityChanged(CGUIElement* pElement)
{
    int iMouseSensitivity = (m_pMouseSensitivity->GetScrollPosition()) * 100;

    m_pLabelMouseSensitivityValue->SetText(SString("%i%%", iMouseSensitivity).c_str());
    return true;
}

bool CSettings::OnVerticalAimSensitivityChanged(CGUIElement* pElement)
{
    int iSensitivity = m_pVerticalAimSensitivity->GetScrollPosition() * 100;
    m_pLabelVerticalAimSensitivityValue->SetText(SString("%i%%", iSensitivity));
    return true;
}

bool CSettings::OnMasterVolumeChanged(CGUIElement* pElement)
{
    int iVolume = m_pAudioMasterVolume->GetScrollPosition() * 100.0f;
    m_pLabelMasterVolumeValue->SetText(SString("%i%%", iVolume).c_str());

    CVARS_SET("mastervolume", m_pAudioMasterVolume->GetScrollPosition());

    OnRadioVolumeChanged(nullptr);
    OnSFXVolumeChanged(nullptr);
    OnMTAVolumeChanged(nullptr);
    OnVoiceVolumeChanged(nullptr);

    return true;
}

bool CSettings::OnRadioVolumeChanged(CGUIElement* pElement)
{
    int iVolume = m_pAudioRadioVolume->GetScrollPosition() * 100.0f;
    m_pLabelRadioVolumeValue->SetText(SString("%i%%", iVolume).c_str());

    SetRadioVolume(m_pAudioRadioVolume->GetScrollPosition());

    return true;
}

bool CSettings::OnSFXVolumeChanged(CGUIElement* pElement)
{
    int iVolume = m_pAudioSFXVolume->GetScrollPosition() * 100.0f;
    m_pLabelSFXVolumeValue->SetText(SString("%i%%", iVolume).c_str());

    SetSFXVolume(m_pAudioSFXVolume->GetScrollPosition());

    return true;
}

bool CSettings::OnMTAVolumeChanged(CGUIElement* pElement)
{
    int iVolume = m_pAudioMTAVolume->GetScrollPosition() * 100.0f;
    m_pLabelMTAVolumeValue->SetText(SString("%i%%", iVolume).c_str());

    CVARS_SET("mtavolume", m_pAudioMTAVolume->GetScrollPosition());

    return true;
}

bool CSettings::OnMasterMuteMinimizedChanged(CGUIElement* pElement)
{
    bool bSelected = m_pCheckBoxMuteMaster->GetSelected();
    m_pCheckBoxMuteRadio->SetEnabled(!bSelected);
    m_pCheckBoxMuteSFX->SetEnabled(!bSelected);
    m_pCheckBoxMuteMTA->SetEnabled(!bSelected);
    m_pCheckBoxMuteVoice->SetEnabled(!bSelected);
    CVARS_SET("mute_master_when_minimized", bSelected);
    return true;
}

bool CSettings::OnRadioMuteMinimizedChanged(CGUIElement* pElement)
{
    CVARS_SET("mute_radio_when_minimized", m_pCheckBoxMuteRadio->GetSelected());
    return true;
}

bool CSettings::OnSFXMuteMinimizedChanged(CGUIElement* pElement)
{
    CVARS_SET("mute_sfx_when_minimized", m_pCheckBoxMuteSFX->GetSelected());
    return true;
}

bool CSettings::OnMTAMuteMinimizedChanged(CGUIElement* pElement)
{
    CVARS_SET("mute_mta_when_minimized", m_pCheckBoxMuteMTA->GetSelected());
    return true;
}

bool CSettings::OnVoiceMuteMinimizedChanged(CGUIElement* pElement)
{
    CVARS_SET("mute_voice_when_minimized", m_pCheckBoxMuteVoice->GetSelected());
    return true;
}

bool CSettings::OnVoiceVolumeChanged(CGUIElement* pElement)
{
    int iVolume = m_pAudioVoiceVolume->GetScrollPosition() * 100.0f;
    m_pLabelVoiceVolumeValue->SetText(SString("%i%%", iVolume).c_str());

    CVARS_SET("voicevolume", m_pAudioVoiceVolume->GetScrollPosition());

    return true;
}

bool CSettings::OnFxQualityChanged(CGUIElement* pElement)
{
    CGUIListItem* pItem = m_pComboFxQuality->GetSelectedItem();
    if (!pItem)
        return true;

    if ((int)pItem->GetData() == 0)
    {
        m_pCheckBoxVolumetricShadows->SetSelected(false);
        m_pCheckBoxVolumetricShadows->SetEnabled(false);
        m_pCheckBoxGrass->SetEnabled(false);
    }
    else
    {
        m_pCheckBoxVolumetricShadows->SetEnabled(true);
        m_pCheckBoxGrass->SetEnabled(true);
    }

    // Enable dynamic ped shadows checkbox if visual quality option is set to high or very high
    m_pCheckBoxDynamicPedShadows->SetEnabled((int)pItem->GetData() >= 2);
    return true;
}

void VolumetricShadowsCallBack(void* ptr, unsigned int uiButton)
{
    CCore::GetSingleton().GetLocalGUI()->GetMainMenu()->GetQuestionWindow()->Reset();
    if (uiButton == 0)
        ((CGUICheckBox*)ptr)->SetSelected(false);
}

bool CSettings::OnVolumetricShadowsClick(CGUIElement* pElement)
{
    if (m_pCheckBoxVolumetricShadows->GetSelected() && !m_bShownVolumetricShadowsWarning)
    {
        m_bShownVolumetricShadowsWarning = true;
        SStringX strMessage(
            _("Volmetric shadows can cause some systems to slow down."
              "\n\nAre you sure you want to enable them?"));
        CQuestionBox* pQuestionBox = CCore::GetSingleton().GetLocalGUI()->GetMainMenu()->GetQuestionWindow();
        pQuestionBox->Reset();
        pQuestionBox->SetTitle(_("PERFORMANCE WARNING"));
        pQuestionBox->SetMessage(strMessage);
        pQuestionBox->SetButton(0, _("No"));
        pQuestionBox->SetButton(1, _("Yes"));
        pQuestionBox->SetCallback(VolumetricShadowsCallBack, m_pCheckBoxVolumetricShadows);
        pQuestionBox->Show();
    }
    return true;
}

//
// ShowUnsafeResolutionsClick
//
bool CSettings::ShowUnsafeResolutionsClick(CGUIElement* pElement)
{
    // Change list of available resolutions
    PopulateResolutionComboBox();
    return true;
}

//
// OnWindowedClick
//
bool CSettings::OnWindowedClick(CGUIElement* pElement)
{
    UpdateFullScreenComboBoxEnabled();
    return true;
}

//
// OnDPIAwareClick
//
static void DPIAwareQuestionCallBack(void* userdata, unsigned int uiButton);

bool CSettings::OnDPIAwareClick(CGUIElement* pElement)
{
    static bool shownWarning = false;

    if (m_pCheckBoxDPIAware->GetSelected() && !shownWarning)
    {
        shownWarning = true;

        SStringX strMessage(
            _("Enabling DPI awareness is an experimental feature and\n"
              "we only recommend it when you play MTA:SA on a scaled monitor.\n"
              "You may experience graphical issues if you enable this option."
              "\n\nAre you sure you want to enable this option?"));
        CQuestionBox* pQuestionBox = CCore::GetSingleton().GetLocalGUI()->GetMainMenu()->GetQuestionWindow();
        pQuestionBox->Reset();
        pQuestionBox->SetTitle(_("EXPERIMENTAL FEATURE"));
        pQuestionBox->SetMessage(strMessage);
        pQuestionBox->SetButton(0, _("No"));
        pQuestionBox->SetButton(1, _("Yes"));
        pQuestionBox->SetCallback(DPIAwareQuestionCallBack, m_pCheckBoxDPIAware);
        pQuestionBox->Show();
    }

    return true;
}

static void DPIAwareQuestionCallBack(void* userdata, unsigned int uiButton)
{
    CCore::GetSingleton().GetLocalGUI()->GetMainMenu()->GetQuestionWindow()->Reset();

    if (uiButton == 0)
    {
        auto const checkBox = reinterpret_cast<CGUICheckBox*>(userdata);
        checkBox->SetSelected(false);
    }
}

void CSettings::SetSelectedIndex(unsigned int uiIndex)
{
    unsigned int uiTabCount = m_pTabs->GetTabCount();

    if (uiIndex < uiTabCount)
    {
        m_pTabs->SetSelectedIndex(uiIndex);
    }
}

void CSettings::TabSkip(bool bBackwards)
{
    unsigned int uiTabCount = m_pTabs->GetTabCount();

    if (bBackwards)
    {
        unsigned int uiIndex = m_pTabs->GetSelectedIndex() - 1;

        if (m_pTabs->GetSelectedIndex() == 0)
        {
            uiIndex = uiTabCount - 1;
        }

        SetSelectedIndex(uiIndex);
    }
    else
    {
        unsigned int uiIndex = m_pTabs->GetSelectedIndex() + 1;
        unsigned int uiNewIndex = uiIndex % uiTabCount;

        SetSelectedIndex(uiNewIndex);
    }
}

bool CSettings::IsActive()
{
    return m_pWindow->IsActive();
}
