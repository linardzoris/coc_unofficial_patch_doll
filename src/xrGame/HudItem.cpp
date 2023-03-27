#include "stdafx.h"
#include "HudItem.h"
#include "physic_item.h"
#include "Actor.h"
#include "ActorEffector.h"
#include "Missile.h"
#include "xrMessages.h"
#include "Level.h"
#include "Inventory.h"
#include "xrEngine/CameraBase.h"
#include "player_hud.h"
#include "xrCore/Animation/SkeletonMotions.hpp"

#include "xrUICore/ui_base.h"

#include "xrScriptEngine/script_callback_ex.h"
#include "script_game_object.h"
#include "HUDManager.h"

ENGINE_API extern float psHUD_FOV_def; //--#SM+#--

CHudItem::CHudItem()
{
    RenderHud(TRUE);
    EnableHudInertion(TRUE);
    AllowHudInertion(TRUE);
    m_bStopAtEndAnimIsRunning = false;
    m_current_motion_def = NULL;
    m_started_rnd_anim_idx = u8(-1);
	m_nearwall_last_hud_fov = psHUD_FOV_def;
}

IFactoryObject* CHudItem::_construct()
{
    m_object = smart_cast<CPhysicItem*>(this);
    VERIFY(m_object);

    m_item = smart_cast<CInventoryItem*>(this);
    VERIFY(m_item);

    return (m_object);
}

CHudItem::~CHudItem() {}

void CHudItem::Load(LPCSTR section)
{
    hud_sect = pSettings->r_string(section, "hud");
    m_animation_slot = pSettings->r_u32(section, "animation_slot");

    m_sounds.LoadSound(section, "snd_bore", "sndBore", true);

	m_strafe_offset[0] = READ_IF_EXISTS(pSettings, r_fvector3, hud_sect, "strafe_hud_offset_pos", Fvector().set(0.015f, 0.0f, 0.0f));
    m_strafe_offset[1] = READ_IF_EXISTS(pSettings, r_fvector3, hud_sect, "strafe_hud_offset_rot", Fvector().set(0.0f, 0.0f, 4.5f));

    float fFullStrafeTime = READ_IF_EXISTS(pSettings, r_float, hud_sect, "strafe_transition_time", 0.25f);
    bool bStrafeEnabled = !!READ_IF_EXISTS(pSettings, r_bool, hud_sect, "strafe_enabled", TRUE);

    m_strafe_offset[2].set(bStrafeEnabled ? 1.0f : 0.0f, fFullStrafeTime, 0.f); // normal

	m_hud_fov_add_mod = READ_IF_EXISTS(pSettings, r_float, hud_sect, "hud_fov_addition_modifier", 0.f);
    m_nearwall_dist_min = READ_IF_EXISTS(pSettings, r_float, hud_sect, "nearwall_dist_min", 0.5f);
    m_nearwall_dist_max = READ_IF_EXISTS(pSettings, r_float, hud_sect, "nearwall_dist_max", 1.f);
    m_nearwall_target_hud_fov = READ_IF_EXISTS(pSettings, r_float, hud_sect, "nearwall_target_hud_fov", 0.45f);
    m_nearwall_speed_mod = READ_IF_EXISTS(pSettings, r_float, hud_sect, "nearwall_speed_mod", 10.f);
}

void CHudItem::PlaySound(LPCSTR alias, const Fvector& position)
{
    m_sounds.PlaySound(alias, position, object().H_Root(), !!GetHUDmode());
}

//Alundaio: Play at index
void CHudItem::PlaySound(pcstr alias, const Fvector& position, u8 index)
{
    m_sounds.PlaySound(alias, position, object().H_Root(), !!GetHUDmode(), false, index);
}
//-Alundaio

void CHudItem::renderable_Render()
{
    UpdateXForm();
    BOOL _hud_render = GEnv.Render->get_HUD() && GetHUDmode();

    if (_hud_render && !IsHidden())
    {
    }
    else
    {
        if (!object().H_Parent() || (!_hud_render && !IsHidden()))
        {
            on_renderable_Render();
            debug_draw_firedeps();
        }
        else if (object().H_Parent())
        {
            CInventoryOwner* owner = smart_cast<CInventoryOwner*>(object().H_Parent());
            VERIFY(owner);
            CInventoryItem* self = smart_cast<CInventoryItem*>(this);
            if (owner->attached(self))
                on_renderable_Render();
        }
    }
}

void CHudItem::SwitchState(u32 S)
{
    if (OnClient())
        return;

    SetNextState(S);

    if (object().Local() && !object().getDestroy())
    {
        // !!! Just single entry for given state !!!
        NET_Packet P;
        object().u_EventGen(P, GE_WPN_STATE_CHANGE, object().ID());
        P.w_u8(u8(S));
        object().u_EventSend(P);
    }
}

