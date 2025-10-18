/*****************************************************************************
 *
 *  PROJECT:     Black Sun Games
 *  LICENSE:     See LICENSE in the top level directory
 *  FILE:        Client/core/LimitAdjuster/CLimitAdjusterSA.cpp
 *  PURPOSE:     Limit Adjuster SA
 *
 *****************************************************************************/

#pragma once

#include <StdInc.h>
#include "CLimitAdjusterSA.h"
#include "CIMGLimitsSA.h"

void CLimitAdjusterSA::Update(unsigned short usVehicle, unsigned short usPed, unsigned char ucIMGLimit)
{
//	WriteDebugEvent(xorstr_("Load hooks values 4"));

	IdeLimits(usVehicle, usPed);
	m_pIMGLimitsSA->SetLimitOfIMGarchives(ucIMGLimit + 1);
}

template <typename T>
void PatchInstall(INT_PTR address, T value, DWORD size = sizeof(T))
{
	DWORD protect[2];
	VirtualProtect((LPVOID)address, size, PAGE_EXECUTE_READWRITE, &protect[0]);
	memcpy((void*)address, &value, size);
	VirtualProtect((LPVOID)address, size, protect[0], &protect[1]);
}

class CEntryExit
{
public:
	char unk[0x3C];
};

static_assert(sizeof(CEntryExit) == 0x3C, "Invalid size for CEntryExit");

void SetEntryExits(unsigned int iEntryExits)
{
	PatchInstall(0x156A797 + 1, iEntryExits);
	PatchInstall(0x5D5A5F + 2, iEntryExits * sizeof(CEntryExit));
}

void SetRwObjectInstances(unsigned int iRwObjectInstances)
{
	const int elementSize = 0xC;

	PatchInstall(0x5B8E54 + 1, iRwObjectInstances * elementSize);
	PatchInstall(0x5B8EAF + 1, iRwObjectInstances * elementSize);
}

struct {
	void*** ptr;
	// DEFINE_IS_ALLOCATED();
} IPLentityIndexArray;

void SetEntityIndexArray(unsigned int iEntityIndexArrayLimit)
{
	IPLentityIndexArray.ptr = new void**[iEntityIndexArrayLimit];

	PatchInstall(0x405C3A + 3, IPLentityIndexArray.ptr);	// mov     eax, ds:dword_8E3F08[edx*4]	; sub_405C00
	PatchInstall(0x406198 + 3, IPLentityIndexArray.ptr);	// mov     edx, ds:dword_8E3F08[ecx*4]	; loadIPLfromIMGarchive
	PatchInstall(0x156115D + 3, IPLentityIndexArray.ptr);	// mov     edx, ds:dword_8E3F08[esi*4]	; sub_15610B0
	PatchInstall(0x15649F7 + 3, IPLentityIndexArray.ptr);	// mov     ds:dword_8E3F08[ecx*4], eax	; sub_404780
	PatchInstall(0x1569774 + 3, IPLentityIndexArray.ptr);	// mov     eax, ds:dword_8E3F08[eax*4]	; sub_1569770
}

