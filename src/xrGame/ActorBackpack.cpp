#include "stdafx.h"
#include "ActorBackpack.h"
#include "Actor.h"
#include "Inventory.h"
#include "BoneProtections.h"
#include "Include/xrRender/Kinematics.h"

CBackpack::CBackpack()
{
    m_flags.set(FUsingCondition, TRUE);

    m_HitTypeProtection.resize(ALife::eHitTypeMax);
    for (int i = 0; i < ALife::eHitTypeMax; i++)
        m_HitTypeProtection[i] = 1.0f;

    m_boneProtection = new SBoneProtections();
    m_BonesProtectionSect = NULL;
}

CBackpack::~CBackpack()
{
	xr_delete(m_boneProtection);
}

BOOL CBackpack::net_Spawn(CSE_Abstract* DC)
{
    if (IsGameTypeSingle())
        ReloadBonesProtection();

    BOOL res = inherited::net_Spawn(DC);
    return (res);
}

void CBackpack::net_Export(NET_Packet& P)
{
    inherited::net_Export(P);
    // Может вызывать проблемы
    P.w_float_q8(GetCondition(), 0.0f, 1.0f);
}

void CBackpack::net_Import(NET_Packet& P)
{
    inherited::net_Import(P);
    // Может вызывать проблемы
    float _cond;
    P.r_float_q8(_cond, 0.0f, 1.0f);
    SetCondition(_cond);
}

void CBackpack::OnH_A_Chield()
{
    inherited::OnH_A_Chield();
    if (!IsGameTypeSingle())
        ReloadBonesProtection();
}

void CBackpack::Load(LPCSTR section)
{
    inherited::Load(section);

    m_HitTypeProtection[ALife::eHitTypeBurn] = pSettings->r_float(section, "burn_protection");
    m_HitTypeProtection[ALife::eHitTypeStrike] = pSettings->r_float(section, "strike_protection");
    m_HitTypeProtection[ALife::eHitTypeShock] = pSettings->r_float(section, "shock_protection");
    m_HitTypeProtection[ALife::eHitTypeWound] = pSettings->r_float(section, "wound_protection");
    m_HitTypeProtection[ALife::eHitTypeRadiation] = pSettings->r_float(section, "radiation_protection");
    m_HitTypeProtection[ALife::eHitTypeTelepatic] = pSettings->r_float(section, "telepatic_protection");
    m_HitTypeProtection[ALife::eHitTypeChemicalBurn] = pSettings->r_float(section, "chemical_burn_protection");
    m_HitTypeProtection[ALife::eHitTypeExplosion] = pSettings->r_float(section, "explosion_protection");
    m_HitTypeProtection[ALife::eHitTypeFireWound] = 0.f;
    m_HitTypeProtection[ALife::eHitTypeLightBurn] = m_HitTypeProtection[ALife::eHitTypeBurn];
    m_boneProtection->m_fHitFracActor = pSettings->r_float(section, "hit_fraction_actor");
    m_BonesProtectionSect = READ_IF_EXISTS(pSettings, r_string, section, "bones_koeff_protection", "");

    m_additional_weight = pSettings->r_float(section, "additional_inventory_weight");
    m_additional_weight2 = pSettings->r_float(section, "additional_inventory_weight2");
    m_fPowerRestoreSpeed = READ_IF_EXISTS(pSettings, r_float, section, "power_restore_speed", 0.0f);
    m_fPowerLoss = READ_IF_EXISTS(pSettings, r_float, section, "power_loss", 1.0f);
    clamp(m_fPowerLoss, EPS, 1.0f);

    m_fHealthRestoreSpeed = READ_IF_EXISTS(pSettings, r_float, section, "health_restore_speed", 0.0f);
    m_fRadiationRestoreSpeed = READ_IF_EXISTS(pSettings, r_float, section, "radiation_restore_speed", 0.0f);
    m_fSatietyRestoreSpeed = READ_IF_EXISTS(pSettings, r_float, section, "satiety_restore_speed", 0.0f);
    m_fPowerRestoreSpeed = READ_IF_EXISTS(pSettings, r_float, section, "power_restore_speed", 0.0f);
    m_fBleedingRestoreSpeed = READ_IF_EXISTS(pSettings, r_float, section, "bleeding_restore_speed", 0.0f);

    m_fJumpSpeed = READ_IF_EXISTS(pSettings, r_float, section, "jump_speed", 1.f);
    m_fWalkAccel = READ_IF_EXISTS(pSettings, r_float, section, "walk_accel", 1.f);
    m_fOverweightWalkK = READ_IF_EXISTS(pSettings, r_float, section, "overweight_walk_accel", 1.f);

    m_flags.set(FUsingCondition, READ_IF_EXISTS(pSettings, r_bool, section, "use_condition", TRUE));
	m_bShowStats = READ_IF_EXISTS(pSettings, r_bool, section, "show_protect_stats", FALSE);
}

void CBackpack::ReloadBonesProtection()
{
    IGameObject* parent = H_Parent();
    if (IsGameTypeSingle())
        parent = smart_cast<IGameObject*>(
            Level().CurrentViewEntity()); // TODO: FIX THIS OR NPC Can't wear outfit without resetting actor

    if (parent && parent->Visual() && m_BonesProtectionSect.size())
        m_boneProtection->reload(m_BonesProtectionSect, smart_cast<IKinematics*>(parent->Visual()));
}

void CBackpack::Hit(float hit_power, ALife::EHitType hit_type)
{
    hit_power *= GetHitImmunity(hit_type);
    ChangeCondition(-hit_power);
}

float CBackpack::GetDefHitTypeProtection(ALife::EHitType hit_type)
{
    return m_HitTypeProtection[hit_type] * GetCondition();
}

