#pragma once

#include "inventory_item_object.h"

struct SBoneProtections;

class CBackpack : public CInventoryItemObject
{
	typedef	CInventoryItemObject inherited;
public:
	CBackpack();
	virtual ~CBackpack();

	virtual void Load(LPCSTR section);

    virtual void Hit(float P, ALife::EHitType hit_type);

    float GetHitTypeProtection(ALife::EHitType hit_type, s16 element);
    float GetDefHitTypeProtection(ALife::EHitType hit_type);
    float GetBoneArmor(s16 element);
    float HitThroughArmor(float hit_power, s16 element, float ap, bool& add_wound, ALife::EHitType hit_type);

	virtual void OnMoveToSlot(const SInvItemPlace& prev);
	virtual void OnMoveToRuck(const SInvItemPlace& previous_place);
	virtual void OnH_A_Chield();

protected:
    HitImmunity::HitTypeSVec m_HitTypeProtection;

    shared_str m_ActorVisual;
    SBoneProtections* m_boneProtection;

public:
	float m_additional_weight;
	float m_additional_weight2;
    float m_fHealthRestoreSpeed;
    float m_fRadiationRestoreSpeed;
    float m_fSatietyRestoreSpeed;
    float m_fPowerRestoreSpeed;
    float m_fBleedingRestoreSpeed;
	float m_fPowerLoss;

    float m_fJumpSpeed;
    float m_fWalkAccel;

    shared_str m_BonesProtectionSect;

    virtual BOOL BonePassBullet(int boneID);
	virtual BOOL net_Spawn(CSE_Abstract* DC);
	virtual void net_Export(NET_Packet& P);
	virtual void net_Import(NET_Packet& P);
    void ReloadBonesProtection();
    void AddBonesProtection(LPCSTR bones_section);

    bool m_bShowStats;
    bool bIsExoskeleton;

protected:
    virtual bool install_upgrade_impl(LPCSTR section, bool test);
};