void SetIDEobjsType1(unsigned int iObjectsType1)
{
	struct CStore
	{
		uint count;
		char array;
	};

	CStore* IDE_objsType1_store = (CStore*)new char[sizeof(uint32_t) + iObjectsType1 * 0x20];

	// Data references to count of CStore (.data:00AAE950)
	PatchInstall(0x4C63E0 + 1, &IDE_objsType1_store->count);	// mov     eax, ds:dword_AAE950
	PatchInstall(0x4C63FD + 1, &IDE_objsType1_store->count);	// mov     eax, ds:dword_AAE950
	PatchInstall(0x4C65D8 + 2, &IDE_objsType1_store->count);	// mov     ds:dword_AAE950, ebp
	PatchInstall(0x4C6620 + 1, &IDE_objsType1_store->count);	// mov     eax, ds:dword_AAE950
	PatchInstall(0x4C6632 + 1, &IDE_objsType1_store->count);	// mov     ds:dword_AAE950, eax
	PatchInstall(0x4C6863 + 2, &IDE_objsType1_store->count);	// mov     ds:dword_AAE950, 1
	PatchInstall(0x4C6899 + 1, &IDE_objsType1_store->count);	// mov     eax, ds:dword_AAE950
	PatchInstall(0x4C68B4 + 1, &IDE_objsType1_store->count);	// mov     ds:dword_AAE950, eax
	PatchInstall(0x4C68E7 + 1, &IDE_objsType1_store->count);	// mov     eax, ds:dword_AAE950
	PatchInstall(0x4C68F8 + 1, &IDE_objsType1_store->count);	// mov     ds:dword_AAE950, eax
	PatchInstall(0x4C6926 + 1, &IDE_objsType1_store->count);	// mov     eax, ds:dword_AAE950
	PatchInstall(0x4C6937 + 1, &IDE_objsType1_store->count);	// mov     ds:dword_AAE950, eax
	PatchInstall(0x4C6965 + 1, &IDE_objsType1_store->count);	// mov     eax, ds:dword_AAE950
	PatchInstall(0x4C6976 + 1, &IDE_objsType1_store->count);	// mov     ds:dword_AAE950, eax
	PatchInstall(0x4C69A4 + 1, &IDE_objsType1_store->count);	// mov     eax, ds:dword_AAE950
	PatchInstall(0x4C69B5 + 1, &IDE_objsType1_store->count);	// mov     ds:dword_AAE950, eax
	PatchInstall(0x4C69E3 + 1, &IDE_objsType1_store->count);	// mov     eax, ds:dword_AAE950
	PatchInstall(0x4C69F4 + 1, &IDE_objsType1_store->count);	// mov     ds:dword_AAE950, eax
	PatchInstall(0x4C6A22 + 1, &IDE_objsType1_store->count);	// mov     eax, ds:dword_AAE950
	PatchInstall(0x4C6A33 + 1, &IDE_objsType1_store->count);	// mov     ds:dword_AAE950, eax
	PatchInstall(0x84BBF0 + 1, &IDE_objsType1_store->count);	// mov     ecx, offset dword_AAE950
	PatchInstall(0x856230 + 1, &IDE_objsType1_store->count);	// mov     ecx, offset dword_AAE950

	// Data references to array of CStore (.data:00AAE954)
	PatchInstall(0x4C63F1 + 1, &IDE_objsType1_store->array);	// mov     edi, offset dword_AAE954
	PatchInstall(0x4C662B + 2, &IDE_objsType1_store->array);	// add     esi, offset dword_AAE954
	PatchInstall(0x4C6821 + 1, &IDE_objsType1_store->array);	// mov     eax, ds:dword_AAE954
	PatchInstall(0x4C6828 + 1, &IDE_objsType1_store->array);	// mov     ecx, offset dword_AAE954
	PatchInstall(0x4C6876 + 1, &IDE_objsType1_store->array);	// mov     ecx, offset dword_AAE954
	PatchInstall(0x4C687B + 6, &IDE_objsType1_store->array);	// mov     ds:_modelPtrs+5D8h, offset dword_AAE954; jumptable 014A8B4E case 3
	PatchInstall(0x4C688F + 1, &IDE_objsType1_store->array);	// mov     ecx, offset dword_AAE954
	PatchInstall(0x4C68A3 + 2, &IDE_objsType1_store->array);	// add     esi, offset dword_AAE954
	PatchInstall(0x4C68F1 + 2, &IDE_objsType1_store->array);	// add     esi, offset dword_AAE954
	PatchInstall(0x4C6930 + 2, &IDE_objsType1_store->array);	// add     esi, offset dword_AAE954
	PatchInstall(0x4C696F + 2, &IDE_objsType1_store->array);	// add     esi, offset dword_AAE954
	PatchInstall(0x4C69AE + 2, &IDE_objsType1_store->array);	// add     esi, offset dword_AAE954
	PatchInstall(0x4C69ED + 2, &IDE_objsType1_store->array);	// add     esi, offset dword_AAE954
	PatchInstall(0x4C6A2C + 2, &IDE_objsType1_store->array);	// add     esi, offset dword_AAE954

	//// Count of objects ////
	// ModelObjsData__constructor
	PatchInstall(0x4C5CBB + 1, iObjectsType1);

	// $ModelObjsData__destructor
	PatchInstall(0x4C5845 + 1, iObjectsType1);
}

class COccluder
{
public:
	int16_t midX;
	int16_t midY;
	int16_t midZ;
	int16_t widthY;
	int16_t widthX;
	int16_t height;
	char rotation;
	char rotation2;
	char rotation3;
	char field_F;
	int16_t flags;
};