float CBackpack::GetHitTypeProtection(ALife::EHitType hit_type, s16 element)
{
    float fBase = m_HitTypeProtection[hit_type] * GetCondition();
    float bone = m_boneProtection->getBoneProtection(element);
    return fBase * bone;
}

float CBackpack::GetBoneArmor(s16 element) 
{ 
    return m_boneProtection->getBoneArmor(element); 
}

float CBackpack::HitThroughArmor(float hit_power, s16 element, float ap, bool& add_wound, ALife::EHitType hit_type)
{
    if (Core.ParamFlags.test(Core.dbgbullet))
        Msg("CCustomOutfit::HitThroughArmor hit_type=%d | unmodified hit_power=%f", (u32)hit_type, hit_power);

    float NewHitPower = hit_power;
    if (hit_type == ALife::eHitTypeFireWound)
    {
        float ba = GetBoneArmor(element);
        if (ba <= 0.0f)
            return NewHitPower;

        float BoneArmor = ba * GetCondition();
        if (/*!fis_zero(ba, EPS) && */ (ap > BoneArmor))
        {
            //пуля пробила бронь
            if (!IsGameTypeSingle())
            {
                float hit_fraction = (ap - BoneArmor) / ap;
                if (hit_fraction < m_boneProtection->m_fHitFracActor)
                    hit_fraction = m_boneProtection->m_fHitFracActor;

                NewHitPower *= hit_fraction;
                NewHitPower *= m_boneProtection->getBoneProtection(element);
            }

            VERIFY(NewHitPower >= 0.0f);

            float d_hit_power = (ap - BoneArmor) / ap;
            if (d_hit_power < m_boneProtection->m_fHitFracActor)
                d_hit_power = m_boneProtection->m_fHitFracActor;

            NewHitPower *= d_hit_power;

            if (Core.ParamFlags.test(Core.dbgbullet))
                Msg("CCustomOutfit::HitThroughArmor AP(%f) > bone_armor(%f) [HitFracActor=%f] modified hit_power=%f",
                    ap, BoneArmor, m_boneProtection->m_fHitFracActor, NewHitPower);
        }
        else
        {
            //пуля НЕ пробила бронь
            NewHitPower *= m_boneProtection->m_fHitFracActor;
            // add_wound = false; //раны нет
            if (Core.ParamFlags.test(Core.dbgbullet))
                Msg("CCustomOutfit::HitThroughArmor AP(%f) <= bone_armor(%f) [HitFracActor=%f] modified hit_power=%f",
                    ap, BoneArmor, m_boneProtection->m_fHitFracActor, NewHitPower);
        }
    }
    else
    {
        float one = 0.1f;
        if (hit_type == ALife::eHitTypeStrike || hit_type == ALife::eHitTypeWound ||
            hit_type == ALife::eHitTypeWound_2 || hit_type == ALife::eHitTypeExplosion)
        {
            one = 1.0f;
        }
        float protect = GetDefHitTypeProtection(hit_type);
        NewHitPower -= protect * one;

        if (NewHitPower < 0.f)
            NewHitPower = 0.f;

        if (Core.ParamFlags.test(Core.dbgbullet))
            Msg("CCustomOutfit::HitThroughArmor hit_type=%d | After HitTypeProtection(%f) hit_power=%f", (u32)hit_type,
                protect * one, NewHitPower);
    }

    //увеличить изношенность костюма
    Hit(hit_power, hit_type);

    if (Core.ParamFlags.test(Core.dbgbullet))
        Msg("CCustomOutfit::HitThroughArmor hit_type=%d | After Immunities hit_power=%f", (u32)hit_type, NewHitPower);

    return NewHitPower;
}

BOOL CBackpack::BonePassBullet(int boneID) 
{ 
    return m_boneProtection->getBonePassBullet(s16(boneID)); 
}

void CBackpack::OnMoveToSlot(const SInvItemPlace& previous_place)
{
    inherited::OnMoveToSlot(previous_place);
    /*
    if (m_pInventory && (previous_place.type == eItemPlaceSlot))
    {
    CActor* pActor = smart_cast<CActor*> (H_Parent());
    if (pActor)
    {
    }
    }
    */
}

void CBackpack::OnMoveToRuck(const SInvItemPlace& previous_place)
{
    inherited::OnMoveToRuck(previous_place);
    /*
    if (m_pInventory && (previous_place.type == eItemPlaceSlot))
    {
    CActor* pActor = smart_cast<CActor*> (H_Parent());
    if (pActor)
    {
    }
    }
    */
}

bool CBackpack::install_upgrade_impl(LPCSTR section, bool test)
{
    bool result = inherited::install_upgrade_impl(section, test);

    result |= process_if_exists(section, "power_restore_speed", &CInifile::r_float, m_fPowerRestoreSpeed, test);
    result |= process_if_exists(section, "power_loss", &CInifile::r_float, m_fPowerLoss, test);
    clamp(m_fPowerLoss, 0.0f, 1.0f);

    result |= process_if_exists(section, "additional_inventory_weight", &CInifile::r_float, m_additional_weight, test);
    result |= process_if_exists(section, "additional_inventory_weight2", &CInifile::r_float, m_additional_weight2, test);

    return result;
}

void CBackpack::AddBonesProtection(LPCSTR bones_section)
{
    IGameObject* parent = H_Parent();
    if (IsGameTypeSingle())
        parent = smart_cast<IGameObject*>(
            Level().CurrentViewEntity()); // TODO: FIX THIS OR NPC Can't wear outfit without resetting actor

    if (parent && parent->Visual() && m_BonesProtectionSect.size())
        m_boneProtection->add(bones_section, smart_cast<IKinematics*>(parent->Visual()));
}
