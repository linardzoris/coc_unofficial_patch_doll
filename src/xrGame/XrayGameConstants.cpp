#include "StdAfx.h"
#include "XrayGameConstants.h"
#include "GamePersistent.h"
#include "game_cl_single.h"

bool m_bSSS_DoF = false;

Fvector4 m_FV4DefaultDoF = Fvector4().set(0.1f, 0.25f, 0.0f, 0.0f);
Fvector4 m_FV4FocusDoF = Fvector4().set(0.1f, 0.25f, 0.0f, 0.0f);

namespace GameConstants
{
	void LoadConstants()
	{
        m_bSSS_DoF = READ_IF_EXISTS(pSettings, r_bool, "gameplay", "SSS_DoF", false);
		m_FV4DefaultDoF = READ_IF_EXISTS(pSettings, r_fvector4, "gameplay", "SSS_default_dof", Fvector4().set(0.1f, 0.25f, 0.0f, 0.0f));
		m_FV4FocusDoF = READ_IF_EXISTS(pSettings, r_fvector4, "gameplay", "SSS_focus_dof", Fvector4().set(0.1f, 0.25f, 0.0f, 0.0f));
	}

	bool GetSSS_DoF()
	{
		return m_bSSS_DoF;
	}

	Fvector4 GetSSFX_DefaultDoF()
	{
		return m_FV4DefaultDoF;
	}

	Fvector4 GetSSFX_FocusDoF()
	{
		return m_FV4FocusDoF;
	}
}