void PatchOccluders()
{
	unsigned int OccludersLimit = 3000;

	COccluder* COcclusion__aOccluders = new COccluder[OccludersLimit];

	PatchInstall(0x71DE6D + 3, &COcclusion__aOccluders->midX);	// mov     word ptr ds:_ZN10COcclusion10aOccludersE.midX[esi], ax; COcclusion::aOccluders
	PatchInstall(0x71DE83 + 3, &COcclusion__aOccluders->midY);	// mov     ds:_ZN10COcclusion10aOccludersE.midY[esi], ax
	PatchInstall(0x71DE93 + 3, &COcclusion__aOccluders->midZ);	// mov     ds:_ZN10COcclusion10aOccludersE.midZ[esi], ax
	PatchInstall(0x71DEA9 + 3, &COcclusion__aOccluders->widthY);	// mov     ds:_ZN10COcclusion10aOccludersE.widthY[esi], ax
	PatchInstall(0x71DEBF + 3, &COcclusion__aOccluders->widthX);	// mov     ds:_ZN10COcclusion10aOccludersE.widthX[esi], ax
	PatchInstall(0x71DEDB + 3, &COcclusion__aOccluders->height);	// mov     ds:_ZN10COcclusion10aOccludersE.height[esi], ax
	PatchInstall(0x71DEF1 + 2, &COcclusion__aOccluders->rotation3);	// mov     ds:_ZN10COcclusion10aOccludersE.rotation3[esi], al
	PatchInstall(0x71DF06 + 2, &COcclusion__aOccluders->rotation2);	// mov     ds:_ZN10COcclusion10aOccludersE.rotation2[esi], al
	PatchInstall(0x71DF11 + 2, &COcclusion__aOccluders->rotation);	// mov     ds:_ZN10COcclusion10aOccludersE.rotation[esi], al
	PatchInstall(0x71DF1F + 3, &COcclusion__aOccluders->flags);	// or      ds:_ZN10COcclusion10aOccludersE.flags[esi], 8000h
	PatchInstall(0x71DF2A + 2, (void*)((int)&COcclusion__aOccluders->flags + 1));	// and     byte ptr (_ZN10COcclusion10aOccludersE.flags+1)[esi], 7Fh
	PatchInstall(0x71DF33 + 3, &COcclusion__aOccluders->flags);	// mov     ax, ds:_ZN10COcclusion10aOccludersE.flags[esi]
	PatchInstall(0x71DF4D + 3, &COcclusion__aOccluders->flags);	// xor     ds:_ZN10COcclusion10aOccludersE.flags[esi], ax
	PatchInstall(0x720228 + 2, &COcclusion__aOccluders->midX);	// lea     ecx, _ZN10COcclusion10aOccludersE.midX[ebx]; COcclusion::aOccluders
	PatchInstall(0x720247 + 3, &COcclusion__aOccluders->flags);	// mov     ax, ds:_ZN10COcclusion10aOccludersE.flags[ebx]
	PatchInstall(0x72025E + 3, &COcclusion__aOccluders->flags);	// mov     dx, ds:_ZN10COcclusion10aOccludersE.flags[ebx]
	PatchInstall(0x720270 + 4, &COcclusion__aOccluders->flags);	// xor     dx, ds:_ZN10COcclusion10aOccludersE.flags[eax*2]
	PatchInstall(0x72027E + 4, &COcclusion__aOccluders->flags);	// xor     ds:_ZN10COcclusion10aOccludersE.flags[eax*2], dx
	PatchInstall(0x720286 + 3, &COcclusion__aOccluders->flags);	// mov     si, ds:_ZN10COcclusion10aOccludersE.flags[ebx]
	PatchInstall(0x720295 + 4, &COcclusion__aOccluders->flags);	// mov     dx, ds:_ZN10COcclusion10aOccludersE.flags[eax*2]
	PatchInstall(0x7202B7 + 4, &COcclusion__aOccluders->flags);	// xor     ds:_ZN10COcclusion10aOccludersE.flags[eax*2], dx
	PatchInstall(0x7202C4 + 3, &COcclusion__aOccluders->flags);	// mov     si, ds:_ZN10COcclusion10aOccludersE.flags[ebx]
	PatchInstall(0x72031C + 3, &COcclusion__aOccluders->midX);	// lea     ecx, _ZN10COcclusion10aOccludersE.midX[ecx*2]; COcclusion::aOccluders
	PatchInstall(0x720341 + 2, &COcclusion__aOccluders->midX);	// lea     ecx, _ZN10COcclusion10aOccludersE.midX[ebx]; COcclusion::aOccluders
	PatchInstall(0x72035C + 3, &COcclusion__aOccluders->flags);	// mov     ax, ds:_ZN10COcclusion10aOccludersE.flags[ebx]
	PatchInstall(0x720373 + 3, &COcclusion__aOccluders->flags);	// mov     dx, ds:_ZN10COcclusion10aOccludersE.flags[ebx]
	PatchInstall(0x720385 + 4, &COcclusion__aOccluders->flags);	// xor     dx, ds:_ZN10COcclusion10aOccludersE.flags[eax*2]
	PatchInstall(0x720393 + 4, &COcclusion__aOccluders->flags);	// xor     ds:_ZN10COcclusion10aOccludersE.flags[eax*2], dx
	PatchInstall(0x72039B + 3, &COcclusion__aOccluders->flags);	// mov     si, ds:_ZN10COcclusion10aOccludersE.flags[ebx]
	PatchInstall(0x7203AA + 4, &COcclusion__aOccluders->flags);	// mov     dx, ds:_ZN10COcclusion10aOccludersE.flags[eax*2]
	PatchInstall(0x7203CC + 4, &COcclusion__aOccluders->flags);	// xor     ds:_ZN10COcclusion10aOccludersE.flags[eax*2], dx
	PatchInstall(0x7203DA + 3, &COcclusion__aOccluders->flags);	// mov     si, ds:_ZN10COcclusion10aOccludersE.flags[ebx]

	PatchInstall(0x71DE43 + 2, OccludersLimit);
}

#pragma pack(push, 1)
class CompressedVector
{
public:
	int16_t x;
	int16_t y;
	int16_t z;
};
#pragma pack(pop)

#pragma pack(push, 1)
class CompressedVector_extended
{
public:
	int32_t x;
	int32_t y;
	int32_t z;
};
#pragma pack(pop)

class CPickup
{
public:
	int field_0;
	class CObject* pObject;
	int field_8;
	int regenerationTime;
	CompressedVector pos;
	int16_t field_16;
	int16_t wModelID;
	int16_t wGeneration;
	uint8_t nbPickupType;
	char flags;
	char field_1E;
	char field_1F;
};

class CPickup_extended : public CPickup
{
public:
	CompressedVector_extended exPos;
};

