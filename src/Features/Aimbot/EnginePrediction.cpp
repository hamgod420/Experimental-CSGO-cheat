#include "EnginePrediction.h"
#include "..\..\SDK\CInput.h"
#include "..\..\SDK\CEntity.h"
#include "..\..\Utils\GlobalVars.h"
#include "..\..\SDK\CPrediction.h"
#include "..\..\SDK\CGlobalVarsBase.h"
#include "..\..\Menu\Menu.h"

//outlassn https://www.unknowncheats.me/forum/1842735-post1.html and vrc https://www.unknowncheats.me/forum/2296204-post1.html

float flOldCurtime;
float flOldFrametime;
int *m_nPredictionRandomSeed;
int *m_pSetPredictionPlayer;
static bool bInit = false;

void engine_prediction::RunEnginePred()
{
	if (!g::pLocalEntity->IsAlive() || !g_pMoveHelper || !g_Menu.Config.Aimbot || g_Menu.Config.LegitBacktrack)
		return;

	if (!m_nPredictionRandomSeed || !m_pSetPredictionPlayer) {
		m_nPredictionRandomSeed = *reinterpret_cast<int**>(Utils::FindSignature("client.dll", "8B 0D ? ? ? ? BA ? ? ? ? E8 ? ? ? ? 83 C4 04") + 2);
		m_pSetPredictionPlayer = *reinterpret_cast<int**>(Utils::FindSignature("client.dll", "89 35 ? ? ? ? F3 0F 10 46") + 2);
	}

	CMoveData data;
	memset(&data, 0, sizeof(CMoveData));
	g::pLocalEntity->SetCurrentCommand(g::pCmd);
	g_pMoveHelper->SetHost(g::pLocalEntity);

	*m_nPredictionRandomSeed = g::pCmd->random_seed;
	*m_pSetPredictionPlayer = uintptr_t(g::pLocalEntity);

	*reinterpret_cast<uint32_t*>(reinterpret_cast<uint32_t>(g::pLocalEntity) + 0x3314) = reinterpret_cast<uint32_t>(g::pCmd); // lol
	*reinterpret_cast<uint32_t*>(reinterpret_cast<uint32_t>(g::pLocalEntity) + 0x326C) = reinterpret_cast<uint32_t>(g::pCmd); // lol

	flOldCurtime = global_vars->curtime;
	flOldFrametime = global_vars->frametime;

	g::uRandomSeed = *m_nPredictionRandomSeed;
	global_vars->curtime = g::pLocalEntity->GetTickBase() * global_vars->intervalPerTick;
	global_vars->frametime = global_vars->intervalPerTick;

	g_pMovement->StartTrackPredictionErrors(g::pLocalEntity);

	g_pPrediction->SetupMove(g::pLocalEntity, g::pCmd, g_pMoveHelper, &data);
	g_pMovement->ProcessMovement(g::pLocalEntity, &data);
	g_pPrediction->FinishMove(g::pLocalEntity, g::pCmd, &data);

	if (g::pLocalEntity->GetActiveWeapon())
		g::pLocalEntity->GetActiveWeapon()->AccuracyPenalty();
}

void engine_prediction::EndEnginePred()
{
	if (!g::pLocalEntity->IsAlive() || !g_pMoveHelper || !g_Menu.Config.Aimbot || g_Menu.Config.LegitBacktrack)
		return;

	g_pMovement->FinishTrackPredictionErrors(g::pLocalEntity);
	g_pMoveHelper->SetHost(nullptr);

	if (m_nPredictionRandomSeed || m_pSetPredictionPlayer)
	{
		*m_nPredictionRandomSeed = -1;
		*m_pSetPredictionPlayer = 0;
	}

	global_vars->curtime = flOldCurtime;
	global_vars->frametime = flOldFrametime;

	g::pLocalEntity->SetCurrentCommand(NULL);
}