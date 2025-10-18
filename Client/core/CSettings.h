/*****************************************************************************
 *
 *  PROJECT:     Multi Theft Auto
 *  LICENSE:     See LICENSE in the top level directory
 *  FILE:        core/CSettings.h
 *  PURPOSE:     Header file for in-game settings window class
 *
 *  Multi Theft Auto is available from http://www.multitheftauto.com/
 *
 *****************************************************************************/

class CSettings;

#pragma once

#include <core/CCoreInterface.h>
#include "CMainMenu.h"
#include "CCore.h"

// #define SHOWALLSETTINGS

struct SKeyBindSection
{
    SKeyBindSection(char* szTitle)
    {
        this->currentIndex = 0;
        this->szTitle = new char[strlen(szTitle) + 1];
        this->szOriginalTitle = new char[strlen(szTitle) + 1];
        // convert to upper case
        for (unsigned int i = 0; i < strlen(szTitle); i++)
        {
            if (isalpha((uchar)szTitle[i]))
                this->szTitle[i] = toupper(szTitle[i]);
            else
                this->szTitle[i] = szTitle[i];
        }

        this->szTitle[strlen(szTitle)] = '\0';

        strcpy(szOriginalTitle, szTitle);

        this->rowCount = 0;

        headerItem = NULL;
    }

    ~SKeyBindSection()
    {
        if (this->szTitle)
            delete[] this->szTitle;

        if (szOriginalTitle)
            delete[] szOriginalTitle;
    }
    int                 currentIndex;            // temporarily stores the index while the list is being created
    char*               szTitle;
    char*               szOriginalTitle;
    int                 rowCount;
    class CGUIListItem* headerItem;
};

class CColor;

enum
{
    FULLSCREEN_STANDARD,
    FULLSCREEN_BORDERLESS,
    FULLSCREEN_BORDERLESS_KEEP_RES,
    FULLSCREEN_BORDERLESS_STRETCHED,
};

class CSettings
{
    friend class CCore;

public:
    CSettings();
    ~CSettings();

    void CreateGUI();
    void DestroyGUI();

    bool ProcessMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

    void Update();
    void Initialize();

    void SetVisible(bool bVisible);
    bool IsVisible();

    void SetIsModLoaded(bool bLoaded);

    void LoadData();

    bool IsCapturingKey() { return m_bCaptureKey; }
    void UpdateCaptureAxis();

    void UpdateAudioTab();

    void UpdateVideoTab();
    void PopulateResolutionComboBox();
    void UpdateFullScreenComboBoxEnabled();

    void AddKeyBindSection(char* szSectionName);
    void RemoveKeyBindSection(char* szSectionName);
    void RemoveAllKeyBindSections();

    void ShowRestartQuestion();

    void TabSkip(bool bBackwards);

    bool IsActive();

    void SetSelectedIndex(unsigned int uiIndex);

protected:
    const static int SecKeyNum = 3;            // Number of secondary keys

    // Keep these protected so we can access them in the event handlers of CClientGame
    CGUIElement*  m_pWindow;
    CGUITabPanel* m_pTabs;
    CGUIButton*   m_pButtonOK;
    CGUIButton*   m_pButtonCancel;

