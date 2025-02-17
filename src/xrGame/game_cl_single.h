#pragma once
#include "game_cl_base.h"

class game_cl_Single : public game_cl_GameState
{
    typedef game_cl_GameState inherited;

public:
    game_cl_Single();

    virtual LPCSTR type_name() const { return "single"; };

    virtual CUIGameCustom* createGameUI();
    virtual bool IsServerControlHits() { return true; };
    virtual ALife::_TIME_ID GetStartGameTime();
    virtual ALife::_TIME_ID GetGameTime();
    virtual float GetGameTimeFactor();
    virtual void SetGameTimeFactor(const float fTimeFactor);

    virtual ALife::_TIME_ID GetEnvironmentGameTime();
    virtual float GetEnvironmentGameTimeFactor();
    virtual void SetEnvironmentGameTimeFactor(const float fTimeFactor);

    void OnDifficultyChanged();
};

// game difficulty
enum ESingleGameDifficulty
{
    egdNovice = 0,
    egdMaster = 1,
    egdCount,
    egd_force_u32 = u32(-1)
};

extern ESingleGameDifficulty g_SingleGameDifficulty;
extern const xr_token difficulty_type_token[];
