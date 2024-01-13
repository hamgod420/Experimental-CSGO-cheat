#include "..\Aimbot\Aimbot.h"
#include "..\Aimbot\Autowall.h"
#include "..\Aimbot\LagComp.h"
#include "..\..\Utils\Utils.h"
#include "..\..\SDK\IVEngineClient.h"
#include "..\..\SDK\Hitboxes.h"
#include "..\..\SDK\PlayerInfo.h"
#include "..\..\Utils\Math.h"
#include "..\..\Menu\Menu.h"
#include "missed_calculation.h"
#include "../../SDK/IVModelInfo.h"
#include "../../SDK/ICvar.h"
MissedShots* g_MissedShots = new MissedShots();
void MissedShots::AddSnapshot(C_BaseEntity* player)
{
	ShotInfo Shot;

	Shot.Player = player;
	Shot.ShootPos = g::pLocalEntity->GetEyePosition();
	Shot.TimeFired = global_vars->curtime;
	Shot.Hitgroup = -1;
	Shot.AimPos = g::LastAimVec;
	Shot.ResolverStage = g::ResolverStage[player->EntIndex()];
	std::memcpy(Shot.Matrix, g::LastAimMatrix, sizeof(g::LastAimMatrix));

	ShotRecords.push_back(Shot);
}

void MissedShots::FSN(ClientFrameStage_t curStage)
{
	if (curStage == FRAME_RENDER_START)
	{
		g::pLocalEntity = g_pEntityList->GetClientEntity(engine->GetLocalPlayer());
		g_MissedShots->ProcessSnapshots();
	}
}

void MissedShots::ProcessSnapshots()
{
	if (ShotRecords.empty() || BulletImpacts.empty())
		return;

	auto Shot = ShotRecords.front();
	if (abs(global_vars->curtime - Shot.TimeFired) > 1.f) {
		ShotRecords.erase(ShotRecords.begin());
		return;
	}

	studiohdr_t* studioHdr = g_pModelInfo->GetStudiomodel(Shot.Player->GetModel());
	if (!studioHdr)
		return;

	mstudiohitboxset_t* set = studioHdr->GetHitboxSet(0);
	if (!set)
		return;

	std::vector<CSphere> m_cSpheres;
	std::vector<COBB> m_cOBBs;
	C_Trace Trace;
	Vector vMin, vMax;

	Vector BulletHitPos;
	float LowestDistance = 100000;

	for (size_t i = 0; i < BulletImpacts.size(); i++) {
		auto current_impact = BulletImpacts.at(i);

		if (current_impact.DistTo(Shot.Player->GetOrigin()) < LowestDistance)
		{
			LowestDistance = current_impact.DistTo(Shot.Player->GetOrigin());
			BulletHitPos = current_impact;
		}
	}

	// we need to use bullet_land_pos but because of shitty things we have to make changes..
	Vector Pos = BulletHitPos.DistTo(Shot.AimPos) <= 6.f ? BulletHitPos : BulletImpacts.back();

	// we hit them
	if (Shot.Hitgroup >= 0)
	{
		g_pCvar->ConsoleColorPrintf(Color(65, 135, 245), "[clockwork] ");
		Color color = Color(255, 255, 255);

		g_pCvar->ConsoleColorPrintf(color, "-%i dealt (%i health left) at %s\n", Shot.Damage, Shot.Player->GetHealth(), Shot.ResolverStage.c_str());
		HitmarkerInfo.push_back({ Shot.AimPos, BulletHitPos, global_vars->curtime + 4.f, 255 });
	}
	else if (Shot.TimeProcessed != 0.f) /// missed
	{
		g_pCvar->ConsoleColorPrintf(Color(65, 135, 245), "[clockwork] ");
		Color color = Color(255, 255, 255);

		if (Trace.m_pEnt == Shot.Player) {
			g_pCvar->ConsoleColorPrintf(color, "missed shot due to bad resolve at %s\n", Shot.ResolverStage.c_str());
			g::MissedShotsResolve[Shot.Player->EntIndex()]++;
		}
		else
			g_pCvar->ConsoleColorPrintf(color, "missed shot due to spread at %s\n", Shot.ResolverStage.c_str());
	}
	else
		return;
		return;

	ShotRecords.erase(ShotRecords.begin());
}