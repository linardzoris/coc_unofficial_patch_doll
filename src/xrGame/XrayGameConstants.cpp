#include "StdAfx.h"
#include "XrayGameConstants.h"
#include "GamePersistent.h"
#include "game_cl_single.h"

bool m_bLessBolts = false;
bool m_bHideWpnInvOpen = false;
bool m_bAfRadiationBackpack = false;
bool m_bAutoreload = false;

bool m_bSatietyEffector = false;
bool m_bThirstEffector = false;
bool m_bSatietyMindLoss = false;
bool m_bThirstMindLoss = false;
bool m_bAlcoholMindLoss = false;

bool m_bWeaponDistantSounds = false;

bool m_bBeltTwoRows = false;
bool m_bEqualWeaponSlots = false;

Fvector4 m_FV4DefaultDoF = Fvector4().set(0.1f, 0.25f, 0.0f, 0.0f);
Fvector4 m_FV4FocusDoF = Fvector4().set(0.1f, 0.25f, 0.0f, 0.0f);

int m_iWeaponSoundDist = 150;
int m_iWeaponSoundDistFar = 250;

int m_iArtefactsCount = 5;

namespace GameConstants
{
	void LoadConstants()
	{
        m_bLessBolts = READ_IF_EXISTS(pSettings, r_bool, "gameplay", "less_bolts", false);
        m_bHideWpnInvOpen = READ_IF_EXISTS(pSettings, r_bool, "gameplay", "hide_weapon_when_inventory_open", false);
        m_bAfRadiationBackpack = READ_IF_EXISTS(pSettings, r_bool, "gameplay", "af_radiation_backpack", false);
        m_bAutoreload = READ_IF_EXISTS(pSettings, r_bool, "gameplay", "autoreload_enabled", false);

        m_bSatietyEffector = READ_IF_EXISTS(pSettings, r_bool, "gameplay", "satiety_effector", false);
        m_bThirstEffector = READ_IF_EXISTS(pSettings, r_bool, "gameplay", "thirst_effector", false);
        m_bSatietyMindLoss = READ_IF_EXISTS(pSettings, r_bool, "gameplay", "satiety_mind_loss", false);
        m_bThirstMindLoss = READ_IF_EXISTS(pSettings, r_bool, "gameplay", "thirst_mind_loss", false);
        m_bAlcoholMindLoss = READ_IF_EXISTS(pSettings, r_bool, "gameplay", "alcohol_mind_loss", false);

        m_bWeaponDistantSounds = READ_IF_EXISTS(pSettings, r_bool, "gameplay", "weapon_distant_sounds", false);
        m_iWeaponSoundDist = READ_IF_EXISTS(pSettings, r_u32, "gameplay", "weapon_distant_sound_distance", 150); 
        m_iWeaponSoundDistFar = READ_IF_EXISTS(pSettings, r_u32, "gameplay", "weapon_distant_sound_distance_far", 200); 

        m_iArtefactsCount = READ_IF_EXISTS(pSettings, r_u32, "gameplay", "artefacts_count", 5); // inventory
        m_bBeltTwoRows = READ_IF_EXISTS(pSettings, r_bool, "gameplay", "belt_two_rows", false); // inventory
        m_bEqualWeaponSlots = READ_IF_EXISTS(pSettings, r_bool, "gameplay", "equal_weapon_slots", false);

		m_FV4DefaultDoF = READ_IF_EXISTS(pSettings, r_fvector4, "gameplay", "SSS_default_dof", Fvector4().set(0.1f, 0.25f, 0.0f, 0.0f));
		m_FV4FocusDoF = READ_IF_EXISTS(pSettings, r_fvector4, "gameplay", "SSS_focus_dof", Fvector4().set(0.1f, 0.25f, 0.0f, 0.0f));
	}

    bool GetLessBolts()
	{
		return m_bLessBolts;
	}
    bool GetHideWpnInvOpen()
	{
		return m_bHideWpnInvOpen;
	}
    bool GetAfRadiationBackpack()
	{
		return m_bAfRadiationBackpack;
	}
    bool GetAutoreload()
	{
		return m_bAutoreload;
	}

    bool GetSatietyEffector()
	{
		return m_bSatietyEffector;
	}
    bool GetThirstEffector()
	{
		return m_bThirstEffector;
	}
    bool GetSatietyMindLoss()
	{
		return m_bSatietyMindLoss;
	}
    bool GetThirstMindLoss()
	{
		return m_bThirstMindLoss;
	}
    bool GetAlcoholMindLoss()
	{
		return m_bAlcoholMindLoss;
	}

    bool GetWeaponDistantSounds()
	{
		return m_bWeaponDistantSounds;
	}

    bool GetBeltTwoRows()
	{
		return m_bBeltTwoRows;
	}
	bool GetEqualWeaponSlots()
	{
		return m_bEqualWeaponSlots;
	}

	Fvector4 GetSSFX_DefaultDoF()
	{
		return m_FV4DefaultDoF;
	}

	Fvector4 GetSSFX_FocusDoF()
	{
		return m_FV4FocusDoF;
	}

	int GetWeaponSoundDist()
	{
		return m_iWeaponSoundDist;
	}
    int GetWeaponSoundDistFar()
	{
		return m_iWeaponSoundDistFar;
	}

	int GetArtefactsCount()
	{
		return m_iArtefactsCount;
	}
}
