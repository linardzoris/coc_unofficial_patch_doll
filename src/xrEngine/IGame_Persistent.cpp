#include "stdafx.h"
#pragma hdrstop

#include "IGame_Persistent.h"
#include "GameFont.h"
#include "PerformanceAlert.hpp"

#ifndef _EDITOR
#include "Environment.h"
#include "x_ray.h"
#include "IGame_Level.h"
#include "XR_IOConsole.h"
#include "Render.h"
#include "PS_instance.h"
#include "CustomHUD.h"
#endif

#include "editor_environment_manager.hpp"

extern Fvector4 ps_ssfx_grass_interactive;
ENGINE_API IGame_Persistent* g_pGamePersistent = nullptr;

bool IGame_Persistent::IsMainMenuActive()
{
    return g_pGamePersistent && g_pGamePersistent->m_pMainMenu && g_pGamePersistent->m_pMainMenu->IsActive();
}

bool IGame_Persistent::MainMenuActiveOrLevelNotExist()
{
    return !g_pGameLevel || g_pGamePersistent->m_pMainMenu && g_pGamePersistent->m_pMainMenu->IsActive();
}

IGame_Persistent::IGame_Persistent()
{
    RDEVICE.seqAppStart.Add(this);
    RDEVICE.seqAppEnd.Add(this);
    RDEVICE.seqFrame.Add(this, REG_PRIORITY_HIGH + 1);
    RDEVICE.seqAppActivate.Add(this);
    RDEVICE.seqAppDeactivate.Add(this);

    m_pMainMenu = nullptr;

    if (RDEVICE.editor())
        pEnvironment = new editor::environment::manager();
    else
        pEnvironment = new CEnvironment();

    m_pGShaderConstants = new ShadersExternalData(); //--#SM+#--
}

IGame_Persistent::~IGame_Persistent()
{
    RDEVICE.seqFrame.Remove(this);
    RDEVICE.seqAppStart.Remove(this);
    RDEVICE.seqAppEnd.Remove(this);
    RDEVICE.seqAppActivate.Remove(this);
    RDEVICE.seqAppDeactivate.Remove(this);
#ifndef _EDITOR
    xr_delete(pEnvironment);
#endif

    xr_delete(m_pGShaderConstants); //--#SM+#--
}

void IGame_Persistent::OnAppActivate() {}
void IGame_Persistent::OnAppDeactivate() {}
void IGame_Persistent::OnAppStart()
{
#ifndef _EDITOR
    Environment().load();
#endif
}

void IGame_Persistent::OnAppEnd()
{
#ifndef _EDITOR
    Environment().unload();
#endif
    OnGameEnd();

#ifndef _EDITOR
    DEL_INSTANCE(g_hud);
#endif
}

void IGame_Persistent::PreStart(LPCSTR op)
{
    string256 prev_type;
    params new_game_params;
    xr_strcpy(prev_type, m_game_params.m_game_type);
    new_game_params.parse_cmd_line(op);

    // change game type
    if (0 != xr_strcmp(prev_type, new_game_params.m_game_type))
    {
        OnGameEnd();
    }
}
void IGame_Persistent::Start(LPCSTR op)
{
    string256 prev_type;
    xr_strcpy(prev_type, m_game_params.m_game_type);
    m_game_params.parse_cmd_line(op);
    // change game type
    if ((0 != xr_strcmp(prev_type, m_game_params.m_game_type)))
    {
        if (*m_game_params.m_game_type)
            OnGameStart();
#ifndef _EDITOR
        if (g_hud)
            DEL_INSTANCE(g_hud);
#endif
    }
    else
        UpdateGameType();

    VERIFY(ps_destroy.empty());
}

void IGame_Persistent::Disconnect()
{
#ifndef _EDITOR
    // clear "need to play" particles
    destroy_particles(true);

    if (g_hud)
        DEL_INSTANCE(g_hud);
//. g_hud->OnDisconnected ();
#endif
}

void IGame_Persistent::OnGameStart()
{
#ifndef _EDITOR
    SetLoadStageTitle("st_prefetching_objects");
    LoadTitle();
    if (!strstr(Core.Params, "-noprefetch"))
        Prefetch();
#endif
}

#ifndef _EDITOR
void IGame_Persistent::Prefetch()
{
    // prefetch game objects & models
    CTimer timer;
    timer.Start();
    const auto memoryBefore = Memory.mem_usage();

    Log("Loading objects...");
    ObjectPool.prefetch();

    Log("Loading models...");
    GEnv.Render->models_Prefetch();

    Log("Loading textures...");
    GEnv.Render->ResourcesDeferredUpload();

    const auto memoryAfter = Memory.mem_usage() - memoryBefore;

    Msg("* [prefetch] time:   %d ms", timer.GetElapsed_ms());
    Msg("* [prefetch] memory: %d Kb", memoryAfter / 1024);
}
#endif

