#include "pch_script.h"
#include "WeaponRPG26.h"

using namespace luabind;

#pragma optimize("s",on)
void CWeaponRPG26::script_register	(lua_State *L)
{
	module(L)
	[
		class_<CWeaponRPG26, CGameObject>("CWeaponRPG26")
			.def(constructor<>())
	];
}
