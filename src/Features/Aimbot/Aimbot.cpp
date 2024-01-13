#include "Aimbot.h"
#include "Autowall.h"
#include "LagComp.h"
#include "..\AntiAim\AntiAim.h"
#include "..\..\Utils\Utils.h"
#include "..\..\SDK\IVEngineClient.h"
#include "..\..\SDK\PlayerInfo.h"
#include "..\..\SDK\ICvar.h"
#include "..\..\Utils\Math.h"
#include "..\..\SDK\Hitboxes.h"
#include "..\..\Menu\Menu.h"

Aimbot g_Aimbot;

void Aimbot::movement_stop()
{
	if (!g_Menu.Config.Autostop)
		return;

	Vector Velocity = g::pLocalEntity->GetVelocity();
	if (Velocity.Length2D() == 0)
		return;

	static float speed = 450.f;

	Vector direction;
	Vector RealView;
	math.VectorAngles(Velocity, direction);
	engine->GetViewAngles(RealView);
	direction.y = RealView.y - direction.y;

	Vector forward;
	math.AngleVectors(direction, &forward);
	Vector negated_direction = forward * -speed;

	g::pCmd->forwardmove = negated_direction.x;
	g::pCmd->sidemove = negated_direction.y;
}

bool Aimbot::HitChance(C_BaseEntity* pEnt, C_BaseCombatWeapon* pWeapon, Vector Angle, Vector Point, int chance)
{
	if (chance == 0 || g_Menu.Config.Hitchance == 0)
		return true;

	if (Backtrack[pEnt->EntIndex()] || ShotBacktrack[pEnt->EntIndex()]) // doing this bec im lazy
	{
		float Velocity = g::pLocalEntity->GetVelocity().Length();
		float SpreadCone = pWeapon->GetAccuracyPenalty() * 256.0f / M_PI * Velocity / 3000.0f; // kmeth https://github.com/DankPaster/kmethdude
		float a = (Point - g::pLocalEntity->GetEyePosition()).Length();
		float b = sqrt(tan(SpreadCone * M_PI / 180.0f) * a);
		if (2.2f > b) return true;
		return (chance <= ( (2.2f / fmax(b, 2.2f)) * 100.0f));
	}

	float Seeds = (g_Menu.Config.Hitchance == 1) ? 356.f : 256.f;

	Angle -= (g::pLocalEntity->GetAimPunchAngle() * g_pCvar->FindVar("weapon_recoil_scale")->GetFloat());

	Vector forward, right, up;

	math.AngleVectors(Angle, &forward, &right, &up);

	int Hits = 0, neededHits = (Seeds * (chance / 100.f));

	float weapSpread = pWeapon->GetSpread(), weapInaccuracy = pWeapon->GetInaccuracy();

	for (int i = 0; i < Seeds; i++)
	{
		float Inaccuracy = math.RandomFloat(0.f, 1.f) * weapInaccuracy;
		float Spread = math.RandomFloat(0.f, 1.f) * weapSpread;

		Vector spreadView((cos(math.RandomFloat(0.f, 2.f * M_PI)) * Inaccuracy) + (cos(math.RandomFloat(0.f, 2.f * M_PI)) * Spread), (sin(math.RandomFloat(0.f, 2.f * M_PI)) * Inaccuracy) + (sin(math.RandomFloat(0.f, 2.f * M_PI)) * Spread), 0), direction;
		direction = Vector(forward.x + (spreadView.x * right.x) + (spreadView.y * up.x), forward.y + (spreadView.x * right.y) + (spreadView.y * up.y), forward.z + (spreadView.x * right.z) + (spreadView.y * up.z)).Normalize();

		Vector viewanglesSpread, viewForward;

		math.VectorAngles(direction, up, viewanglesSpread);
		math.NormalizeAngles(viewanglesSpread);

		math.AngleVectors(viewanglesSpread, &viewForward);
		viewForward.NormalizeInPlace();

		viewForward = g::pLocalEntity->GetEyePosition() + (viewForward * pWeapon->GetCSWpnData()->m_flRange);

		C_Trace Trace;

		g_pTrace->ClipRayToEntity(C_Ray(g::pLocalEntity->GetEyePosition(), viewForward), mask_shot | contents_grate, pEnt, &Trace);

		if (Trace.m_pEnt == pEnt)
			Hits++;

		if (((Hits / Seeds) * 100.f) >= chance)
			return true;

		if ((Seeds - i + Hits) < neededHits)
			return false;
	}
	
	return false;
}
bool ShouldBaim(C_BaseEntity* pEnt) 
{
	static float oldSimtime[65];
	static float storedSimtime[65];

	static float ShotTime[65];
	static float NextShotTime[65];
	static bool BaimShot[65];

	if (storedSimtime[pEnt->EntIndex()] != pEnt->GetSimulationTime())
	{
		oldSimtime[pEnt->EntIndex()] = storedSimtime[pEnt->EntIndex()];
		storedSimtime[pEnt->EntIndex()] = pEnt->GetSimulationTime();
	}

	float simDelta = storedSimtime[pEnt->EntIndex()] - oldSimtime[pEnt->EntIndex()];

	bool Shot = false;

	if (pEnt->GetActiveWeapon() && !pEnt->IsKnifeorNade())
	{
		if (ShotTime[pEnt->EntIndex()] != pEnt->GetActiveWeapon()->GetLastShotTime())
		{
			Shot = true;
			BaimShot[pEnt->EntIndex()] = false;
			ShotTime[pEnt->EntIndex()] = pEnt->GetActiveWeapon()->GetLastShotTime();
		}
		else
			Shot = false;
	}
	else
	{
		Shot = false;
		ShotTime[pEnt->EntIndex()] = 0.f;
	}

	if (Shot)
	{
		NextShotTime[pEnt->EntIndex()] = pEnt->GetSimulationTime() + pEnt->FireRate();

		if (simDelta >= pEnt->FireRate())
			BaimShot[pEnt->EntIndex()] = true;
	}

	if (BaimShot[pEnt->EntIndex()])
	{
		if (pEnt->GetSimulationTime() >= NextShotTime[pEnt->EntIndex()])
			BaimShot[pEnt->EntIndex()] = false;
	}

	if (g_Menu.Config.BaimPitch && BaimShot[pEnt->EntIndex()] && !(pEnt->GetFlags() & FL_ONGROUND))
		return true;

	if (g_Menu.Config.BaimInAir && !(pEnt->GetFlags() & FL_ONGROUND))
		return true;

	return false;
}