void PatchPickups()
{
	uint32_t PickupsLimit = 3000;

	CPickup_extended* CPickups__aPickUps = new CPickup_extended[PickupsLimit];

	uint32_t sizeof_CPickup = sizeof(CPickup_extended);

	PatchInstall(0x4020BB + 1, CPickups__aPickUps);	// sub     eax, offset _ZN8CPickups8aPickUpsE; CPickups::aPickUps	; sub_4556C0
	PatchInstall(0x455204 + 1, &CPickups__aPickUps->regenerationTime);	// mov     eax, offset _ZN8CPickups8aPickUpsE.regenerationTime	; sub_4414C0
	PatchInstall(0x455471 + 1, &CPickups__aPickUps->pObject);	// mov     esi, offset _ZN8CPickups8aPickUpsE.pObject	; sub_455470
	PatchInstall(0x4555BD + 1, &CPickups__aPickUps->pos.y);	// mov     ecx, offset _ZN8CPickups8aPickUpsE.pos.y	; sub_4555A0
	PatchInstall(0x455664 + 2, &CPickups__aPickUps->field_8);	// lea     eax, _ZN8CPickups8aPickUpsE.field_8[edx]	; sub_4555A0
	PatchInstall(0x4556D2 + 3, &CPickups__aPickUps->wGeneration);	// movzx   edx, ds:_ZN8CPickups8aPickUpsE.wGeneration[ecx]	; sub_4556C0
	PatchInstall(0x4563A9 + 1, &CPickups__aPickUps->pos.y);	// mov     esi, offset _ZN8CPickups8aPickUpsE.pos.y	; sub_4563A0
	PatchInstall(0x456453 + 1, &CPickups__aPickUps->pos.y);	// mov     ecx, offset _ZN8CPickups8aPickUpsE.pos.y	; sub_456450
	PatchInstall(0x456A39 + 2, &CPickups__aPickUps->wGeneration);	// lea     eax, _ZN8CPickups8aPickUpsE.wGeneration[eax]	; sub_456A30
	PatchInstall(0x456A83 + 1, &CPickups__aPickUps->pos.y);	// mov     esi, offset _ZN8CPickups8aPickUpsE.pos.y	; sub_456A70
	PatchInstall(0x456BF3 + 2, CPickups__aPickUps);	// add     edi, offset _ZN8CPickups8aPickUpsE; CPickups::aPickUps	; sub_456A70
	PatchInstall(0x456BFD + 1, CPickups__aPickUps);	// sub     eax, offset _ZN8CPickups8aPickUpsE; CPickups::aPickUps	; sub_456A70
	PatchInstall(0x456C0A + 3, &CPickups__aPickUps->wGeneration);	// movzx   ecx, ds:_ZN8CPickups8aPickUpsE.wGeneration[edx]	; sub_456A70
	PatchInstall(0x456D32 + 1, &CPickups__aPickUps->pos);	// mov     esi, offset _ZN8CPickups8aPickUpsE.pos	; sub_456D30
	PatchInstall(0x456DE2 + 1, &CPickups__aPickUps->pObject);	// mov     esi, offset _ZN8CPickups8aPickUpsE.pObject	; sub_456DE0
	PatchInstall(0x456E00 + 3, &CPickups__aPickUps->wGeneration);	// movzx   edx, ds:_ZN8CPickups8aPickUpsE.wGeneration[ecx]	; sub_456DE0
	PatchInstall(0x456E6C + 1, &CPickups__aPickUps->pObject);	// mov     esi, offset _ZN8CPickups8aPickUpsE.pObject	; sub_456E60
	PatchInstall(0x456E82 + 3, &CPickups__aPickUps->wGeneration);	// movzx   edx, ds:_ZN8CPickups8aPickUpsE.wGeneration[ecx]	; sub_456E60
	PatchInstall(0x456ED6 + 1, &CPickups__aPickUps->wGeneration);	// mov     eax, offset _ZN8CPickups8aPickUpsE.wGeneration	; sub_456E60
	PatchInstall(0x456F43 + 1, &((CPickup*)((char*)CPickups__aPickUps + sizeof_CPickup * (PickupsLimit - 1)))->nbPickupType);	// mov     eax, (offset _ZN8CPickups8aPickUpsE.nbPickupType+4D60h)	; sub_456F20
	PatchInstall(0x456F51 + 1, &CPickups__aPickUps->nbPickupType);	// cmp     eax, offset _ZN8CPickups8aPickUpsE.nbPickupType	; sub_456F20
	PatchInstall(0x456F64 + 1, &((CPickup*)((char*)CPickups__aPickUps + sizeof_CPickup))->nbPickupType);	// mov     eax, (offset _ZN8CPickups8aPickUpsE.nbPickupType+20h)	; sub_456F20
	PatchInstall(0x457002 + 1, &((CPickup*)((char*)CPickups__aPickUps + sizeof_CPickup))->nbPickupType);	// mov     eax, (offset _ZN8CPickups8aPickUpsE.nbPickupType+20h)	; sub_456F20
	PatchInstall(0x457099 + 1, &((CPickup*)((char*)CPickups__aPickUps + sizeof_CPickup))->nbPickupType);	// mov     eax, (offset _ZN8CPickups8aPickUpsE.nbPickupType+20h)	; sub_456F20
	PatchInstall(0x45719F + 2, CPickups__aPickUps);	// add     esi, offset _ZN8CPickups8aPickUpsE; CPickups::aPickUps	; sub_456F20
	PatchInstall(0x4571E8 + 2, &CPickups__aPickUps->field_8);	// mov     ds:_ZN8CPickups8aPickUpsE.field_8[esi], eax	; sub_456F20
	PatchInstall(0x4571F3 + 3, &CPickups__aPickUps->field_16);	// mov     ds:_ZN8CPickups8aPickUpsE.field_16[esi], ax	; sub_456F20
	PatchInstall(0x4571FA + 2, &CPickups__aPickUps->flags);	// mov     al, ds:_ZN8CPickups8aPickUpsE.flags[esi]	; sub_456F20
	PatchInstall(0x457214 + 2, &CPickups__aPickUps->field_0);	// lea     edi, _ZN8CPickups8aPickUpsE.field_0[esi]; CPickups::aPickUps	; sub_456F20
	PatchInstall(0x45721A + 2, &CPickups__aPickUps->nbPickupType);	// mov     ds:_ZN8CPickups8aPickUpsE.nbPickupType[esi], cl	; sub_456F20
	PatchInstall(0x457226 + 2, &CPickups__aPickUps->regenerationTime);	// mov     ds:_ZN8CPickups8aPickUpsE.regenerationTime[esi], ebp	; sub_456F20
	PatchInstall(0x45722C + 2, &CPickups__aPickUps->flags);	// mov     ds:_ZN8CPickups8aPickUpsE.flags[esi], dl	; sub_456F20
	PatchInstall(0x457266 + 2, &CPickups__aPickUps->nbPickupType);	// mov     ds:_ZN8CPickups8aPickUpsE.nbPickupType[esi], 9	; sub_456F20
	PatchInstall(0x45726D + 2, &CPickups__aPickUps->regenerationTime);	// mov     ds:_ZN8CPickups8aPickUpsE.regenerationTime[esi], edx	; sub_456F20
	PatchInstall(0x45727D + 2, &CPickups__aPickUps->nbPickupType);	// mov     ds:_ZN8CPickups8aPickUpsE.nbPickupType[esi], 0Bh	; sub_456F20
	PatchInstall(0x45728A + 2, &CPickups__aPickUps->regenerationTime);	// mov     ds:_ZN8CPickups8aPickUpsE.regenerationTime[esi], ebp	; sub_456F20
	PatchInstall(0x45729A + 3, &CPickups__aPickUps->wModelID);	// mov     ds:_ZN8CPickups8aPickUpsE.wModelID[esi], ax	; sub_456F20
	PatchInstall(0x4572AA + 2, &CPickups__aPickUps->flags);	// mov     cl, ds:_ZN8CPickups8aPickUpsE.flags[esi]	; sub_456F20
	PatchInstall(0x4572C2 + 2, &CPickups__aPickUps->flags);	// mov     ds:_ZN8CPickups8aPickUpsE.flags[esi], cl	; sub_456F20
	PatchInstall(0x4572FE + 2, &CPickups__aPickUps->flags);	// mov     cl, ds:_ZN8CPickups8aPickUpsE.flags[esi]	; sub_456F20
	PatchInstall(0x457313 + 2, &CPickups__aPickUps->pObject);	// lea     ebp, _ZN8CPickups8aPickUpsE.pObject[esi]	; sub_456F20
	PatchInstall(0x457319 + 2, &CPickups__aPickUps->flags);	// mov     ds:_ZN8CPickups8aPickUpsE.flags[esi], dl	; sub_456F20
	PatchInstall(0x457344 + 3, &CPickups__aPickUps->wGeneration);	// mov     ax, ds:_ZN8CPickups8aPickUpsE.wGeneration[esi]	; sub_456F20
	PatchInstall(0x457354 + 3, &CPickups__aPickUps->wGeneration);	// mov     ds:_ZN8CPickups8aPickUpsE.wGeneration[esi], ax	; sub_456F20
	PatchInstall(0x457366 + 3, &CPickups__aPickUps->wGeneration);	// mov     ds:_ZN8CPickups8aPickUpsE.wGeneration[esi], 1	; sub_456F20
	PatchInstall(0x45736F + 3, &CPickups__aPickUps->wGeneration);	// movzx   eax, ds:_ZN8CPickups8aPickUpsE.wGeneration[esi]	; sub_456F20
	PatchInstall(0x4573E8 + 3, &CPickups__aPickUps->wGeneration);	// cmp     cx, ds:_ZN8CPickups8aPickUpsE.wGeneration[edx]	; sub_4573D0
	PatchInstall(0x4573F6 + 2, &CPickups__aPickUps->field_0);	// lea     ecx, _ZN8CPickups8aPickUpsE.field_0[edx]; CPickups::aPickUps	; sub_4573D0
	PatchInstall(0x458E2A + 2, &CPickups__aPickUps->pObject);	// add     esi, offset _ZN8CPickups8aPickUpsE.pObject	; sub_458DE0
	PatchInstall(0x458FE4 + 2, &CPickups__aPickUps->nbPickupType);	// add     esi, offset _ZN8CPickups8aPickUpsE.nbPickupType	; sub_458DE0
	PatchInstall(0x4590CB + 1, CPickups__aPickUps);	// mov     esi, offset _ZN8CPickups8aPickUpsE; CPickups::aPickUps	; sub_4590C0
	PatchInstall(0x4590E2 + 2, ((CPickup*)((char*)CPickups__aPickUps + sizeof_CPickup * PickupsLimit)));	// cmp     esi, offset dword_97D640	; sub_4590C0
	PatchInstall(0x47AC91 + 1, CPickups__aPickUps);	// mov     edi, offset _ZN8CPickups8aPickUpsE; CPickups::aPickUps	; sub_47A760
	PatchInstall(0x48ADC2 + 1, CPickups__aPickUps);	// mov     edi, offset _ZN8CPickups8aPickUpsE; CPickups::aPickUps	; sub_48A320
	PatchInstall(0x494725 + 2, CPickups__aPickUps);	// add     ecx, offset _ZN8CPickups8aPickUpsE; CPickups::aPickUps	; sub_493FE0
	PatchInstall(0x585F08 + 2, CPickups__aPickUps);	// add     ecx, offset _ZN8CPickups8aPickUpsE; CPickups::aPickUps	; sub_585BF0
	PatchInstall(0x587110 + 1, CPickups__aPickUps);	// add     eax, offset _ZN8CPickups8aPickUpsE; CPickups::aPickUps	; sub_587000
	PatchInstall(0x5D3542 + 1, CPickups__aPickUps);	// mov     esi, offset _ZN8CPickups8aPickUpsE; CPickups::aPickUps	; sub_5D3540
	PatchInstall(0x5D3555 + 2, ((CPickup*)((char*)CPickups__aPickUps + sizeof_CPickup * PickupsLimit)));	// cmp     esi, offset dword_97D640	; sub_5D3540
	// PatchInstall(0x5D35A2 + 1, &CPickups__aPickUps->pObject);	// mov     esi, offset _ZN8CPickups8aPickUpsE.pObject	; sub_5D35A0
	PatchInstall(0x1561589 + 3, &CPickups__aPickUps->wGeneration);	// movzx   edx, ds:_ZN8CPickups8aPickUpsE.wGeneration[ecx]	; sub_1561580
	PatchInstall(0x1564659 + 1, &CPickups__aPickUps->wGeneration);	// mov     eax, offset _ZN8CPickups8aPickUpsE.wGeneration	; sub_1564650
	PatchInstall(0x1564BC8 + 3, &CPickups__aPickUps->wGeneration);	// cmp     cx, ds:_ZN8CPickups8aPickUpsE.wGeneration[edx]	; sub_1564BB0
	PatchInstall(0x156B378 + 3, &CPickups__aPickUps->wGeneration);	// cmp     cx, ds:_ZN8CPickups8aPickUpsE.wGeneration[edx]	; sub_156B360
	PatchInstall(0x156B38B + 3, &CPickups__aPickUps->field_16);	// mov     ds:_ZN8CPickups8aPickUpsE.field_16[edx], ax	; sub_156B360
	PatchInstall(0x156D876 + 1, &CPickups__aPickUps->pObject);	// mov     ecx, offset _ZN8CPickups8aPickUpsE.pObject	; sub_156D870
	PatchInstall(0x156D896 + 1, CPickups__aPickUps);	// mov     eax, offset _ZN8CPickups8aPickUpsE; CPickups::aPickUps	; sub_156D870
	PatchInstall(0x156D8A0 + 1, CPickups__aPickUps);	// add     eax, offset _ZN8CPickups8aPickUpsE; CPickups::aPickUps	; sub_156D870

	PatchInstall(0x455231 + 1, &((CPickup*)((char*)CPickups__aPickUps + sizeof_CPickup * PickupsLimit))->regenerationTime);	// cmp     eax, offset flt_97D64C	; sub_4414C0
	PatchInstall(0x4554A9 + 2, &((CPickup*)((char*)CPickups__aPickUps + sizeof_CPickup * PickupsLimit))->pObject);	// cmp     esi, offset dword_97D644	; sub_455470
	PatchInstall(0x455649 + 2, &((CPickup*)((char*)CPickups__aPickUps + sizeof_CPickup * PickupsLimit))->pos.y);	// cmp     ecx, (offset flt_97D650+2)	; sub_4555A0
	PatchInstall(0x456436 + 2, &((CPickup*)((char*)CPickups__aPickUps + sizeof_CPickup * PickupsLimit))->pos.y);	// cmp     esi, (offset flt_97D650+2)	; sub_4563A0
	PatchInstall(0x4564D7 + 2, &((CPickup*)((char*)CPickups__aPickUps + sizeof_CPickup * PickupsLimit))->pos.y);	// cmp     ecx, (offset flt_97D650+2)	; sub_456450
	PatchInstall(0x456BDC + 2, &((CPickup*)((char*)CPickups__aPickUps + sizeof_CPickup * PickupsLimit))->pos.y);	// cmp     esi, (offset flt_97D650+2)	; sub_456A70
	PatchInstall(0x456DD1 + 2, &((CPickup*)((char*)CPickups__aPickUps + sizeof_CPickup * PickupsLimit))->pos);	// cmp     esi, offset flt_97D650	; sub_456D30
	PatchInstall(0x456E4A + 2, &((CPickup*)((char*)CPickups__aPickUps + sizeof_CPickup * PickupsLimit))->pObject);	// cmp     esi, offset dword_97D644	; sub_456DE0
	PatchInstall(0x456EC7 + 2, &((CPickup*)((char*)CPickups__aPickUps + sizeof_CPickup * PickupsLimit))->pObject);	// cmp     esi, offset dword_97D644	; sub_456E60
	PatchInstall(0x456EEE + 1, &((CPickup*)((char*)CPickups__aPickUps + sizeof_CPickup * PickupsLimit))->wGeneration);	// cmp     eax, (offset flt_97D658+2)	; sub_456E60
	PatchInstall(0x5D35D2 + 2, &((CPickup*)((char*)CPickups__aPickUps + sizeof_CPickup * PickupsLimit))->pObject);	// cmp     esi, offset dword_97D644	; sub_5D35A0
	PatchInstall(0x156466E + 1, &((CPickup*)((char*)CPickups__aPickUps + sizeof_CPickup * PickupsLimit))->wGeneration);	// cmp     eax, (offset flt_97D658+2)	; sub_1564650
	PatchInstall(0x156D88E + 2, &((CPickup*)((char*)CPickups__aPickUps + sizeof_CPickup * PickupsLimit))->pObject);	// cmp     ecx, offset dword_97D644	; sub_156D870

	PatchInstall(0x456FC2 + 1, &((CPickup*)((char*)CPickups__aPickUps + sizeof_CPickup * (PickupsLimit + 1)))->nbPickupType);	// cmp     eax, offset unk_97D67C
	PatchInstall(0x457059 + 1, &((CPickup*)((char*)CPickups__aPickUps + sizeof_CPickup * (PickupsLimit + 1)))->nbPickupType);	// cmp     eax, offset unk_97D67C
	PatchInstall(0x457154 + 1, &((CPickup*)((char*)CPickups__aPickUps + sizeof_CPickup * (PickupsLimit + 1)))->nbPickupType);	// cmp     eax, offset unk_97D67C

	PatchInstall(0x456FF4 + 2, PickupsLimit);	// cmp     ebx, 26Ch	; sub_456F20
	PatchInstall(0x45708B + 2, PickupsLimit);	// cmp     ebx, 620	; sub_456F20
	PatchInstall(0x45718E + 2, PickupsLimit);	// cmp     ebx, 26Ch	; sub_456F20
	PatchInstall(0x4571D1 + 2, PickupsLimit);	// cmp     ebx, 26Ch	; sub_456F20
	PatchInstall(0x458DF7 + 2, PickupsLimit);	// imul    ecx, 26Ch	; sub_458DE0
	PatchInstall(0x458E0B + 2, PickupsLimit);	// lea     eax, [ecx+26Ch]	; sub_458DE0
	PatchInstall(0x458FB0 + 2, PickupsLimit);	// imul    ecx, 26Ch	; sub_458DE0
	PatchInstall(0x458FBF + 2, PickupsLimit);	// add     ecx, 26Ch	; sub_458DE0

	uint32_t PickupsCollectedLimit = 256;
	uint32_t* CPickups__aPickUpsCollected = new uint32_t[PickupsCollectedLimit];

	// Patch references to CPickups::aPickUpsCollected
	{
		PatchInstall(0x456EFC + 1, CPickups__aPickUpsCollected);	// mov     edi, offset _ZN8CPickups17aPickUpsCollectedE; CPickups::aPickUpsCollected	; sub_456E60
		PatchInstall(0x45904F + 3, CPickups__aPickUpsCollected);	// mov     ds:_ZN8CPickups17aPickUpsCollectedE[edx*4], ecx; CPickups::aPickUpsCollected	; sub_458DE0
		PatchInstall(0x5D3578 + 1, CPickups__aPickUpsCollected);	// mov     esi, offset _ZN8CPickups17aPickUpsCollectedE; CPickups::aPickUpsCollected	; sub_5D3540
		PatchInstall(0x5D35FE + 1, CPickups__aPickUpsCollected);	// mov     esi, offset _ZN8CPickups17aPickUpsCollectedE; CPickups::aPickUpsCollected	; sub_5D35A0
		PatchInstall(0x15615A4 + 3, CPickups__aPickUpsCollected);	// mov     ds:_ZN8CPickups17aPickUpsCollectedE[ecx*4], edx; CPickups::aPickUpsCollected	; sub_1561580
		PatchInstall(0x156467D + 1, CPickups__aPickUpsCollected);	// mov     edi, offset _ZN8CPickups17aPickUpsCollectedE; CPickups::aPickUpsCollected	; sub_1564650
		PatchInstall(0x156DA66 + 3, CPickups__aPickUpsCollected);	// cmp     ds:_ZN8CPickups17aPickUpsCollectedE[eax*4], ecx; CPickups::aPickUpsCollected	; sub_156DA60
		PatchInstall(0x156DA78 + 3, CPickups__aPickUpsCollected);	// mov     ds:_ZN8CPickups17aPickUpsCollectedE[eax*4], 0; CPickups::aPickUpsCollected	; sub_156DA60
	}

	// Patch the number of pickups collected
	{
		PatchInstall(0x1564676 + 1, PickupsCollectedLimit);	// mov     ecx, 20

		PatchInstall(0x156DA70 + 2, PickupsCollectedLimit);	// cmp     eax, 20

		PatchInstall(0x15615A0 + 3, PickupsCollectedLimit);	// cmp     ax, 20

		PatchInstall(0x456EF5 + 1, PickupsCollectedLimit);	// mov     ecx, 20

		PatchInstall(0x45904B + 2, PickupsCollectedLimit);	// cmp     ax, 20			
	}
}

