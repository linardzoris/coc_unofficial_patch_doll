#pragma once

namespace GameConstants
{
	void LoadConstants();

    bool GetLessBolts();
    bool GetHideWpnInvOpen();
    bool GetAfRadiationBackpack();
    bool GetAutoreload();
    bool GetSatietyEffector();
    bool GetThirstEffector();
    bool GetSatietyMindLoss();
    bool GetThirstMindLoss();
    bool GetAlcoholMindLoss();
    bool GetWeaponDistantSounds();
    bool GetBeltTwoRows();
    bool GetEqualWeaponSlots();
	bool GetSSS_DoF();

	Fvector4 GetSSFX_DefaultDoF();
	Fvector4 GetSSFX_FocusDoF();

	int GetWeaponSoundDist();
    int GetWeaponSoundDistFar();
	int GetArtefactsCount();
};