    CGUILabel*     m_pVideoGeneralLabel;
    CGUILabel*     m_pVideoResolutionLabel;
    CGUIComboBox*  m_pComboResolution;
    CGUICheckBox*  m_pCheckBoxMipMapping;
    CGUICheckBox*  m_pCheckBoxWindowed;
    CGUICheckBox*  m_pCheckBoxDPIAware = nullptr;
    CGUICheckBox*  m_pCheckBoxHudMatchAspectRatio;
    CGUICheckBox*  m_pCheckBoxMinimize;
    CGUIComboBox*  m_pComboFxQuality;
    CGUILabel*     m_pFXQualityLabel;
    CGUIComboBox*  m_pComboAspectRatio;
    CGUILabel*     m_pAspectRatioLabel;
    CGUICheckBox*  m_pCheckBoxVolumetricShadows;
    CGUICheckBox*  m_pCheckBoxDeviceSelectionDialog;
    CGUICheckBox*  m_pCheckBoxShowUnsafeResolutions;
    CGUICheckBox*  m_pCheckBoxGrass;
    CGUICheckBox*  m_pCheckBoxHeatHaze;
    CGUICheckBox*  m_pCheckBoxTyreSmokeParticles;
    CGUICheckBox*  m_pCheckBoxHighDetailVehicles;
    CGUICheckBox*  m_pCheckBoxHighDetailPeds;
    CGUICheckBox*  m_pCheckBoxBlur;
    CGUICheckBox*  m_pCheckBoxCoronaReflections;
    CGUICheckBox*  m_pCheckBoxDynamicPedShadows;
    CGUILabel*     m_pFieldOfViewLabel;
    CGUIScrollBar* m_pFieldOfView;
    CGUILabel*     m_pFieldOfViewValueLabel;
    CGUILabel*     m_pDrawDistanceLabel;
    CGUIScrollBar* m_pDrawDistance;
    CGUILabel*     m_pDrawDistanceValueLabel;
    CGUILabel*     m_pBrightnessLabel;
    CGUIScrollBar* m_pBrightness;
    CGUILabel*     m_pBrightnessValueLabel;
    CGUILabel*     m_pAnisotropicLabel;
    CGUIScrollBar* m_pAnisotropic;
    CGUILabel*     m_pAnisotropicValueLabel;
    CGUIComboBox*  m_pComboAntiAliasing;
    CGUILabel*     m_pAntiAliasingLabel;
    CGUIButton*    m_pVideoDefButton;

    CGUILabel*     m_pFullscreenStyleLabel;
    CGUIComboBox*  m_pFullscreenStyleCombo;
    CGUILabel*     m_pAudioGeneralLabel;
    CGUILabel*     m_pUserTrackGeneralLabel;
    CGUILabel*     m_pWin8Label;
    CGUICheckBox*  m_pWin8ColorCheckBox;
    CGUICheckBox*  m_pWin8MouseCheckBox;
    CGUILabel*     m_pLabelMasterVolume;
    CGUILabel*     m_pLabelRadioVolume;
    CGUILabel*     m_pLabelSFXVolume;
    CGUILabel*     m_pLabelMTAVolume;
    CGUILabel*     m_pLabelVoiceVolume;
    CGUILabel*     m_pLabelMasterVolumeValue;
    CGUILabel*     m_pLabelRadioVolumeValue;
    CGUILabel*     m_pLabelSFXVolumeValue;
    CGUILabel*     m_pLabelMTAVolumeValue;
    CGUILabel*     m_pLabelVoiceVolumeValue;
    CGUIScrollBar* m_pAudioMasterVolume;
    CGUIScrollBar* m_pAudioRadioVolume;
    CGUIScrollBar* m_pAudioSFXVolume;
    CGUIScrollBar* m_pAudioMTAVolume;
    CGUIScrollBar* m_pAudioVoiceVolume;
    CGUILabel*     m_pAudioRadioLabel;
    CGUICheckBox*  m_pCheckBoxAudioEqualizer;
    CGUICheckBox*  m_pCheckBoxAudioAutotune;
    CGUILabel*     m_pAudioMuteLabel;
    CGUICheckBox*  m_pCheckBoxMuteMaster;
    CGUICheckBox*  m_pCheckBoxMuteSFX;
    CGUICheckBox*  m_pCheckBoxMuteRadio;
    CGUICheckBox*  m_pCheckBoxMuteMTA;
    CGUICheckBox*  m_pCheckBoxMuteVoice;
    CGUILabel*     m_pAudioUsertrackLabel;
    CGUICheckBox*  m_pCheckBoxUserAutoscan;
    CGUILabel*     m_pLabelUserTrackMode;
    CGUIComboBox*  m_pComboUsertrackMode;
    CGUIButton*    m_pAudioDefButton;

    CGUIGridList* m_pBindsList;
    CGUIButton*   m_pBindsDefButton;
    CGUIHandle    m_hBind, m_hPriKey, m_hSecKeys[SecKeyNum];

    CGUIEdit*                m_pEditDeadzone;
    CGUIEdit*                m_pEditSaturation;

    CGUILabel*     m_pControlsMouseLabel;
    CGUICheckBox*  m_pInvertMouse;
    CGUICheckBox*  m_pSteerWithMouse;
    CGUICheckBox*  m_pFlyWithMouse;
    CGUILabel*     m_pLabelMouseSensitivity;
    CGUIScrollBar* m_pMouseSensitivity;
    CGUILabel*     m_pLabelMouseSensitivityValue;
    CGUILabel*     m_pLabelVerticalAimSensitivity;
    CGUIScrollBar* m_pVerticalAimSensitivity;
    CGUILabel*     m_pLabelVerticalAimSensitivityValue;

