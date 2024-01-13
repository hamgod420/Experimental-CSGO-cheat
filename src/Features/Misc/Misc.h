#pragma once

#include "..\Aimbot\Autowall.h"
#include "..\Aimbot\Aimbot.h"
#include "..\Aimbot\LagComp.h"
#include "..\..\Utils\GlobalVars.h"
#include "..\..\Utils\Math.h"
#include "..\..\SDK\ICvar.h"
#include "..\..\SDK\CPrediction.h"
#include "..\..\Menu\Menu.h"
#include <iostream>
#include <algorithm>

// kinda just throw shit in here

#define _SOLVEY(a, b, c, d, e, f) ((c * b - d * a) / (c * f - d * e))
#define SOLVEY(...) _SOLVEY(?, ?, ?, ?, ?, ?)
#define SOLVEX(y, world, forward, right) ((world.x - right.x * y) / forward.x)

class Misc
{
public:
    void OnCreateMove()
    {
        this->pCmd   = g::pCmd;
        this->pLocal = g::pLocalEntity;

		this->DoAutostrafe();
		this->DoBhop();
		this->DoFakeLag();
    };

	void MovementFix(Vector& oldang) // i think osmium
	{
		Vector vMovements(g::pCmd->forwardmove, g::pCmd->sidemove, 0.f);

		if (vMovements.Length2D() == 0)
			return;

		Vector vRealF, vRealR;
		Vector aRealDir = g::pCmd->viewangles;
		aRealDir.Clamp();

		math.AngleVectors(aRealDir, &vRealF, &vRealR, nullptr);
		vRealF[2] = 0;
		vRealR[2] = 0;

		VectorNormalize(vRealF);
		VectorNormalize(vRealR);

		Vector aWishDir = oldang;
		aWishDir.Clamp();

		Vector vWishF, vWishR;
		math.AngleVectors(aWishDir, &vWishF, &vWishR, nullptr);

		vWishF[2] = 0;
		vWishR[2] = 0;

		VectorNormalize(vWishF);
		VectorNormalize(vWishR);

		Vector vWishVel;
		vWishVel[0] = vWishF[0] * g::pCmd->forwardmove + vWishR[0] * g::pCmd->sidemove;
		vWishVel[1] = vWishF[1] * g::pCmd->forwardmove + vWishR[1] * g::pCmd->sidemove;
		vWishVel[2] = 0;

		float a = vRealF[0], b = vRealR[0], c = vRealF[1], d = vRealR[1];
		float v = vWishVel[0], w = vWishVel[1];

		float flDivide = (a * d - b * c);
		float x = (d * v - b * w) / flDivide;
		float y = (a * w - c * v) / flDivide;

		g::pCmd->forwardmove = x;
		g::pCmd->sidemove = y;
	}

	void ThirdPerson(ClientFrameStage_t curStage)
	{
		if (!engine->IsInGame() || !engine->IsConnected() || !g::pLocalEntity)
			return;
			
		static bool init = false;

		if (GetKeyState(VK_MBUTTON) && g::pLocalEntity->IsAlive())
		{
			if (init)
			{
				ConVar* sv_cheats = g_pCvar->FindVar("sv_cheats");
				*(int*)((DWORD)&sv_cheats->fnChangeCallback + 0xC) = 0; // ew
				sv_cheats->SetValue(1);
				engine->ExecuteClientCmd("thirdperson");
			}
			init = false;
		}
		else
		{
			if (!init)
			{
				ConVar* sv_cheats = g_pCvar->FindVar("sv_cheats");
				*(int*)((DWORD)&sv_cheats->fnChangeCallback + 0xC) = 0; // ew
				sv_cheats->SetValue(1);
				engine->ExecuteClientCmd("firstperson");
			}
			init = true;
		}

		if (curStage == FRAME_RENDER_START && GetKeyState(VK_MBUTTON) && g::pLocalEntity->IsAlive())
		{
			g_pPrediction->SetLocalViewAngles(Vector(g::RealAngle.x, g::RealAngle.y, 0)); // lol
		}
	}