void CHudItem::OnEvent(NET_Packet& P, u16 type)
{
    switch (type)
    {
    case GE_WPN_STATE_CHANGE:
    {
        u8 S;
        P.r_u8(S);
        OnStateSwitch(u32(S), GetState());
    }
    break;
    }
}

void CHudItem::OnStateSwitch(u32 S, u32 oldState)
{
    SetState(S);

    if (object().Remote())
        SetNextState(S);

    switch (S)
    {
    case eBore:
        SetPending(FALSE);

        PlayAnimBore();
        if (HudItemData())
        {
            Fvector P = HudItemData()->m_item_transform.c;
            m_sounds.PlaySound("sndBore", P, object().H_Root(), !!GetHUDmode(), false, m_started_rnd_anim_idx);
        }

        break;
    }
    g_player_hud->updateMovementLayerState();
}

void CHudItem::OnAnimationEnd(u32 state)
{
    CActor* A = smart_cast<CActor*>(object().H_Parent());
    if (A)
        A->callback(GameObject::eActorHudAnimationEnd)(smart_cast<CGameObject*>(this)->lua_game_object(),
                                                       this->hud_sect.c_str(), this->m_current_motion.c_str(), state,
                                                       this->animation_slot());

    switch (state)
    {
    case eBore: { SwitchState(eIdle);
    }
    break;
    }
}

void CHudItem::PlayAnimBore() { PlayHUDMotion("anm_bore", TRUE, this, GetState()); }
bool CHudItem::ActivateItem()
{
    OnActiveItem();
    return true;
}

void CHudItem::DeactivateItem() { OnHiddenItem(); }
void CHudItem::OnMoveToRuck(const SInvItemPlace& prev) { SwitchState(eHidden); }
void CHudItem::SendDeactivateItem() { SendHiddenItem(); }
void CHudItem::SendHiddenItem()
{
    if (!object().getDestroy())
    {
        NET_Packet P;
        object().u_EventGen(P, GE_WPN_STATE_CHANGE, object().ID());
        P.w_u8(u8(eHiding));
        object().u_EventSend(P);
    }
}

void __inertion(float& _val_cur, const float& _val_trgt, const float& _friction)
{
    float friction_i = 1.f - _friction;
    _val_cur = _val_cur * _friction + _val_trgt * friction_i;
}

void CHudItem::UpdateHudAdditonal(Fmatrix& hud_trans) 
{
    attachable_hud_item* hi = HudItemData();
    R_ASSERT(hi);

    CActor* pActor = smart_cast<CActor*>(object().H_Parent());
    if (!pActor)
        return;
}
void CHudItem::UpdateCL()
{
    if (m_current_motion_def)
    {
        if (m_bStopAtEndAnimIsRunning)
        {
            const xr_vector<motion_marks>& marks = m_current_motion_def->marks;
            if (!marks.empty())
            {
                float motion_prev_time = ((float)m_dwMotionCurrTm - (float)m_dwMotionStartTm) / 1000.0f;
                float motion_curr_time = ((float)Device.dwTimeGlobal - (float)m_dwMotionStartTm) / 1000.0f;

                xr_vector<motion_marks>::const_iterator it = marks.begin();
                xr_vector<motion_marks>::const_iterator it_e = marks.end();
                for (; it != it_e; ++it)
                {
                    const motion_marks& M = (*it);
                    if (M.is_empty())
                        continue;

                    const motion_marks::interval* Iprev = M.pick_mark(motion_prev_time);
                    const motion_marks::interval* Icurr = M.pick_mark(motion_curr_time);
                    if (Iprev == NULL && Icurr != NULL /* || M.is_mark_between(motion_prev_time, motion_curr_time)*/)
                    {
                        OnMotionMark(m_startedMotionState, M);
                    }
                }
            }

            m_dwMotionCurrTm = Device.dwTimeGlobal;
            if (m_dwMotionCurrTm > m_dwMotionEndTm)
            {
                m_current_motion_def = NULL;
                m_dwMotionStartTm = 0;
                m_dwMotionEndTm = 0;
                m_dwMotionCurrTm = 0;
                m_bStopAtEndAnimIsRunning = false;
                OnAnimationEnd(m_startedMotionState);
            }
        }
    }
}

void CHudItem::OnH_A_Chield() {}

void CHudItem::OnH_B_Chield() 
{ 
    StopCurrentAnimWithoutCallback();
    m_nearwall_last_hud_fov = psHUD_FOV_def;
}