Vector Aimbot::Hitscan(C_BaseEntity* pEnt) // supremeemmemememememe
{
	float DamageArray[28];
	float tempDmg = 0.f;
	Vector tempHitbox = { 0,0,0 };
	static int HitboxForMuti[] = { 2,2,4,4,6,6 };

	float angToLocal = math.CalcAngle(g::pLocalEntity->GetOrigin(), pEnt->GetOrigin()).y;

	Vector2D MutipointXY = { (sin(math.GRD_TO_BOG(angToLocal))),(cos(math.GRD_TO_BOG(angToLocal))) };
	Vector2D MutipointXY180 = { (sin(math.GRD_TO_BOG(angToLocal + 180))) ,(cos(math.GRD_TO_BOG(angToLocal + 180))) };
	Vector2D Mutipoint[] = { Vector2D(MutipointXY.x, MutipointXY.y), Vector2D(MutipointXY180.x, MutipointXY180.y) };

	float Velocity = abs(pEnt->GetVelocity().Length2D());

	if (!g_Menu.Config.DelayShot && Velocity > 29.f)
		Velocity = 30.f;

	std::vector<int> Scan;

	int HeadHeight = 0;

	bool Baim = ShouldBaim(pEnt);

	if (!Baim)
		Scan.push_back(HITBOX_HEAD);

	if (Velocity <= 215.f || Baim)
	{
		Scan.push_back(HITBOX_PELVIS);
		Scan.push_back(HITBOX_THORAX);
		Scan.push_back(HITBOX_LOWER_CHEST);
		Scan.push_back(HITBOX_UPPER_CHEST);

		if (g_Menu.Config.MultiPoint)
		{
			Scan.push_back(19);//pelvis
			Scan.push_back(20);

			Scan.push_back(21);//thorax
			Scan.push_back(22);

			Scan.push_back(23);//upperchest
			Scan.push_back(24);

			if (!Baim)
			{
				Scan.push_back(25);//head
				Scan.push_back(26);
				Scan.push_back(27);
			}

			HeadHeight = g_Menu.Config.HeadScale;
		}

		if (!g_Menu.Config.IgnoreLimbs)
			Velocity = 0.f;

		if (Velocity <= 29.f)
		{
			Scan.push_back(HITBOX_LEFT_FOOT);
			Scan.push_back(HITBOX_RIGHT_FOOT);
			Scan.push_back(HITBOX_LEFT_UPPER_ARM);
			Scan.push_back(HITBOX_RIGHT_UPPER_ARM);
			Scan.push_back(HITBOX_LEFT_THIGH);
			Scan.push_back(HITBOX_RIGHT_THIGH);
		}
	}

	Vector Hitbox;
	int bestHitboxint = 0;

	for (int hitbox : Scan)
	{
		if (hitbox < 19)
			Hitbox = pEnt->GetHitboxPosition(hitbox, Matrix[pEnt->EntIndex()]);
		else if (hitbox > 18 && hitbox < 25)
		{
			float Radius = 0;
			Hitbox = pEnt->GetHitboxPosition(HitboxForMuti[hitbox - 19], Matrix[pEnt->EntIndex()], &Radius);
			Radius *= (g_Menu.Config.BodyScale / 100.f);
			Hitbox = Vector(Hitbox.x + (Radius * Mutipoint[((hitbox - 19) % 2)].x), Hitbox.y - (Radius * Mutipoint[((hitbox - 19) % 2)].y), Hitbox.z);
		}
		else if (hitbox > 24 && hitbox < 28)
		{
			float Radius = 0;
			Hitbox = pEnt->GetHitboxPosition(0, Matrix[pEnt->EntIndex()], &Radius);
			Radius *= (HeadHeight / 100.f);
			if (hitbox != 27)
				Hitbox = Vector(Hitbox.x + (Radius * Mutipoint[((hitbox - 25) % 2)].x), Hitbox.y - (Radius * Mutipoint[((hitbox - 25) % 2)].y), Hitbox.z);
			else
				Hitbox += Vector(0, 0, Radius);
		}

		float Damage = g_Autowall.Damage(Hitbox);

		if (Damage > 0.f)
			DamageArray[hitbox] = Damage;
		else
			DamageArray[hitbox] = 0;

		if (g_Menu.Config.BaimLethal && hitbox != 0 && hitbox != 25 && hitbox != 26 && hitbox != 27 && Damage >= (pEnt->GetHealth() + 10))
		{
			DamageArray[hitbox] = 400;
			Baim = true;
		}

		if (DamageArray[hitbox] > tempDmg)
		{
			tempHitbox = Hitbox;
			bestHitboxint = hitbox;
			tempDmg = DamageArray[hitbox];
		}

		g::AimbotHitbox[pEnt->EntIndex()][hitbox] = Hitbox;
	}

	PlayerRecords pPlayerEntityRecord = g_LagComp.PlayerRecord[pEnt->EntIndex()].at(0);

	Backtrack[pEnt->EntIndex()] = false;
	ShotBacktrack[pEnt->EntIndex()] = false;

	if (g_Menu.Config.ShotBacktrack && g_LagComp.ShotTick[pEnt->EntIndex()] != -1 && g_Autowall.CanHitFloatingPoint(pEnt->GetHitboxPosition(HITBOX_HEAD, g_LagComp.PlayerRecord[pEnt->EntIndex()].at(g_LagComp.ShotTick[pEnt->EntIndex()]).Matrix) + Vector(0, 0, 1), g::pLocalEntity->GetEyePosition()) && !Baim)
	{
		bestEntDmg = (1000000.f - fabs(math.Distance(Vector2D(g::pLocalEntity->GetOrigin().x, g::pLocalEntity->GetOrigin().y), Vector2D(pEnt->GetOrigin().x, pEnt->GetOrigin().y)))); // just doing this to get the closest player im backtracking
		ShotBacktrack[pEnt->EntIndex()] = true;
		return pEnt->GetHitboxPosition(HITBOX_HEAD, g_LagComp.PlayerRecord[pEnt->EntIndex()].at(g_LagComp.ShotTick[pEnt->EntIndex()]).Matrix) + Vector(0, 0, 1);
	}
	else if (tempDmg >= g_Menu.Config.Mindmg)
	{
		bestEntDmg = tempDmg;

		if ((bestHitboxint == 25 || bestHitboxint == 26 || bestHitboxint == 27) && abs(DamageArray[HITBOX_HEAD] - DamageArray[bestHitboxint]) <= 10.f)
			return pEnt->GetHitboxPosition(HITBOX_HEAD, Matrix[pEnt->EntIndex()]);
		else if ((bestHitboxint == 19 || bestHitboxint == 20) && DamageArray[HITBOX_PELVIS] > 30)
			return pEnt->GetHitboxPosition(HITBOX_PELVIS, Matrix[pEnt->EntIndex()]);
		else if ((bestHitboxint == 21 || bestHitboxint == 22) && DamageArray[HITBOX_THORAX] > 30)
			return pEnt->GetHitboxPosition(HITBOX_THORAX, Matrix[pEnt->EntIndex()]);
		else if ((bestHitboxint == 23 || bestHitboxint == 24) && DamageArray[HITBOX_UPPER_CHEST] > 30)
			return pEnt->GetHitboxPosition(HITBOX_UPPER_CHEST, Matrix[pEnt->EntIndex()]);

		return tempHitbox;
	}
	else if (g_Menu.Config.PosBacktrack && pPlayerEntityRecord.Velocity >= 29.f && g_Autowall.CanHitFloatingPoint(pEnt->GetHitboxPosition(HITBOX_HEAD, pPlayerEntityRecord.Matrix), g::pLocalEntity->GetEyePosition()))
	{
		bestEntDmg = (100000.f - fabs(math.Distance(Vector2D(g::pLocalEntity->GetOrigin().x, g::pLocalEntity->GetOrigin().y), Vector2D(pEnt->GetOrigin().x, pEnt->GetOrigin().y))));
		Backtrack[pEnt->EntIndex()] = true;
		return pEnt->GetHitboxPosition(HITBOX_HEAD, pPlayerEntityRecord.Matrix);
	}

	return Vector(0, 0, 0);
}

