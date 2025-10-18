/*****************************************************************************
 *
 *  PROJECT:     Black Sun Games
 *  LICENSE:     See LICENSE in the top level directory
 *  FILE:        Client/core/LimitAdjuster/CIMGLimitsSA.h
 *  PURPOSE:     Limits img archives
 *
 *****************************************************************************/

#pragma once

class CIMGLimitsSA
{
public:
    void Update();
    void SetLimitOfIMGarchives(unsigned int numberOfIMGarchives);

    struct tImgDescriptor
    {
        char name[40];
        char isNotPlayerImg;
        char __align[3];
        int streamHandle;
    };

    unsigned int uiNumberOfIMGarchives;
};

static CIMGLimitsSA* m_pIMGLimitsSA;