    CGUIScrollPane*  m_pControlsInputTypePane;
    CGUIRadioButton* m_pStandardControls;
    CGUIRadioButton* m_pClassicControls;

    CGUICheckBox* m_pFlashWindow;
    CGUICheckBox* m_pTrayBalloon;

    bool OnAxisSelectClick(CGUIElement* pElement);
    bool OnAudioDefaultClick(CGUIElement* pElement);
    bool OnControlsDefaultClick(CGUIElement* pElement);
    bool OnBindsDefaultClick(CGUIElement* pElement);
    bool OnVideoDefaultClick(CGUIElement* pElement);
    bool OnBindsListClick(CGUIElement* pElement);
    bool OnOKButtonClick(CGUIElement* pElement);
    bool OnCancelButtonClick(CGUIElement* pElement);
    bool OnFieldOfViewChanged(CGUIElement* pElement);
    bool OnDrawDistanceChanged(CGUIElement* pElement);
    bool OnBrightnessChanged(CGUIElement* pElement);
    bool OnAnisotropicChanged(CGUIElement* pElement);
    bool OnMasterVolumeChanged(CGUIElement* pElement);
    bool OnRadioVolumeChanged(CGUIElement* pElement);
    bool OnSFXVolumeChanged(CGUIElement* pElement);
    bool OnMTAVolumeChanged(CGUIElement* pElement);
    bool OnVoiceVolumeChanged(CGUIElement* pElement);
    bool OnMasterMuteMinimizedChanged(CGUIElement* pElement);
    bool OnRadioMuteMinimizedChanged(CGUIElement* pElement);
    bool OnSFXMuteMinimizedChanged(CGUIElement* pElement);
    bool OnMTAMuteMinimizedChanged(CGUIElement* pElement);
    bool OnVoiceMuteMinimizedChanged(CGUIElement* pElement);
    bool OnMouseSensitivityChanged(CGUIElement* pElement);
    bool OnVerticalAimSensitivityChanged(CGUIElement* pElement);

    bool OnMouseDoubleClick(CGUIMouseEventArgs Args);

    bool OnFxQualityChanged(CGUIElement* pElement);
    bool OnVolumetricShadowsClick(CGUIElement* pElement);
    bool ShowUnsafeResolutionsClick(CGUIElement* pElement);
    bool OnWindowedClick(CGUIElement* pElement);
    bool OnDPIAwareClick(CGUIElement* pElement);
    bool OnTabChanged(CGUIElement* pElement);

private:
    void ProcessKeyBinds();

    void SaveData();

    int    GetMilliseconds(CGUIEdit* pEdit);
    void   SetMilliseconds(CGUIEdit* pEdit, int milliseconds);

    void ResetGTAVolume();
    void SetRadioVolume(float fVolume);
    void SetSFXVolume(float fVolume);

    unsigned int m_uiCaptureKey;
    bool         m_bCaptureKey;
    bool         m_bCaptureAxis;

    bool m_bIsModLoaded;

    float m_fRadioVolume;
    float m_fSFXVolume;

    float m_fOldMasterVolume;
    float m_fOldRadioVolume;
    float m_fOldSFXVolume;
    float m_fOldMTAVolume;
    float m_fOldVoiceVolume;

    bool m_bOldMuteMaster;
    bool m_bOldMuteSFX;
    bool m_bOldMuteRadio;
    bool m_bOldMuteMTA;
    bool m_bOldMuteVoice;

    bool m_bMuteMaster;
    bool m_bMuteSFX;
    bool m_bMuteRadio;
    bool m_bMuteMTA;
    bool m_bMuteVoice;

    CGUIListItem* m_pSelectedBind;

    DWORD m_dwFrameCount;
    bool  m_bShownVolumetricShadowsWarning;
    bool  m_bShownAllowScreenUploadMessage;
    bool  m_bShownAllowExternalSoundsMessage;
    int   m_iMaxAnisotropic;

    std::list<SKeyBindSection*> m_pKeyBindSections;
};
