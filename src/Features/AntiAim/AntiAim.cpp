#include "AntiAim.h"
#include "..\Aimbot\Autowall.h"
#include "..\..\Utils\Utils.h"
#include "..\..\SDK\IVEngineClient.h"
#include "..\..\SDK\PlayerInfo.h"
#include "..\..\Utils\Math.h"
#include "..\..\Menu\Menu.h"
#include "../Aimbot/Aimbot.h"
AntiAim g_AntiAim;

bool Swtich = false;

void FreeStanding() // cancer v1
{
	static float FinalAngle;
	bool bside1 = false;
	bool bside2 = false;
	bool autowalld = false;
	for (int i = 1; i <= engine->GetMaxClients(); ++i)
	{
		C_BaseEntity* pPlayerEntity = g_pEntityList->GetClientEntity(i);

		if (!pPlayerEntity
			|| !pPlayerEntity->IsAlive()
			|| pPlayerEntity->IsDormant()
			|| pPlayerEntity == g::pLocalEntity
			|| pPlayerEntity->GetTeam() == g::pLocalEntity->GetTeam())
			continue;

		float angToLocal = math.CalcAngle(g::pLocalEntity->GetOrigin(), pPlayerEntity->GetOrigin()).y;
		Vector ViewPoint = pPlayerEntity->GetOrigin() + Vector(0, 0, 90);

		Vector2D Side1 = { (45 * sin(math.GRD_TO_BOG(angToLocal))),(45 * cos(math.GRD_TO_BOG(angToLocal))) };
		Vector2D Side2 = { (45 * sin(math.GRD_TO_BOG(angToLocal + 180))) ,(45 * cos(math.GRD_TO_BOG(angToLocal + 180))) };

		Vector2D Side3 = { (50 * sin(math.GRD_TO_BOG(angToLocal))),(50 * cos(math.GRD_TO_BOG(angToLocal))) };
		Vector2D Side4 = { (50 * sin(math.GRD_TO_BOG(angToLocal + 180))) ,(50 * cos(math.GRD_TO_BOG(angToLocal + 180))) };

		Vector Origin = g::pLocalEntity->GetOrigin();

		Vector2D OriginLeftRight[] = { Vector2D(Side1.x, Side1.y), Vector2D(Side2.x, Side2.y) };

		Vector2D OriginLeftRightLocal[] = { Vector2D(Side3.x, Side3.y), Vector2D(Side4.x, Side4.y) };

		for (int side = 0; side < 2; side++)
		{
			Vector OriginAutowall = { Origin.x + OriginLeftRight[side].x,  Origin.y - OriginLeftRight[side].y , Origin.z + 80 };
			Vector OriginAutowall2 = { ViewPoint.x + OriginLeftRightLocal[side].x,  ViewPoint.y - OriginLeftRightLocal[side].y , ViewPoint.z };

			if (g_Autowall.CanHitFloatingPoint(OriginAutowall, ViewPoint))
			{
				if (side == 0)
				{
					bside1 = true;
					FinalAngle = angToLocal + 90;
				}
				else if (side == 1)
				{
					bside2 = true;
					FinalAngle = angToLocal - 90;
				}
				autowalld = true;
			}
			else
			{
				for (int side222 = 0; side222 < 2; side222++)
				{
					Vector OriginAutowall222 = { Origin.x + OriginLeftRight[side222].x,  Origin.y - OriginLeftRight[side222].y , Origin.z + 80 };

					if (g_Autowall.CanHitFloatingPoint(OriginAutowall222, OriginAutowall2))
					{
						if (side222 == 0)
						{
							bside1 = true;
							FinalAngle = angToLocal + 90;
						}
						else if (side222 == 1)
						{
							bside2 = true;
							FinalAngle = angToLocal - 90;
						}
						autowalld = true;
					}
				}
			}
		}
	}

	if (!autowalld || (bside1 && bside2))
		g::pCmd->viewangles.y += 180;
	else
		g::pCmd->viewangles.y = FinalAngle;

	if (g_Menu.Config.JitterRange != 0)
	{
		float Offset = g_Menu.Config.JitterRange / 2.f;

		if (!g_Menu.Config.RandJitterInRange)
		{
			Swtich ? g::pCmd->viewangles.y -= Offset : g::pCmd->viewangles.y += Offset;
		}
		else
		{
			static bool oldSwtich = Swtich;

			g::pCmd->viewangles.y -= Offset;

			static int Add = 0;

			if (oldSwtich != Swtich)
			{
				Add = rand() % g_Menu.Config.JitterRange;
				oldSwtich = Swtich;
			}

			g::pCmd->viewangles.y += Add;
		}
	}
}
void AtTargets(CUserCmd* pCmd)
{
	for (int i = 1; i <= engine->GetMaxClients(); ++i) {
		C_BaseEntity* Entity = g_pEntityList->GetClientEntity(i);
		if (!Entity
			|| !Entity->IsAlive()
			|| Entity->IsDormant()
			|| Entity == g::pLocalEntity
			|| Entity->GetTeam() == g::pLocalEntity->GetTeam())
			continue;

		pCmd->viewangles.y = math.CalcAngle(g::pLocalEntity->GetOrigin(), Entity->GetOrigin()).y;
		continue;
	}
}
void FakeWalk(CUserCmd* pCmd, bool auto_stop, bool bSendPacket)
{
	if (!GetAsyncKeyState(VK_SHIFT) && !auto_stop)
		return;

	bSendPacket = true;
	Vector velocity = g::pLocalEntity->GetVelocity();
	int choked = engine->GetNetChannel()->m_nChokedPackets;
	int ticks_left = 14 - choked;

	bSendPacket = (choked < 14) ? false : true;

	if (!choked || ticks_left < 4 || bSendPacket)
		g_Aimbot.movement_stop();
}

void AntiAim::OnCreateMove()
{
	if (!engine->IsInGame() || !g_Menu.Config.Antiaim || g_Menu.Config.LegitBacktrack)
		return;

	if (!g::pLocalEntity->IsAlive())
		return;

	if (!g::pLocalEntity->GetActiveWeapon() || g::pLocalEntity->IsKnifeorNade())
		return;

	//lby check
	static bool Switch = true;

	//at targets
	if (g_Menu.Config.attarget /*&& g_Vars->HvH.Yaws != 1*/)
		AtTargets(g::pCmd);

	float flServerTime = g::pLocalEntity->GetTickBase() * global_vars->intervalPerTick;
	bool canShoot = (g::pLocalEntity->GetActiveWeapon()->GetNextPrimaryAttack() <= flServerTime);

	if (canShoot && (g::pCmd->buttons & IN_ATTACK))
		return;

	//lby breaker
	static int tick = 0;
	if (tick >= 180)
		tick = 0;

	g::pCmd->viewangles.x = 89.9f;

	FreeStanding();

	static bool chek = true;

	if (g::bSendPacket && chek)
	{
		Switch = !Switch;
		chek = false;
	}
	else if (!g::bSendPacket)
		chek = true;

	tick++;
	FakeWalk(g::pCmd, false, g::bSendPacket);
}