void IGame_Persistent::OnGameEnd()
{
#ifndef _EDITOR
    ObjectPool.clear();
    GEnv.Render->models_Clear(TRUE);
#endif
}

void IGame_Persistent::OnFrame()
{
#ifndef _EDITOR
    if (!Device.Paused() || Device.dwPrecacheFrame)
        Environment().OnFrame();

    stats.Starting = ps_needtoplay.size();
    stats.Active = ps_active.size();
    stats.Destroying = ps_destroy.size();
    // Play req particle systems
    while (ps_needtoplay.size())
    {
        CPS_Instance* psi = ps_needtoplay.back();
        ps_needtoplay.pop_back();
        psi->Play(false);
    }
    // Destroy inactive particle systems
    while (ps_destroy.size())
    {
        // u32 cnt = ps_destroy.size();
        CPS_Instance* psi = ps_destroy.back();
        VERIFY(psi);
        if (psi->Locked())
        {
            Log("--locked");
            break;
        }
        ps_destroy.pop_back();
        psi->PSI_internal_delete();
    }
#endif
}

void IGame_Persistent::destroy_particles(const bool& all_particles)
{
#ifndef _EDITOR

    ps_needtoplay.clear();

	xr_set<CPS_Instance*>::iterator I = ps_active.begin();
    xr_set<CPS_Instance*>::iterator E = ps_active.end();
    for (; I != E; ++I)
    {
        if (all_particles || (*I)->destroy_on_game_load())
            (*I)->PSI_destroy();
    }
    while (ps_destroy.size())
    {
        CPS_Instance* psi = ps_destroy.back();
        VERIFY(psi);
        VERIFY(!psi->Locked());
        ps_destroy.pop_back();
        psi->PSI_internal_delete();
    }

    VERIFY(ps_needtoplay.empty() && ps_destroy.empty() && (!all_particles || ps_active.empty()));
#endif
}

void IGame_Persistent::OnAssetsChanged()
{
#ifndef _EDITOR
    GEnv.Render->OnAssetsChanged();
#endif
}

void IGame_Persistent::DumpStatistics(IGameFont& font, IPerformanceAlert* alert)
{
    // XXX: move to particle engine
    stats.FrameEnd();
    font.OutNext("Particles:");
    font.OutNext("- starting:   %u", stats.Starting);
    font.OutNext("- active:     %u", stats.Active);
    font.OutNext("- destroying: %u", stats.Destroying);
    stats.FrameStart();
}

void IGame_Persistent::GrassBendersUpdate(u16 id, u8& data_idx, u32& data_frame, Fvector& position)
{
    // Interactive grass disabled
    if (ps_ssfx_grass_interactive.y < 1)
        return;

    if (RDEVICE.dwFrame < data_frame)
    {
        // Just update position if not NULL
        if (data_idx != NULL)
        {
            // Explosions can take the mem spot, unassign and try to get a spot later.
            if (grass_shader_data.id[data_idx] != id)
            {
                data_idx = NULL;
                data_frame = RDEVICE.dwFrame + Random.randI(10, 35);
            }
            else
            {
                // Just Update... ( FadeIn if str < 1.0f )
                if (grass_shader_data.str[data_idx] < 1.0f)
                    grass_shader_data.str[data_idx] += 0.5f * Device.fTimeDelta;
                else
                    grass_shader_data.str[data_idx] = 1.0f;

                grass_shader_data.pos[data_idx] = position;
            }
            return;
        }

        // Wait some random frames to split the checks
        data_frame = RDEVICE.dwFrame + Random.randI(10, 35);

        // Check Distance
        if (position.distance_to_xz_sqr(Device.vCameraPosition) > ps_ssfx_grass_interactive.z)
        {
            GrassBendersRemoveByIndex(data_idx);
            return;
        }

        CFrustum& view_frust = GEnv.Render->ViewBase;
        u32 mask = 0xff;

        // In view frustum?
        if (!view_frust.testSphere(position, 1, mask))
        {
            GrassBendersRemoveByIndex(data_idx);
            return;
        }

        // Empty slot, let's use this
        if (data_idx == NULL)
        {
            u8 idx = grass_shader_data.index + 1;

            // Add to grass blenders array
            if (grass_shader_data.id[idx] == NULL)
            {
                data_idx = idx;
                GrassBendersSet(idx, id, position, Fvector3().set(0, -99, 0), 0, 0, 0.0f, NULL, true);
                grass_shader_data.radius_curr[idx] = -1.0f;
            }

            // Back to 0 when the array limit is reached
            grass_shader_data.index = idx < ps_ssfx_grass_interactive.y ? idx : 0;
        }
        else
        {
            // Already inview, let's add more time to re-check
            data_frame += 60;
            grass_shader_data.pos[data_idx] = position;
        }
    }
}

