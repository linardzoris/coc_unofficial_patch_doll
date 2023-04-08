#include "stdafx.h"
#include "WeaponPistol.h"
#include "ParticlesObject.h"
#include "Actor.h"

CWeaponPistol::CWeaponPistol()
{
    m_eSoundClose = ESoundTypes(SOUND_TYPE_WEAPON_RECHARGING);
    SetPending(FALSE);
}

CWeaponPistol::~CWeaponPistol(void) {}
void CWeaponPistol::net_Destroy() { inherited::net_Destroy(); }
void CWeaponPistol::Load(LPCSTR section)
{
    inherited::Load(section);

    m_sounds.LoadSound(section, "snd_close", "sndClose", false, m_eSoundClose);
}

void CWeaponPistol::OnH_B_Chield() { inherited::OnH_B_Chield(); }
void CWeaponPistol::PlayAnimShow()
{
    VERIFY(GetState() == eShowing);

    inherited::PlayAnimShow();
}

void CWeaponPistol::PlayAnimBore()
{
    inherited::PlayAnimBore();
}

void CWeaponPistol::PlayAnimIdleSprint()
{
    inherited::PlayAnimIdleSprint();
}

void CWeaponPistol::PlayAnimIdleMoving()
{
    inherited::PlayAnimIdleMoving();
}

void CWeaponPistol::PlayAnimIdleMovingCrouch() 
{ 
    inherited::PlayAnimIdleMovingCrouch(); 
}

void CWeaponPistol::PlayAnimIdle()
{
    if (TryPlayAnimIdle())
        return;

    inherited::PlayAnimIdle();
}

void CWeaponPistol::PlayAnimAim()
{
    inherited::PlayAnimAim();
}

void CWeaponPistol::PlayAnimReload()
{
    inherited::PlayAnimReload(); //AVO: refactored to use grand-parent (CWeaponMagazined) function
}

void CWeaponPistol::PlayAnimHide()
{
    VERIFY(GetState() == eHiding);

    inherited::PlayAnimHide();
}

void CWeaponPistol::PlayAnimShoot()
{
    VERIFY(GetState() == eFire);

    inherited::PlayAnimShoot();
}

void CWeaponPistol::switch2_Reload() { inherited::switch2_Reload(); }
void CWeaponPistol::OnAnimationEnd(u32 state) { inherited::OnAnimationEnd(state); }
void CWeaponPistol::OnShot()
{
    inherited::OnShot(); //Alundaio: not changed from inherited, so instead of copying changes from weaponmagazined, we just do this
}

void CWeaponPistol::UpdateSounds()
{
    inherited::UpdateSounds();
    m_sounds.SetPosition("sndClose", get_LastFP());
}
