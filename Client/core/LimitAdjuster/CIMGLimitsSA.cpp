/*****************************************************************************
 *
 *  PROJECT:     Black Sun Games
 *  LICENSE:     See LICENSE in the top level directory
 *  FILE:        Client/core/LimitAdjuster/CIMGLimitsSA.cpp
 *  PURPOSE:     Limits img archives
 *
 *****************************************************************************/

#pragma once

#include <StdInc.h>
#include "CIMGLimitsSA.h"
#include <filesystem>

namespace fs = std::filesystem;

template <typename T>
void PatchInstall(INT_PTR address, T value, DWORD size = sizeof(T))
{
    DWORD protect[2];
    VirtualProtect((LPVOID)address, size, PAGE_EXECUTE_READWRITE, &protect[0]);
    memcpy((void*)address, &value, size);
    VirtualProtect((LPVOID)address, size, protect[0], &protect[1]);
}

void CIMGLimitsSA::Update()
{
    SetLimitOfIMGarchives(15 + 1);
}


void CIMGLimitsSA::SetLimitOfIMGarchives(unsigned int numberOfIMGarchives)
{
    //WriteDebugEvent(xorstr_("Load hooks values 1"));

    tImgDescriptor* ms_files = new tImgDescriptor[numberOfIMGarchives];

    // CStreaming__InitImageList
    PatchInstall(0x4083C0 + 1, &ms_files->streamHandle);
    PatchInstall(0x4083DD + 1, &ms_files[numberOfIMGarchives].streamHandle);
    PatchInstall(0x408459 + 2, &ms_files->streamHandle);
    PatchInstall(0x4084A0 + 2, &ms_files->streamHandle);
    PatchInstall(0x4083E8 + 1, ms_files);
    PatchInstall(0x4083F9 + 1, &ms_files[numberOfIMGarchives]);
    PatchInstall(0x40840A + 1, ms_files);
    PatchInstall(0X408419 + 1, &ms_files[numberOfIMGarchives]);
    PatchInstall(0x408439 + 2, ms_files);
    PatchInstall(0x408477 + 2, ms_files);

    // CStreaming__LoadCdDirectory
    PatchInstall(0x5B82F0 + 1, ms_files);
    PatchInstall(0x5B82FC + 1, ms_files);
    PatchInstall(0x5B8301 + 2, &ms_files[numberOfIMGarchives]);

    // CStreaming__AddImageToList_HOODLUM
    PatchInstall(0x1567B93 + 1, ms_files);
    PatchInstall(0x1567BA1 + 1, &ms_files[numberOfIMGarchives]);
    PatchInstall(0x1567BB8 + 2, ms_files);
    PatchInstall(0x1567BD4 + 2, &ms_files->streamHandle);
    PatchInstall(0x1567BE1 + 2, &ms_files->isNotPlayerImg);

    // CStreaming__InitImageList
    PatchInstall(0x40845F + 2, &ms_files->isNotPlayerImg);
    PatchInstall(0x4084A6 + 2, &ms_files->isNotPlayerImg);

    // CStreaming::GetArchiveStreamHandle
    PatchInstall(0x40757D + 2, &ms_files->streamHandle);

    // CStreaming__GetNextFileOnCd
    PatchInstall(0x408FDA + 2, &ms_files->streamHandle);

    // sub_409D10
    PatchInstall(0x409D58 + 2, &ms_files->streamHandle);

    // CStreaming__RequestModelStream
    PatchInstall(0x40CC52 + 2, &ms_files->streamHandle);
    PatchInstall(0x40CCC5 + 2, &ms_files->streamHandle);

    // CStreamingInfo__GetCdPosnAndSize_HOODLUM
    PatchInstall(0x1560E66 + 2, &ms_files->streamHandle);

    // CStreaming__InitObjectInfoAndRequestModel_HOODLUM
    PatchInstall(0x15663E5 + 2, &ms_files->streamHandle);
}