	void Crosshair()
	{
		if (!g::pLocalEntity)
			return;

		if (!g::pLocalEntity->IsAlive())
			return;
		
		if (g::pLocalEntity->IsScoped() && g_Menu.Config.NoScope)
		{
			int Height, Width;
			engine->GetScreenSize(Width, Height);

			Vector punchAngle = g::pLocalEntity->GetAimPunchAngle();

			float x = Width / 2;
			float y = Height / 2;
			int dy = Height / 90;
			int dx = Width / 90;
			x -= (dx*(punchAngle.y));
			y += (dy*(punchAngle.x));

			Vector2D screenPunch = { x, y };

			surface->Line(0, screenPunch.y, Width, screenPunch.y, Color(0, 0, 0, 255));
			surface->Line(screenPunch.x, 0, screenPunch.x, Height, Color(0, 0, 0, 255));
		}

		static bool init = false;
		static bool init2 = false;

		if (g_Menu.Config.Crosshair)
		{
			if (g::pLocalEntity->IsScoped())
			{
				if (init2)
				{
					ConVar* sv_cheats = g_pCvar->FindVar("sv_cheats");
					*(int*)((DWORD)&sv_cheats->fnChangeCallback + 0xC) = 0; // ew
					sv_cheats->SetValue(1);

					engine->ExecuteClientCmd("weapon_debug_spread_show 0");
					engine->ExecuteClientCmd("crosshair 0");
				}
				init2 = false;
			}
			else
			{
				if (!init2)
				{
					ConVar* sv_cheats = g_pCvar->FindVar("sv_cheats");
					*(int*)((DWORD)&sv_cheats->fnChangeCallback + 0xC) = 0; // ew
					sv_cheats->SetValue(1);

					engine->ExecuteClientCmd("weapon_debug_spread_show 3");
					engine->ExecuteClientCmd("crosshair 1");
				}
				init2 = true;
			}

			init = false;
		}
		else
		{
			if (!init)
			{
				ConVar* sv_cheats = g_pCvar->FindVar("sv_cheats");
				*(int*)((DWORD)&sv_cheats->fnChangeCallback + 0xC) = 0; // ew
				sv_cheats->SetValue(1);

				engine->ExecuteClientCmd("weapon_debug_spread_show 0");
				engine->ExecuteClientCmd("crosshair 1");
			}
			init = true;
		}
	}

	void NormalWalk() // heh
	{
		g::pCmd->buttons &= ~IN_MOVERIGHT;
		g::pCmd->buttons &= ~IN_MOVELEFT;
		g::pCmd->buttons &= ~IN_FORWARD;
		g::pCmd->buttons &= ~IN_BACK;

		if (g::pCmd->forwardmove > 0.f)
			g::pCmd->buttons |= IN_FORWARD;
		else if (g::pCmd->forwardmove < 0.f)
			g::pCmd->buttons |= IN_BACK;
		if (g::pCmd->sidemove > 0.f)
		{
			g::pCmd->buttons |= IN_MOVERIGHT;
		}
		else if (g::pCmd->sidemove < 0.f)
		{
			g::pCmd->buttons |= IN_MOVELEFT;
		}
	}

private:
    CUserCmd*     pCmd;
    C_BaseEntity* pLocal;

    void DoBhop() const
    {
        if (!g_Menu.Config.Bhop)
            return;

		if (!g::pLocalEntity->IsAlive())
			return;

        static bool bLastJumped = false;
        static bool bShouldFake = false;

        if (!bLastJumped && bShouldFake)
        {
            bShouldFake = false;
            pCmd->buttons |= IN_JUMP;
        }
        else if (pCmd->buttons & IN_JUMP)
        {
            if (pLocal->GetFlags() & FL_ONGROUND)
                bShouldFake = bLastJumped = true;
            else 
            {
                pCmd->buttons &= ~IN_JUMP;
                bLastJumped = false;
            }
        }
        else
            bShouldFake = bLastJumped = false;
    }