void CHudItem::OnH_B_Independent(bool just_before_destroy)
{
    m_sounds.StopAllSounds();
    UpdateXForm();
    m_nearwall_last_hud_fov = psHUD_FOV_def;

    // next code was commented
    /*
    if(HudItemData() && !just_before_destroy)
    {
        object().XFORM().set( HudItemData()->m_item_transform );
    }

    if (HudItemData())
    {
        g_player_hud->detach_item(this);
        Msg("---Detaching hud item [%s][%d]", this->HudSection().c_str(), this->object().ID());
    }*/
    // SetHudItemData			(NULL);
}

void CHudItem::OnH_A_Independent()
{
    if (HudItemData())
        g_player_hud->detach_item(this);
    StopCurrentAnimWithoutCallback();
}

void CHudItem::on_b_hud_detach() { m_sounds.StopAllSounds(); }
void CHudItem::on_a_hud_attach()
{
    if (m_current_motion_def)
    {
        PlayHUDMotion_noCB(m_current_motion, FALSE);
#ifdef DEBUG
//		Msg("continue playing [%s][%d]",m_current_motion.c_str(), Device.dwFrame);
#endif // #ifdef DEBUG
    }
    else
    {
#ifdef DEBUG
//		Msg("no active motion");
#endif // #ifdef DEBUG
    }
}

u32 CHudItem::PlayHUDMotion(const shared_str& M, bool bMixIn, CHudItem* W, u32 state)
{
    u32 anim_time = PlayHUDMotion_noCB(M, bMixIn);
    if (anim_time > 0)
    {
        m_bStopAtEndAnimIsRunning = true;
        m_dwMotionStartTm = Device.dwTimeGlobal;
        m_dwMotionCurrTm = m_dwMotionStartTm;
        m_dwMotionEndTm = m_dwMotionStartTm + anim_time;
        m_startedMotionState = state;
    }
    else
        m_bStopAtEndAnimIsRunning = false;

    return anim_time;
}

u32 CHudItem::PlayHUDMotionNew(const shared_str& M, const bool bMixIn, const u32 state, const bool randomAnim)
{
    // Msg("~~[%s] Playing motion [%s] for [%s]", __FUNCTION__, M.c_str(), HudSection().c_str());
    u32 anim_time = PlayHUDMotion_noCB(M, bMixIn);
    if (anim_time > 0)
    {
        m_bStopAtEndAnimIsRunning = true;
        m_dwMotionStartTm = Device.dwTimeGlobal;
        m_dwMotionCurrTm = m_dwMotionStartTm;
        m_dwMotionEndTm = m_dwMotionStartTm + anim_time;
        m_startedMotionState = state;
    }
    else
        m_bStopAtEndAnimIsRunning = false;

    return anim_time;
}

u32 CHudItem::PlayHUDMotionIfExists(std::initializer_list<const char*> Ms, const bool bMixIn, const u32 state, const bool randomAnim)
{
    for (const auto* M : Ms)
        if (isHUDAnimationExist(M))
            return PlayHUDMotionNew(M, bMixIn, state, randomAnim);

    std::string dbg_anim_name;
    for (const auto* M : Ms)
    {
        dbg_anim_name += M;
        dbg_anim_name += ", ";
    }
    Msg("~~[%s] Motions [%s] not found for [%s]", __FUNCTION__, dbg_anim_name.c_str(), HudSection().c_str());

    return 0;
}

u32 CHudItem::PlayHUDMotion_noCB(const shared_str& motion_name, const bool bMixIn, const bool randomAnim)
{
    m_current_motion = motion_name;

    if (bDebug && item().m_pInventory)
    {
        Msg("-[%s] as[%d] [%d]anim_play [%s][%d]", HudItemData() ? "HUD" : "Simulating",
            item().m_pInventory->GetActiveSlot(), item().object_id(), motion_name.c_str(), Device.dwFrame);
    }
    if (HudItemData())
    {
        return HudItemData()->anim_play(motion_name, bMixIn, m_current_motion_def, m_started_rnd_anim_idx);
    }
    else
    {
        m_started_rnd_anim_idx = 0;
        return g_player_hud->motion_length(motion_name, HudSection(), m_current_motion_def);
    }
}

void CHudItem::StopCurrentAnimWithoutCallback()
{
    m_dwMotionStartTm = 0;
    m_dwMotionEndTm = 0;
    m_dwMotionCurrTm = 0;
    m_bStopAtEndAnimIsRunning = false;
    m_current_motion_def = NULL;
}

BOOL CHudItem::GetHUDmode()
{
    if (object().H_Parent())
    {
        CActor* A = smart_cast<CActor*>(object().H_Parent());
        return (A && A->HUDview() && HudItemData());
    }
    else
        return FALSE;
}

void CHudItem::PlayAnimIdle()
{
    if (TryPlayAnimIdle())
        return;

    PlayHUDMotion("anm_idle", TRUE, NULL, GetState());
}