void SetInstLimitPerFile(unsigned int iInstPerFile)
{
	void* IPLentriesOfFile = new void*[iInstPerFile];

	PatchInstall(0x5B51F0 + 3, IPLentriesOfFile);		// mov     eax, ds:__iplInst[edx*4]
	PatchInstall(0x5B51FF + 3, IPLentriesOfFile);		// mov     ecx, ds:__iplInst[ecx*4]
	PatchInstall(0x5B524B + 1, IPLentriesOfFile);		// push    offset __iplInst
	PatchInstall(0x5B5258 + 3, IPLentriesOfFile);		// mov     esi, ds:__iplInst[ebx*4]
	PatchInstall(0x5B531C + 1, IPLentriesOfFile);		// push    offset __iplInst
	PatchInstall(0x5B5340 + 3, IPLentriesOfFile);		// mov     edx, ds:__iplInst[esi*4]
	PatchInstall(0x5B8938 + 3, IPLentriesOfFile);		// mov     ds:__iplInst[ecx*4], eax
	PatchInstall(0x5B8A50 + 1, IPLentriesOfFile);		// mov     ecx, offset __iplInst
	PatchInstall(0x5B8A50 + 1, IPLentriesOfFile);		// mov     ecx, offset __iplInst
	PatchInstall(0x5B8A6F + 3, IPLentriesOfFile);		// lea     eax, __iplInst[edx*4]
}