	void DoAutostrafe() const
	{
		if (!engine->IsConnected() || !engine->IsInGame() || !g_Menu.Config.AutoStrafe)
			return;

		if (!g::pLocalEntity->IsAlive())
			return;

		if (!(g::pLocalEntity->GetFlags() & FL_ONGROUND) && GetAsyncKeyState(VK_SPACE))
		{
			pCmd->forwardmove = (10000.f / g::pLocalEntity->GetVelocity().Length2D() > 450.f) ? 450.f : 10000.f / g::pLocalEntity->GetVelocity().Length2D();
			pCmd->sidemove = (pCmd->mousedx != 0) ? (pCmd->mousedx < 0.0f) ? -450.f : 450.f : (pCmd->command_number % 2) == 0 ? -450.f : 450.f;	
		}
	}

	void DoFakeLag() const
	{
		if (!engine->IsConnected() || !engine->IsInGame() || g_Menu.Config.Fakelag == 0 || g_Menu.Config.LegitBacktrack)
			return;
		
		if (!g::pLocalEntity->IsAlive())
			return;

		if (g::pLocalEntity->IsKnifeorNade())
			return;

		auto NetChannel = engine->GetNetChannel();

		if (!NetChannel)
			return;
		
		static float maxSpeed = 320.f;
		static float Acceleration = 5.5f;
		float maxAccelspeed = Acceleration * maxSpeed * global_vars->intervalPerTick;

		float TicksToStop = g::pLocalEntity->GetVelocity().Length() / maxAccelspeed;

		bool canHit = false;
		static bool init = false;
		static bool stop = true;
		static bool stop2 = true;
		bool skip = false;

		if (g_Menu.Config.FakeLagOnPeek)
		{
			for (int i = 1; i < engine->GetMaxClients(); ++i)
			{
				C_BaseEntity* pPlayerEntity = g_pEntityList->GetClientEntity(i);

				if (!pPlayerEntity
					|| !pPlayerEntity->IsAlive()
					|| pPlayerEntity->IsDormant()
					|| pPlayerEntity == g::pLocalEntity
					|| pPlayerEntity->GetTeam() == g::pLocalEntity->GetTeam())
					continue;

				Vector EnemyHead = { pPlayerEntity->GetOrigin().x, pPlayerEntity->GetOrigin().y, (pPlayerEntity->GetHitboxPosition(0, g_Aimbot.Matrix[pPlayerEntity->EntIndex()]).z + 10.f) };

				Vector Head = { g::pLocalEntity->GetOrigin().x, g::pLocalEntity->GetOrigin().y, (g::pLocalEntity->GetHitboxPosition(0, g_Aimbot.Matrix[pPlayerEntity->EntIndex()]).z + 10.f) };
				Vector HeadExtr = (Head + (g::pLocalEntity->GetVelocity() * 0.203125f));
				Vector OriginExtr = ((g::pLocalEntity->GetOrigin() + (g::pLocalEntity->GetVelocity() * 0.21875f)) + Vector(0, 0, 8));

				float dmg;

				if (abs(g::pLocalEntity->GetVelocity().Length2D()) > 50.f && (g_Autowall.CanHitFloatingPoint(HeadExtr, EnemyHead) || g_Autowall.CanHitFloatingPoint(OriginExtr, EnemyHead)))
				{
					canHit = true;
				}
			}

			if (canHit)
			{
				if (stop2)
				{
					init = true;
					stop2 = false;
				}
			}
			else
				stop2 = true;

			if (init)
			{
				if (!stop)
				{
					g::bSendPacket = true;
					g::LagPeek = true;
					stop = true;
					skip = true;
				}
			}

			if (!skip)
			{
				if (g::LagPeek)
				{
					if (NetChannel->m_nChokedPackets < 13)
						g::bSendPacket = false;
					else
						g::LagPeek = false;
				}
				else
				{
					g::bSendPacket = (NetChannel->m_nChokedPackets >= g_Menu.Config.Fakelag);
					stop = false;
					init = false;
				}
			}
		}
		else
		{
			init = false;
			stop = true;
			stop2 = true;
			g::LagPeek = false;

			g::bSendPacket = (NetChannel->m_nChokedPackets >= g_Menu.Config.Fakelag);

			if (GetAsyncKeyState(VK_SHIFT))
				g::bSendPacket = (NetChannel->m_nChokedPackets >= 13);
		}

	}
};

extern Misc g_Misc;