void Aimbot::OnCreateMove()
{
	if (!engine->IsInGame())
		return;

	Vector Aimpoint = { 0,0,0 };
	C_BaseEntity* Target = nullptr;

	int targetID = 0;
	int tempDmg = 0;
	static bool shot = false;

	for (int i = 1; i <= engine->GetMaxClients(); ++i)
	{
		C_BaseEntity* pPlayerEntity = g_pEntityList->GetClientEntity(i);

		if (!pPlayerEntity
			|| !pPlayerEntity->IsAlive()
			|| pPlayerEntity->IsDormant())
		{
			g_LagComp.ClearRecords(i);
			continue;
		}

		g_LagComp.StoreRecord(pPlayerEntity);

		if (pPlayerEntity == g::pLocalEntity || pPlayerEntity->GetTeam() == g::pLocalEntity->GetTeam())
			continue;

		g::EnemyEyeAngs[i] = pPlayerEntity->GetEyeAngles();

		if (g_LagComp.PlayerRecord[i].size() == 0 || !g::pLocalEntity->IsAlive() || !g_Menu.Config.Aimbot || g_Menu.Config.LegitBacktrack)
			continue;

		if (!g::pLocalEntity->GetActiveWeapon() || g::pLocalEntity->IsKnifeorNade())
			continue;

		bestEntDmg = 0;

		Vector Hitbox = Hitscan(pPlayerEntity);

		if (Hitbox != Vector(0,0,0) && tempDmg <= bestEntDmg)
		{
			Aimpoint = Hitbox;
			Target = pPlayerEntity;
			targetID = Target->EntIndex();
			tempDmg = bestEntDmg;
		}
	}

	if (!g::pLocalEntity->IsAlive())
	{
		shot = false;
		return;
	}

	if (!g::pLocalEntity->GetActiveWeapon() || g::pLocalEntity->IsKnifeorNade())
	{
		shot = false;
		return;
	}

	if (shot)
	{
		if (g_Menu.Config.FixShotPitch) // ik it dosnt realy fix much just makes ur pitch go down faster
		{
			g::bSendPacket = true;
			g_AntiAim.OnCreateMove();
		}
		shot = false;
	}

	float flServerTime = g::pLocalEntity->GetTickBase() * global_vars->intervalPerTick;
	bool canShoot = (g::pLocalEntity->GetActiveWeapon()->GetNextPrimaryAttack() <= flServerTime);

	if (Target)
	{
		g::TargetIndex = targetID;
		float SimulationTime = 0.f;

		if (Backtrack[targetID])
			SimulationTime = g_LagComp.PlayerRecord[targetID].at(0).SimTime;
		else
			SimulationTime = g_LagComp.PlayerRecord[targetID].at(g_LagComp.PlayerRecord[targetID].size() - 1).SimTime;

		if (ShotBacktrack[targetID])
			SimulationTime = g_LagComp.PlayerRecord[targetID].at(g_LagComp.ShotTick[targetID]).SimTime;

		bool shouldstop = g_Menu.Config.Autostop ? true : canShoot;
		Vector Angle = math.CalcAngle(g::pLocalEntity->GetEyePosition(), Aimpoint);

		/* autostop */
		if (g::pLocalEntity->GetVelocity().Length() >= (g::pLocalEntity->GetActiveWeapon()->GetCSWpnData()->max_speed_alt * .34f) - 5 && shouldstop && !GetAsyncKeyState(VK_SPACE))
			movement_stop();

		if (!(g::pCmd->buttons & IN_ATTACK) && canShoot && HitChance(Target, g::pLocalEntity->GetActiveWeapon(), Angle, Aimpoint, g_Menu.Config.HitchanceValue))
		{		

			if (!Backtrack[targetID] && !ShotBacktrack[targetID])
				g::Shot[targetID] = true;

			if (g_Menu.Config.Ak47meme)
				engine->ExecuteClientCmd("play weapons\\ak47\\ak47-1.wav");

			g::bSendPacket = true;
			shot = true;

			g::pCmd->viewangles = Angle - (g::pLocalEntity->GetAimPunchAngle() * g_pCvar->FindVar("weapon_recoil_scale")->GetFloat());
			g::pCmd->buttons |= IN_ATTACK;
			g::pCmd->tick_count = TIME_TO_TICKS(SimulationTime + g_LagComp.LerpTime());
		}
	}
}