void SetTcyc(unsigned int iTcycEntries)
{

}

void CLimitAdjusterSA::IdeLimits(unsigned short usVehicle, unsigned short usPed)
{
	//WriteDebugEvent(xorstr_("Load hooks values 2"));

	//SetIDEobjsType1(50000);

	//SetEntryExits(30000);
	//SetRwObjectInstances(13000);
	//SetEntityIndexArray(50); // 40
	//PatchOccluders(); // 3000
	//PatchPickups(); // 3000

	//SetInstLimitPerFile(8192);

	// AlphaEntityList
	PatchInstall<unsigned char>(0x527E6C, 128);

	// IPL Files
	PatchInstall<unsigned char>(0x405F25, 0x68);
	PatchInstall<unsigned char>(0x405F26, 128);

	// Polygons
	//PatchInstall<unsigned char>(0x731F5F, 0x68);
	//PatchInstall<uint32_t>(0x731F60, 512000);

	// QuadtreeNodes
	//PatchInstall<unsigned char>(0x552C3E, 0x68);
	//PatchInstall<uint32_t>(0x552C3F, 500000);

	//WriteDebugEvent(xorstr_("Load hooks values 3"));
	struct IdeLimit
	{
		uint CountLimit;
		char ArrayLimit;
	};

	IdeLimit* VehicleLimit = (IdeLimit*)new char[sizeof(uint32_t) + usVehicle * 0x308];
	IdeLimit* PedLimit = (IdeLimit*)new char[sizeof(uint32_t) + usPed * 0x44];

	PatchInstall(0x4C64EC + 1, &VehicleLimit->CountLimit);
	PatchInstall(0x4C6507 + 1, &VehicleLimit->CountLimit);
	PatchInstall(0x4C6602 + 2, &VehicleLimit->CountLimit);
	PatchInstall(0x4C6770 + 1, &VehicleLimit->CountLimit);
	PatchInstall(0x4C6785 + 1, &VehicleLimit->CountLimit);
	PatchInstall(0x4C6851 + 2, &VehicleLimit->CountLimit);
	PatchInstall(0x84BCD0 + 1, &VehicleLimit->CountLimit);
	PatchInstall(0x8562A0 + 1, &VehicleLimit->CountLimit);
	PatchInstall(0x4C64F7 + 1, &VehicleLimit->ArrayLimit);
	PatchInstall(0x4C677E + 2, &VehicleLimit->ArrayLimit);
	PatchInstall(0x4C5F5B + 1, usVehicle);
	PatchInstall(0x4C6375 + 1, usVehicle);

	PatchInstall(0x4C6517 + 1, &PedLimit->CountLimit);
	PatchInstall(0x4C652E + 1, &PedLimit->CountLimit);
	PatchInstall(0x4C6608 + 2, &PedLimit->CountLimit);
	PatchInstall(0x4C67A0 + 1, &PedLimit->CountLimit);
	PatchInstall(0x4C67B2 + 1, &PedLimit->CountLimit);
	PatchInstall(0x4C6857 + 2, &PedLimit->CountLimit);
	PatchInstall(0x84BCF0 + 1, &PedLimit->CountLimit);
	PatchInstall(0x8562B0 + 1, &PedLimit->CountLimit);
	PatchInstall(0x4C6522 + 1, &PedLimit->ArrayLimit);
	PatchInstall(0x4C67AB + 2, &PedLimit->ArrayLimit);
	PatchInstall(0x4C67DB + 1, usPed);
	PatchInstall(0x4C6395 + 1, usPed);

	VehicleLimit = (IdeLimit*)0xB1F650;
	PedLimit = (IdeLimit*)0xB478F8;
}

void CLimitAdjusterSA::UpdateLimitIde()
{
    IdeLimits(900, 600);
}