void IGame_Persistent::GrassBendersAddExplosion(
    u16 id, Fvector position, Fvector3 dir, float fade, float speed, float intensity, float radius)
{
    if (ps_ssfx_grass_interactive.y < 1)
        return;

    for (int idx = 1; idx < ps_ssfx_grass_interactive.y + 1; idx++)
    {
        // Add explosion to any spot not already taken by an explosion.
        if (grass_shader_data.radius[idx] == NULL)
        {
            // Add 99 to avoid conflicts between explosions and basic benders.
            GrassBendersSet(idx, id + 99, position, dir, fade, speed, intensity, radius, true);
            grass_shader_data.str_target[idx] = intensity;
            break;
        }
    }
}

void IGame_Persistent::GrassBendersAddShot(
    u16 id, Fvector position, Fvector3 dir, float fade, float speed, float intensity, float radius)
{
    // Is disabled?
    if (ps_ssfx_grass_interactive.y < 1 || intensity <= 0.0f)
        return;

    // Check distance
    if (position.distance_to_xz_sqr(Device.vCameraPosition) > ps_ssfx_grass_interactive.z)
        return;

    int AddAt = -1;

    // Look for a spot
    for (int idx = 1; idx < ps_ssfx_grass_interactive.y + 1; idx++)
    {
        // Already exist, just update and increase intensity
        if (grass_shader_data.id[idx] == id)
        {
            float currentSTR = grass_shader_data.str[idx];
            GrassBendersSet(idx, id, position, dir, fade, speed, currentSTR, radius, false);
            grass_shader_data.str_target[idx] += intensity;
            AddAt = -1;
            break;
        }
        else
        {
            // Check all index and keep usable index to use later if needed...
            if (AddAt == -1 && grass_shader_data.radius[idx] == NULL)
                AddAt = idx;
        }
    }

    // We got an available index... Add bender at AddAt
    if (AddAt != -1)
    {
        GrassBendersSet(AddAt, id, position, dir, fade, speed, 0.001f, radius, true);
        grass_shader_data.str_target[AddAt] = intensity;
    }
}

void IGame_Persistent::GrassBendersUpdateExplosions()
{
    for (int idx = 1; idx < ps_ssfx_grass_interactive.y + 1; idx++)
    {
        if (grass_shader_data.radius[idx] != NULL)
        {
            // Radius
            grass_shader_data.time[idx] += Device.fTimeDelta * grass_shader_data.speed[idx];
            grass_shader_data.radius_curr[idx] =
                grass_shader_data.radius[idx] * std::min(1.0f, grass_shader_data.time[idx]);

            grass_shader_data.str_target[idx] = std::min(1.0f, grass_shader_data.str_target[idx]);

            // Easing
            float diff = abs(grass_shader_data.str[idx] - grass_shader_data.str_target[idx]);
            diff = std::max(0.1f, diff);

            // Intensity
            if (grass_shader_data.str_target[idx] <= grass_shader_data.str[idx])
            {
                grass_shader_data.str[idx] -= Device.fTimeDelta * grass_shader_data.fade[idx] * diff;
            }
            else
            {
                grass_shader_data.str[idx] += Device.fTimeDelta * grass_shader_data.speed[idx] * diff;

                if (grass_shader_data.str[idx] >= grass_shader_data.str_target[idx])
                    grass_shader_data.str_target[idx] = 0;
            }

            // Remove Bender
            if (grass_shader_data.str[idx] < 0.0f)
                GrassBendersReset(idx);
        }
    }
}

void IGame_Persistent::GrassBendersRemoveByIndex(u8& idx)
{
    if (idx != NULL)
    {
        GrassBendersReset(idx);
        idx = NULL;
    }
}

void IGame_Persistent::GrassBendersRemoveById(u16 id)
{
    // Search by Object ID ( Used when removing benders CPHMovementControl::DestroyCharacter() )
    for (int i = 1; i < ps_ssfx_grass_interactive.y + 1; i++)
        if (grass_shader_data.id[i] == id)
            GrassBendersReset(i);
}

void IGame_Persistent::GrassBendersReset(u8 idx)
{
    GrassBendersSet(idx, NULL, {0, 0, 0}, Fvector3().set(0, -99, 0), 0, 0, 1, NULL, true);
}

void IGame_Persistent::GrassBendersSet(u8 idx, u16 id, Fvector position, Fvector3 dir, float fade, float speed,
    float intensity, float radius, bool resetTime)
{
    // Set values
    grass_shader_data.pos[idx] = position;
    grass_shader_data.id[idx] = id;
    grass_shader_data.radius[idx] = radius;
    grass_shader_data.str[idx] = intensity;
    grass_shader_data.fade[idx] = fade;
    grass_shader_data.speed[idx] = speed;
    grass_shader_data.dir[idx] = dir;

    if (resetTime)
    {
        grass_shader_data.radius_curr[idx] = 0.01f;
        grass_shader_data.time[idx] = 0;
    }
}
