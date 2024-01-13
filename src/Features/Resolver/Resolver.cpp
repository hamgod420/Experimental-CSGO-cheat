#include "Resolver.h"
#include "..\Aimbot\Aimbot.h"
#include "..\Aimbot\Autowall.h"
#include "..\Aimbot\LagComp.h"
#include "..\..\Utils\Utils.h"
#include "..\..\SDK\IVEngineClient.h"
#include "..\..\SDK\Hitboxes.h"
#include "..\..\SDK\PlayerInfo.h"
#include "..\..\Utils\Math.h"
#include "..\..\Menu\Menu.h"

Resolver g_Resolver;

bool is_yaw_sideways(C_BaseEntity* Entity, float yaw) {
	const float at_target_yaw = math.CalcAngle(Entity->GetVecOrigin(), g::pLocalEntity->GetVecOrigin()).y;
	const float delta = fabs(math.NormalizedAngle(at_target_yaw - yaw));

	//return the delta
	return delta > 20.f && delta < 160.f;
}
bool is_yaw_backwards(C_BaseEntity* Entity, float yaw) {
	auto at_target_yaw = math.CalcAngle(Entity->GetVecOrigin(), g::pLocalEntity->GetVecOrigin()).y;
	math.NormalizeAngle(at_target_yaw);
	float delta = fabs(math.NormalizedAngle(at_target_yaw - yaw));

	//return the delta
	return delta >= 0.f && delta < 17.f;
}
float get_lby_rotated_yaw(float lby, float yaw) {
	float delta = math.NormalizedAngle(yaw - lby);
	if (fabs(delta) < 25.f)
		return lby;

	if (delta > 0.f)
		return yaw + 25.f;

	return yaw;
}

void Resolver::Resolve(C_BaseEntity* Entity)
{
	int index = Entity->EntIndex();
	static Vector Angle;
	Angle = Entity->GetEyeAngles();

	static float moving_lby[65];
	static float moving_sim[65];
	static float stored_lby[65];
	static float old_lby[65];
	static float lby_delta[65];
	static float predicted_yaw[65];
	static bool lby_changes[65];
	static int shots_check[65];
	static float angle_brute[65];
	static float AtTargetAngle;
	static float FixPitch;
	static float FixPitch2;
	static bool HitNS[65];
	static Vector StoredAngle[65];
	static Vector Hitstored[65];
	static int StoredShots[65];
	static int HitShots[65];
	static int HitShotsStored[65];

	// entity speed.
	auto speed = Entity->GetVelocity().Length2D();
	auto flags = Entity->GetFlags();
	auto delta = Entity->GetSimulationTime() - moving_sim[index];

	if (stored_lby[index] != Entity->GetLowerBodyYaw())
	{
		old_lby[index] = stored_lby[index];
		lby_changes[index] = true;
		stored_lby[index] = Entity->GetLowerBodyYaw();
	}

	lby_delta[index] = math.NormalizeYaw(stored_lby[index] - old_lby[index]);

	// setup moving.
	if (speed > 0.1f && flags & FL_ONGROUND) {
		moving_sim[index] = Entity->GetSimulationTime(); // setup moving simulation
	}


	if (lby_changes[index])
	{
		if ((Entity->GetSimulationTime() - moving_sim[index]) > .22f)
			predicted_yaw[index] = lby_delta[index];

		lby_changes[index] = false;
	}

	if (stored_lby[index] != Entity->GetLowerBodyYaw())
	{
		old_lby[index] = stored_lby[index];
		Angle.y = Entity->GetLowerBodyYaw();
		lby_changes[index] = true;
		stored_lby[index] = Entity->GetLowerBodyYaw();
		g::ResolverStage[index] = "LBY UPDATE";
	}
	else if (abs(Entity->GetVelocity().Length2D()) > 29.f && (Entity->GetFlags() & FL_ONGROUND))
	{
		Angle.y = Entity->GetLowerBodyYaw();
		moving_lby[index] = Entity->GetLowerBodyYaw();
		moving_sim[index] = Entity->GetSimulationTime();
		lby_changes[index] = false;
		predicted_yaw[index] = 0;
		g::MissedShotsResolve[index] = 0;
		angle_brute[index] = 0;
		g::ResolverStage[index] = "MOVING RESOLVE";
	}
	else if ((Entity->GetFlags() & FL_ONGROUND))
	{
		if (shots_check[index] != g::MissedShotsResolve[index])
		{
			angle_brute[index] += predicted_yaw[index];
			shots_check[index] = g::MissedShotsResolve[index];
		}

		Angle.y = math.NormalizeYaw(angle_brute[index] + moving_lby[index]);
		g::ResolverStage[index] = "MOVING-LBY + " + std::to_string(angle_brute[index]);
	}
	else
	{
		Angle.y = Entity->GetLowerBodyYaw();
		g::ResolverStage[index] = "AIR RESOLVE";
	}

	if (*g::MissedShotsResolve > 3) {
		switch (*g::MissedShotsResolve)
		{
			case 0: {
				Angle.y = Entity->GetLowerBodyYaw();
			} break;
			case 1: {
				Angle.y = Entity->GetLowerBodyYaw() + 180.f;
			} break;
			case 2: {
				Angle.y = Entity->GetLowerBodyYaw() + 110.f;
			} break;
			case 3: {
				Angle.y = Entity->GetLowerBodyYaw() - 110.f;
			} break;

		//default
			default:
				break;
		}
	}
	Entity->SetEyeAngles(Angle);
}
void Resolver::Apply(ClientFrameStage_t curStage)
{
	if (engine->IsInGame() && engine->IsConnected()) {
		for (int i = 1; i <= engine->GetMaxClients(); ++i) {
			C_BaseEntity* Entity = g_pEntityList->GetClientEntity(i);
			if (!Entity
				|| !Entity->IsAlive()
				|| Entity->IsDormant()
				|| Entity == g::pLocalEntity)
				continue;

			if (curStage == FRAME_NET_UPDATE_POSTDATAUPDATE_START && g::pLocalEntity->IsAlive() && Entity->GetTeam() != g::pLocalEntity->GetTeam())
			{
				Resolve(Entity);
			}
			else if (curStage == FRAME_NET_UPDATE_END)
			{
				auto VarMap = reinterpret_cast<uintptr_t>(Entity) + 36;
				auto VarMapSize = *reinterpret_cast<int*>(VarMap + 20);

				for (auto index = 0; index < VarMapSize; index++)
					*reinterpret_cast<uintptr_t*>(*reinterpret_cast<uintptr_t*>(VarMap) + index * 12) = 0; // bameware
			}
		}
	}
}