bool CHudItem::TryPlayAnimIdle()
{
    if (MovingAnimAllowedNow())
    {
        CActor* pActor = smart_cast<CActor*>(object().H_Parent());
        if (pActor)
        {
            CEntity::SEntityState st;
            pActor->g_State(st);
            if (st.bSprint)
            {
                PlayAnimIdleSprint();
                return true;
            }
            if (pActor->AnyMove())
            {
                if (!st.bCrouch)
                {
                    PlayAnimIdleMoving();
                    return true;
                }
#ifdef NEW_ANIMS //AVO: new crouch idle animation
                if (st.bCrouch && isHUDAnimationExist("anm_idle_moving_crouch"))
                {
                    PlayAnimIdleMovingCrouch();
                    return true;
                }
#endif //-NEW_ANIMS
            }
        }
    }
    return false;
}

//AVO: check if animation exists
bool CHudItem::isHUDAnimationExist(pcstr anim_name) const
{
    if (HudItemData()) // First person
    {
        string256 anim_name_r;
        bool is_16x9 = UI().is_widescreen();
        u16 attach_place_idx = pSettings->r_u16(HudItemData()->m_sect_name, "attach_place_idx");
        xr_sprintf(anim_name_r, "%s%s", anim_name, (attach_place_idx == 1 && is_16x9) ? "_16x9" : "");
        player_hud_motion* anm = HudItemData()->m_hand_motions.find_motion(anim_name_r);
        if (anm)
            return true;
    }
    else // Third person
    {
        const CMotionDef* temp_motion_def;
        if (g_player_hud->motion_length(anim_name, HudSection(), temp_motion_def) > 100)
            return true;
    }
#ifdef DEBUG
    Msg("~ [WARNING] ------ Animation [%s] does not exist in [%s]", anim_name, HudSection().c_str());
#endif
    return false;
}

void CHudItem::PlayAnimIdleMovingCrouch() 
{ 
    PlayHUDMotion("anm_idle_moving_crouch", false, nullptr, GetState()); 
}

bool CHudItem::NeedBlendAnm()
{
    u32 state = GetState();
    return (state != eIdle && state != eHidden);
}

void CHudItem::PlayAnimIdleMoving() 
{ 
    PlayHUDMotion("anm_idle_moving", false, nullptr, GetState()); 
}

void CHudItem::PlayAnimIdleSprint() 
{ 
    PlayHUDMotion("anm_idle_sprint", false, nullptr, GetState()); 
}

void CHudItem::OnMovementChanged(ACTOR_DEFS::EMoveCommand cmd)
{
    if (GetState() == eIdle && !m_bStopAtEndAnimIsRunning)
    {
        if ((cmd == ACTOR_DEFS::mcSprint) || (cmd == ACTOR_DEFS::mcAnyMove))
        {
            PlayAnimIdle();
            ResetSubStateTime();
        }
    }
}

attachable_hud_item* CHudItem::HudItemData() const
{
    attachable_hud_item* hi = NULL;
    if (!g_player_hud)
        return hi;

    hi = g_player_hud->attached_item(0);
    if (hi && hi->m_parent_hud_item == this)
        return hi;

    hi = g_player_hud->attached_item(1);
    if (hi && hi->m_parent_hud_item == this)
        return hi;

    return NULL;
}

BOOL CHudItem::ParentIsActor()
{
    IGameObject* O = object().H_Parent();
    if (!O)
        return false;

    CEntityAlive* EA = smart_cast<CEntityAlive*>(O);
    if (!EA)
        return false;

    return !!EA->cast_actor();
}

void CHudItem::ReplaceHudSection(LPCSTR hud_section)
{
    if (hud_section != hud_sect)
        hud_sect = hud_section;
}

float CHudItem::GetHudFov()
{
    if (smart_cast<CActor*>(this->object().H_Parent()) && (Level().CurrentViewEntity() == object().H_Parent()))
    {
        collide::rq_result& RQ = HUD().GetCurrentRayQuery();
        float dist = RQ.range;

        clamp(dist, m_nearwall_dist_min, m_nearwall_dist_max);
        float fDistanceMod =
            ((dist - m_nearwall_dist_min) / (m_nearwall_dist_max - m_nearwall_dist_min)); // 0.f ... 1.f

        float fBaseFov = psHUD_FOV_def + m_hud_fov_add_mod;
        clamp(fBaseFov, 0.0f, FLT_MAX);

        float src = m_nearwall_speed_mod * Device.fTimeDelta;
        clamp(src, 0.f, 1.f);

        float fTrgFov = m_nearwall_target_hud_fov + fDistanceMod * (fBaseFov - m_nearwall_target_hud_fov);
        m_nearwall_last_hud_fov = m_nearwall_last_hud_fov * (1 - src) + fTrgFov * src;
    }
    return m_nearwall_last_hud_fov;
}
