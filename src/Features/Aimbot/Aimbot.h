#pragma once
#include "..\..\Utils\GlobalVars.h"
#include "..\..\SDK\CGlobalVarsBase.h"
#include <deque>

class Aimbot
{
public:
	matrix3x4_t Matrix[65][128];
	int Sequence;
	void OnCreateMove();
	int bestEntDmg;
	void movement_stop();
private:
	bool Backtrack[65];
	bool ShotBacktrack[65];
	Vector Hitscan(C_BaseEntity* pEnt);
	bool HitChance(C_BaseEntity* pEnt, C_BaseCombatWeapon* pWeapon, Vector Angle, Vector Point, int chance);
};
extern Aimbot g_Aimbot;